// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectile.h"
#include "DrawDebugHelpers.h"
#include "Public/SWeapon.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASProjectile::ASProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASProjectile::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASProjectile::Explode()
{

	// Create a collision sphere
	FCollisionShape ExplosionSphere = FCollisionShape::MakeSphere(200.0f);

	// draw collision sphere
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionSphere.GetSphereRadius(), 12, FColor::Yellow, true, 1.0f, 0, 2.0f);

	//Apply radial damage
	UGameplayStatics::ApplyRadialDamage(GetWorld(), 100.0f, GetActorLocation(), ExplosionSphere.GetSphereRadius(), UDamageType::StaticClass(), TArray<AActor*>(), this, this->GetInstigatorController(), true);

	// Play explosion FX
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, GetActorLocation());

	// Destroy the actor
	Destroy();

}
