// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * The Player State is similar to Game State except for player controllers.
 * By default, the player controller only exists on the original client and the server
 * This means that a players cannot see any information from other players
 * PlayerState solves this by replicating variables that can be propagated to clients
 */
UCLASS()
class COOPHORDESHOOTER_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// BlueprintCallable allows us to implement this function in Blueprints
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
		void AddScore(float ScoreDelta);

};
