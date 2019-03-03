// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"

void ASPlayerState::AddScore(float ScoreDelta)
{
	// The Score variable is inherited from the PlayerState class
	Score += ScoreDelta;
}
