// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "Kismet/GameplayStatics.h"
#include "Public/Components/SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "UnrealNetwork.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set to physics body to let radial component affect us (eg. when a nearby barrel explodes)
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;

	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->ImpulseStrength = 250;
	RadialForceComp->bImpulseVelChange = true;
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	RadialForceComp->bIgnoreOwningActor = true;


	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	SetReplicates(true);

	// Replicates physics
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();

}

void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{

	if (bHasExploded)
		return;

	if (Health <= 0.0f)
	{
		// Explode!
		bHasExploded = true;

		Explode();
	}
}

// Replication function that runs from server to clients
void ASExplosiveBarrel::OnRep_Exploded()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Explosion!"));

	// Play FX and change self material to black
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeFX, GetActorLocation());

	// Override material on mesh with blackened version
	MeshComp->SetMaterial(0, BurntMaterial);
}

void ASExplosiveBarrel::Explode()
{

	// Boost the barrel upwards
	FVector BoostIntensity = FVector::UpVector * UpwardBarrelLaunchForce;
	MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

	// Play FX and change self material to black
	UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeFX, GetActorLocation());

	// Override material on mesh with blackened version
	MeshComp->SetMaterial(0, BurntMaterial);

	// Blast away nearby physics actors
	RadialForceComp->FireImpulse();

}


// This needs to be made whenever a corresponding header file variable uses replication
// GetLifetimeReplicatedProps allows us to specify what we want to replicate and how we want to replicate it
void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Default most simple replication
	// Replicate to any relevant client that is connected to us
	DOREPLIFETIME(ASExplosiveBarrel, bHasExploded);


}


