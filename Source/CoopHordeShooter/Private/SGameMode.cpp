// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 2.0f;

	// By default the tick is set to false for a GameMode
	// For our game we need to set this to true
	PrimaryActorTick.bCanEverTick = true;
	// Setting the tick to only tick once a second
	// Ticking on a very low frequency
	PrimaryActorTick.TickInterval = 1.0f;
}


void ASGameMode::StartWave()
{
	// Wave Level
	WaveCount++;

	// Set the number of bots to spawn
	// As the wave level increases so does the number of bots
	NumberOfBotsToSpawn = 2 * WaveCount;


	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
}

void ASGameMode::EndWave()
{
	// Wave has finished so we must clear the bot spawning timer
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	// With the timer cleared, we can begin preparation for the next wave
	PrepareForNextWave();

}

void ASGameMode::PrepareForNextWave()
{
	// Handler initialization
	FTimerHandle TimerHandle_NextWaveStart;

	// Set a timer to start the next wave
	// Waiting for TimeBetweenWaves number of seconds
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);
}


void ASGameMode::CheckWaveState()
{
	// setting a boolean that derives its value from an FTimerHandle
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

	// if we are still spawning bots, or we are in the middle of preparing for the next wave
	if (NumberOfBotsToSpawn > 0 || bIsPreparingForWave)
	{
		// return out, as we are not ready to check waves
		return;
	}

	bool bIsAnyBotAlive = false;

	// Keep a list of pawns in the level
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		// If there is no TestPawn or the Pawn is controlled by a player (IE. Not AI)
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		// If the bot has a health comp and the bot's health is above 0,
		// We are not ready to start the next wave
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			// there is at least one bot alive so we set the boolean to true
			bIsAnyBotAlive = true;

			// break right away as we don't have to find any other bots
			break;
		}

	}

	// no bots are alive,
	if (!bIsAnyBotAlive)
	{
		// Get the next wave ready
		PrepareForNextWave();
	}

}

void ASGameMode::CheckAnyPlayerAlive()
{
	// Iterate over all the player controllers available
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		// Store the player controller from the iterator
		APlayerController* PC = Iterator->Get();

		// Check if player controller not null and player controller has a pawn
		if (PC && PC->GetPawn())
		{
			// Store the pawn from the player controller
			APawn* MyPawn = PC->GetPawn();

			// Store the HealthComponent from the pawn actor
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

			// Ensure is an assert
			// This will create a break in our code if HealthComp is false or null
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				// A player is still alive.
				return;
			}
		}
	}

	// No player alive
	GameOver();


}

void ASGameMode::GameOver()
{
	EndWave();

	// @TODO: Finish up the match, present 'game over' to players.

	UE_LOG(LogTemp, Log, TEXT("GAME OVER! Players Died"));
}

void ASGameMode::StartPlay()
{
	// This is a GameMode specific function that we are overriding
	// Therefore we must call Super to receive powers from our forefathers
	Super::StartPlay();

	// Begin next wave prep work
	PrepareForNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	// We almost always want to call super
	Super::Tick(DeltaSeconds);


	// Every once per second, we will check the wave state
	CheckWaveState();

	// Every once per second, we want to check if a player is still alive
	CheckAnyPlayerAlive();

}

// Although the name may be tricky
// This method will be called whenever we want to spawn a new bot
void ASGameMode::SpawnBotTimerElapsed()
{
	// spawn bot
	SpawnNewBot();

	// Decrement the number of bots to spawn
	NumberOfBotsToSpawn--;

	// If we have reached zero bots
	if (NumberOfBotsToSpawn <= 0)
	{
		// End the current wave
		EndWave();
	}
}
