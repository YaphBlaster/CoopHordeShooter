// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopHordeShooter.h"
#include "TimerManager.h"
#include "UnrealNetwork.h"

// Sets the DebugWeaponDrawing value
// FALSE = 0
// TRUE = 1
static int32 DebugWeaponDrawing = 0;

// Console Variable
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons")
	TEXT("0 = off\n")
	TEXT("1 = visualize weapon fire lines\n"),
	ECVF_Cheat
);


// Sets default values
ASWeapon::ASWeapon()
{
	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMeshComp"));
	RootComponent = SkelMeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	// This allows us to spawn the weapon on the client when the weapon is originally spawned on the server
	// Ripples through from server to client
	SetReplicates(true);

	// Sets up the networks update frequency for this class
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;

}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;

	LastFireTime = -TimeBetweenShots;

}


void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location (center screen)

	// When the client calls Fire, their role will be less than the ROLE_AUTHORITY (Server)
	// The second time this runs, after ServerFire is called, the Role will be ROLE_AUTHORITY (Server) and will therefore be skipped
	if (Role < ROLE_Authority)
	{
		// make the request to the server to fire for the client
		ServerFire();
	}

	// Otherwise we are the server so we fire our own weapon
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		// Initialize variables
		FVector EyeLocation;
		FRotator EyeRotation;

		// Since the parameters from GetActorEyesViewPoint are references,
		// It will fill in EyeLocation and EyeRotation when the function executes
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		/* Bullet Spread */

		// Radian for random cone
		float HalfRad = FMath::DegreesToRadians(BulletSpread);

		// Vector random and cone
		// This will make a random shot direction so the bullet isn't pixel perfect
		// We will overwrite the original shot direction (which was 100% accurate) to the new slightly randomized cone shot direction
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad);

		// Where the line trace will end
		// Wherever we are currently looking, plus the eye rotation multiplied by 10,000 
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		// Ignore the owner in our collision query 
		QueryParams.AddIgnoredActor(MyOwner);
		// Ignore the weapon (this class) in our collision query 
		QueryParams.AddIgnoredActor(this);
		// Trace against each individual triangle of hit mesh
		// More expensive but much more precise
		// This precision helps with logic for specific hit location logic (EX. Headshots)
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		// Particle "Target" Parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		// Will fill our linetrace results in this FHitResult object
		FHitResult HitResult;

		// Line traces return a boolean (hit or not hit)
		// Therefore we check to see if the boolean is true (something was hit)
		// COLLISION_WEAPON is an global variable that we added in the CoopHordeShooter.h file
		// It equates to ECC_GameTraceChannel1, which can be set in the Editor (Collision - Add Trace Channel)
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Blocking hit! Process damage

			// Get the actor that was hit
			AActor* HitActor = HitResult.GetActor();


			// Weak object pointer
			// Allows the system to delete it if it's not being used
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

			// Retrieve damage amount
			float ActualDamage = BaseDamage;

			// Check if the surface type that was hit was a vulnerable area (EX: Headshot)
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				// Multiply the damage amount for damage bonus
				ActualDamage *= 4.0f;
			}

			// Apply point damage
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			PlayImpactEffect(SurfaceType, HitResult.ImpactPoint);
			// Set the TracerEndpoint to be where the hitresult's impact point was
			TracerEndPoint = HitResult.ImpactPoint;

			HitScanTrace.SurfaceType = SurfaceType;
		}

		// Console commands can be accessed by hitting the tilde "~" key
		// The following variable is attached to this command "COOP.DebugWeapons"
		// Initialization can be found on line 15
		// If the console variable is greater than 0, ...
		if (DebugWeaponDrawing > 0)
		{
			// Draw a debug line to see the linetrace
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		PlayFireEffects(TracerEndPoint);

		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		// Keep track of the time when the shot was fired
		// Used to not allow the player to spam the fire button
		LastFireTime = GetWorld()->TimeSeconds;
	}


}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);

	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);

}

// Since this is a replicated function implementation
// This code will only be run on the server
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

// Used for anti-cheating
// Used to validate code to see if something was wrong
// If there was something wrong, this would automatically disconnect the client from the server
bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::StartFire()
{
	// FMath::Max
	// Forces to choose the max value of the two arguments given
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);


}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{

	if (MuzzleEffect)
	{
		// Since the weapon is constantly moving, we use SpawnEmitterAttached
		// NOTE: SpawnEmitterAtLocation would not work because of this constant movement
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkelMeshComp, MuzzleSocketName);
	}

	FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocketName);

	if (TracerEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	// Camera Shake

	APawn* MyPawn = Cast<APawn>(GetOwner());
	if (MyPawn)
	{
		APlayerController* PC = Cast<APlayerController>(MyPawn->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	// Create an temporary value
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
		// Global Vars set in the CoopHordeShooter.h file
		// If FleshDefault or FleshVulnerable, set the effect to the flesh impact effect
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
		// If no specified surface was hit, apply the default impact effect
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	// If designer has set the impact effect in blueprints...
	if (SelectedEffect)
	{
		FVector MuzzleLocation = SkelMeshComp->GetSocketLocation(MuzzleSocketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		// Spawn a particle effect at the impact point's location and rotation
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

// This needs to be made whenever a corresponding header file variable uses replication
// GetLifetimeReplicatedProps allows us to specify what we want to replicate and how we want to replicate it
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Default most simple replication
	// Replicate to any relevant client that is connected to us
	// DOREPLIFETIME_CONDITION allows us to make a variable replicated based on a condition
	// The following has a condition of COND_SkipOwner and will therefore not replicate to the owning client
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);

}