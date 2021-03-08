// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"
#include "Net/UnrealNetwork.h"


void ASGameState::OnRep_WaveState(EWaveState OldWaveState)
{
	WaveStateChanged(WaveState, OldWaveState);
}

void ASGameState::SetWaveState(EWaveState NewWaveState)
{
	// Only run the wave state change on the server so it can be replicated to clients
	if (HasAuthority())
	{
		// Pass old wave state in case we need it in future implementations
		EWaveState OldWaveState = WaveState;

		WaveState = NewWaveState;
		OnRep_WaveState(OldWaveState);
	}
}

void ASGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Replicates to any relevant client connected to server
	DOREPLIFETIME(ASGameState, WaveState);
}