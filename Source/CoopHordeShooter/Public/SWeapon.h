// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;

UCLASS()
class COOPHORDESHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Skeletal meshes are used for meshes that need animation while Static Meshes do not use animations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* SkelMeshComp;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void Fire();

	// EditDefaultsOnly allows setup in editor but not at runtime
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
