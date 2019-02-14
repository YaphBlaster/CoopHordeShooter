// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class UParticleSystem;
class USHealthComponent;
class URadialForceComponent;

UCLASS()
class COOPHORDESHOOTER_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USHealthComponent* HealthComp;

	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Replicated, Category = "FX")
		UParticleSystem* ExplodeFX;

	/*Pawn died previously*/
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Exploded, Category = "Components")
		bool bHasExploded;

	UFUNCTION()
		void OnRep_Exploded();

	void Explode();

	UPROPERTY(VisibleAnywhere, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Material")
		UMaterial* BurntMaterial;

	UPROPERTY(EditAnywhere, Category = "Explosion")
		float UpwardBarrelLaunchForce;


	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComp;


};
