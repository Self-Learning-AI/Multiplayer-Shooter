// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	SpawningWave,

	WaveInProgress,

	WaveComplete,

	GameOver
};

/**
 * 
 */
UCLASS()
class SHOOTER_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:
	
	UFUNCTION()
	void OnRep_WaveState(EWaveState OldWaveState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewWaveState, EWaveState OldWaveState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;
	
public:

	void SetWaveState(EWaveState NewWaveState);
};
