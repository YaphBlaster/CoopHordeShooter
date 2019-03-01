// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameState.h"
#include "UnrealNetwork.h"

void ASGameState::OnRep_WaveState(EWaveState OldState)
{
	WaveStateChanged(WaveState, OldState);
}

// GetLifetimeReplicatedProps allows us to specify what we want to replicate and how we want to replicate it
void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Default most simple replication
	// Replicate to any relevant client that is connected to us
	DOREPLIFETIME(ASGameState, WaveState);

}