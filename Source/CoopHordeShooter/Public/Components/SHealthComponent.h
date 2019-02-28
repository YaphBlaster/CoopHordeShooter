// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// OnHealthChanged event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS(ClassGroup = (COOP), meta = (BlueprintSpawnableComponent))
class COOPHORDESHOOTER_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
		float Health;

	UFUNCTION()
		void OnRep_Health(float OldHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
		float DefaultHealth;

	// Derived from the Actor.h Delegate to match signature of
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams( FTakeAnyDamageSignature, AActor*, DamagedActor, float, Damage, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser );
	// FTakeAnyDamageSignature
	UFUNCTION()
		void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Health Component")
		void Heal(float HealAmount);
};
