// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "UnrealNetwork.h"
#include "SGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefaultHealth = 100;
	bIsDead = false;

	TeamNum = 255;

	// Make sure that the component is being replicated
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Only hook if we are server
	// Since we are in a component and NOT an actor
	// We have to call GetOwnerRole
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();

		if (MyOwner)
		{
			// Subscribe a function to the owner's OnTakeAnyDamage event
			// This is done with AddDynamic
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = DefaultHealth;

}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	// Derive damage form the Health minus the OldHealth
	float Damage = Health - OldHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// If the damage incurred is less than 0 OR if the character is already dead
	if (Damage <= 0.0f || bIsDead)
	{
		// Exit out of this function
		return;
	}

	// If the DamageCauser IS NOT the DamageActor (Self harming TrackerBots)
	// AND
	// If the DamageActor and the DamageCauser are both friendly (On the same team)
	if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
	{
		// Exit out of this function
		return;
	}

	// Update health clamped
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));

	// Single line conditional assignment
	// If the Health is less than OR equal to zero then we set the bIsDead to TRUE
	// Otherwise it is greater than zero and we set it to FALSE
	bIsDead = Health <= 0.0f;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

	if (bIsDead)
	{
		// Getting the GameMode
		// Only valid on the server
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode());

		if (GM)
		{
			// Broadcast the event to all connected parties
			// The First parameter in the Broadcast is the victim (The owner of this health component)
			// The Second parameter in the Broadcast is the killer (The DamageCauser Actor)
			// The Third parameter in the Broadcast is the killer's controller (The InstigatedBy Controller)
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}


}


float USHealthComponent::GetHealth() const
{
	return Health;
}

void USHealthComponent::Heal(float HealAmount)
{
	if (HealAmount <= 0.0f || Health <= 0.0f)
	{
		return;
	}

	Health = FMath::Clamp(Health + HealAmount, 0.0f, DefaultHealth);
	UE_LOG(LogTemp, Log, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

	OnHealthChanged.Broadcast(this, Health, -HealAmount, nullptr, nullptr, nullptr);


}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorB == nullptr && ActorB == nullptr)
	{
		// Assume friendly
		return true;
	}
	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		// Assume friendly
		return true;
	}

	// Return true if the 2 health components are from the same team
	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}

// This needs to be made whenever a corresponding header file variable uses replication
// GetLifetimeReplicatedProps allows us to specify what we want to replicate and how we want to replicate it
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Default most simple replication
	// Replicate to any relevant client that is connected to us
	DOREPLIFETIME(USHealthComponent, Health);

}

