// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

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
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMeshComp"));
	RootComponent = SkelMeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

}

void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location (center screen)

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

		// Particle "Target" Parameter
		FVector TracerEndPoint = TraceEnd;

		// Will fill our linetrace results in this FHitResult object
		FHitResult HitResult;

		// Line traces return a boolean (hit or not hit)
		// Therefore we check to see if the boolean is true (something was hit)
		if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, ECC_Visibility, QueryParams))
		{
			// Blocking hit! Process damage

			AActor* HitActor = HitResult.GetActor();

			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, HitResult, MyOwner->GetInstigatorController(), this, DamageType);

			// If designer has set the impact effect in blueprints...
			if (ImpactEffect)
			{
				// Spawn a particle effect at the impact point's location and rotation
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
			}

			TraceEnd = HitResult.ImpactPoint;
		}

		if (DebugWeaponDrawing > 0)
		{
			// Draw a debug line to see the linetrace
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}


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

	}


}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

