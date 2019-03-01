// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

// Enumeration declaration
// Enum classes are prefixed with an "E"
// We will pass in BlueprintType as we will be using the states to derive what we want to display for the player's HUD
// Enums must extend uint8 if we want to access it in Blueprints
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	// No longer spawning new bots, waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver,
};


/**
 *
 */
UCLASS()
class COOPHORDESHOOTER_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UFUNCTION()
		void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
		void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	// We set the property type to BlueprintReadOnly as we don't want blueprints to mess with the state, however we do want blueprint to be able to read from the WaveState
	// Replicated using lets us use the previous value
	// IE Equipping a new weapon and rembering the old weapon to switch back to quickly
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "Game State")
		EWaveState WaveState;

public:
	void SetWaveState(EWaveState NewState);

};
