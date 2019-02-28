// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Public/SCharacter.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Public/Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	// This object will not get in the way of a navigation volume
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	// Set to query only as we don't want physics on the component
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// By setting the response to all channels to Ignore, we put less stress on the physics engine
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	// We only set what we need
	// Set the the collider to respond to pawns when we overlap
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistancetoTarget = 100;

	ExplosionDamage = 40.0f;
	ExplosionRadius = 200.0f;

	SelfDamageInterval = 0.25f;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		// Find initial move to
		NextPathPoint = GetNextPathPoint();
	}

	MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));


}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta,
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// If the material instance has not been set
	if (MatInst == nullptr)
	{
		// Create dynamic instance
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());

	// Explode on hitpoins == 0
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	// Hack, to get player location
	// Pointer to the player that the tracker ball will move to
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);


	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	// If Navpath's Pathpoints are less than 1 that means we are at the current point
	if (NavPath && NavPath->PathPoints.Num() > 1)
	{
		// Return the next point in the path (at index 1)
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}

void ASTrackerBot::SelfDestruct()
{
	// If the trackerbot has already exploded
	if (bExploded)
	{
		// Return out
		return;
	}

	// Set validation boolean to true
	bExploded = true;

	// Play Particle System at the trackerbot's position
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	// Check for null is not needed as the PlaySoundAtLocation already does the check
	// Alt+G into PlaySoundAtLocation to see this
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	// Hide the Mesh Component
	MeshComp->SetVisibility(false, true);

	// Disable all collision
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		// TArray is just UE syntax for an array
		// It will be of type Actor*
		// This will store the actors that will not take damage
		TArray<AActor*> IgnoredActor;

		// We add ourselves to the ignore list
		IgnoredActor.Add(this);

		// Increase damage based on the power level (challenge code)
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		// Apply Damage
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActor, this, GetInstigatorController(), true);

		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);

		// Destroy actor in 2 seconds
		SetLifeSpan(2.0f);

	}


}

void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::CheckNearbyBots()
{

	// distance to check for nearby bots
	const float Radius = 600.0f;

	// Create temporary collision shape for overlaps
	FCollisionShape SphereCollisionShape;
	SphereCollisionShape.SetSphere(Radius);

	// Only find Pawns (eg. players and AI bots)
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bot's mesh component is set to Physics Body in blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);
	QueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	// This following is good for overlapping actors within a single collision area
	// However, this does not allow us to use a bigger or smaller collision area
	// Check if any bots are nearby
	// TArray<AActor*> NearbyTrackerBots;
	// GetOverlappingActors(NearbyTrackerBots, ASTrackerBot::StaticClass());

	TArray<FOverlapResult> Overlaps;

	// Returns an array of overlap results that is stored in the first parameter of the function
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, SphereCollisionShape);


	int32 NumberOfBots = 0;


	// Loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps)
	{

		// Check if we overlapped with another tracker but (ignoring players and other bot types)
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		// Ignore this Trackerbot Instance
		if (Bot && Bot != this)
		{
			NumberOfBots++;
		}
	}


	const int32 MaxPowerLevel = 4;

	// Clamp between min=0 and max=4
	PowerLevel = FMath::Clamp(NumberOfBots, 0, MaxPowerLevel);

	// Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know the max power level
	// which can be tweaked many times by gameplay decisions (would mean we need to keep 2 places up to date)
	float Alpha = PowerLevel / float(MaxPowerLevel);
	// NOTE: (float)MaxPowerLevel converts the int32 to a float,
	// otherwise the following happens when dealing when dividing integers: 1/4 = 0 ('PowerLevel' int / 'MaxPowerLevel' int = 0 int)
	// this is a common programming problem and can be fixed by 'casting' the int (MaxPowerLevel) to a float before dividing

	//Set material to pulse per number of nearby bots
	MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
}


// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckNearbyBots();

	if (Role == ROLE_Authority && !bExploded)
	{
		// Returns a float that is the distance between the tracker ball's current location and the next point
		// .Size() converts vector to float
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		// If the distance to the target is less than the required distance, ...
		if (DistanceToTarget <= RequiredDistancetoTarget)
		{

			// Get the next path point
			NextPathPoint = GetNextPathPoint();

			DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached");

		}
		else
		{
			// Keep moving towards next target

			// In order to get a direction, we subtract the desired location by the tracker ball's current location
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();

			ForceDirection *= MovementForce;

			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);


		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
	}



}

// Override of base Actor Method
void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	if (!bStartedSelfDestruction && !bExploded)
	{
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);

		if (PlayerPawn)
		{
			// We overlapped with a player!

			if (Role == ROLE_Authority)
			{
				// Start self destruction sequence
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;

			// We use SpawnSoundAttached instead of PlaySoundAtLocation because
			// The TrackerBot will be moving and we want the sound to move with it
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}

}

