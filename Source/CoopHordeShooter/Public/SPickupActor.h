// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPHORDESHOOTER_API ASPickupActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		// Decals create a flat image that can be applied to a plane
		// It always 2 dimensional
		UDecalComponent* DecalComp;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
		TSubclassOf<ASPowerupActor> PowerUpClass;

	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditDefaultsOnly, Category = "PickupActor")
		float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	void Respawn();

public:
	// Override of base function NotifyActorBeginOverlap
	// This allows us to see which Other Actors are overlapping with our Actor
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
