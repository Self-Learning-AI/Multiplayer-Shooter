// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"


enum class EWaveState : uint8;

// What do we want to send when the player gets a kill?
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AActor*, KillerController); // Victim actor, killer actor, 

/**
 * 
 */
UCLASS()
class SHOOTER_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	// Timer handle for the bot spawner
	FTimerHandle TimerHandle_BotSpawner;

	// Timer handle for delay between waves
	FTimerHandle TimerHandle_NextWaveStart;

	// Timer float for the delay between waves
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;

	// Number of bots that will be spawned on each wave
	int32 BotsToSpawn;

	// Number of waves cleared and the current wave the player is on
	int32 WaveCount;

	// Hook to spawn a single bot
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	// Start spawning bots
	void StartWaveSpawning();

	// Stop spawning bots
	void EndWaveSpawning();

	// Set timer for new wave
	void SetupNextWave();

	void BotSpawnerTimerElapsed();

	void CheckWaveState();

	void CheckPlayersLive();

	void GameOver();

	void SetWaveState(EWaveState NewWaveState);

	void RespawnDeadPlayers();

public:

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
};
