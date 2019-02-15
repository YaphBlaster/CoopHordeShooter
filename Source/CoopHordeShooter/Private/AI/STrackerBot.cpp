// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "Public/SCharacter.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	// This object will not get in the way of a navigation volume
	MeshComp->SetCanEverAffectNavigation(false);

	RootComponent = MeshComp;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

}

FVector ASTrackerBot::GetNextPathPoint()
{
	// Hack, to get player location
	// Pointer to the player that the tracker ball will move to
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);


	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	// If Navpath's Pathpoints are less than 1 that means we are at the current point
	if (NavPath->PathPoints.Num() > 1)
	{
		// Return the next point in the path (at index 1)
		return NavPath->PathPoints[1];
	}

	// Failed to find path
	return GetActorLocation();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


