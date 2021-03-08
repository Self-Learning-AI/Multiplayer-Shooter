// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "EngineUtils.h"
#include "SGameState.h"
#include "SPlayerState.h"

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 4.0f;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 2.0f;

	// Set up game state class
	GameStateClass = ASGameState::StaticClass();

	// Set up player state class
	PlayerStateClass = ASPlayerState::StaticClass();
}

void ASGameMode::StartPlay()
{
	Super::StartPlay();

	SetupNextWave();
}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// NOTE this function is called every 2 seconds

	// Check to see if the wave has cleared
	CheckWaveState();
	// Check to see if there are any players alive
	CheckPlayersLive();
}

void ASGameMode::StartWaveSpawning()
{
	// Increment the wave count
	WaveCount++;

	// Multiply the number of bots to spawn by the wave count
	BotsToSpawn = 2 * WaveCount;

	// Set timer and call function to spawn bots after timer expires
	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::BotSpawnerTimerElapsed, 1.0f, true, 0.0f);

	// Set wave state
	SetWaveState(EWaveState::SpawningWave);
}

void ASGameMode::EndWaveSpawning()
{
	// Reset the bot spawner timer
	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);

	// Set wave state
	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameMode::SetupNextWave()
{
	// Set timer for next wave to spawn
	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWaveSpawning, TimeBetweenWaves, false);

	// Set new wave state
	SetWaveState(EWaveState::WaitingToStart);

	// Respawn dead players
	RespawnDeadPlayers();
}

void ASGameMode::BotSpawnerTimerElapsed()
{
	SpawnNewBot();

	// Decrement bots to spawn count
	BotsToSpawn--;

	if (BotsToSpawn <= 0)
	{
		EndWaveSpawning();
	}
}

void ASGameMode::CheckWaveState()
{
	// Check if we are currently waiting for the next wave (already setting up wave)
	bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);
	// If we are still spawning bots OR waiting for next wave, dont check wave
	if (BotsToSpawn > 0 || bIsPreparingForWave)
	{
		return;
	}

	bool bIsAnyBotAlive = false;

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* TestPawn = *It;
		// Ignore all pawns that are null or is player controlled
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			continue;
		}

		USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		// Check if one of the pawns has health
		if (HealthComp && HealthComp->GetHealth() > 0.0f)
		{
			bIsAnyBotAlive = true;
			break;
		}
	}

	// If there are no more bots left with health, we set up the next wave
	if (!bIsAnyBotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		SetupNextWave();
	}
}

void ASGameMode::CheckPlayersLive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			APawn* MyPawn = PC->GetPawn();
			USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			// Ensure is used to breakpoint the code if a player character does not have a health component
			// Check if player is also alive
			if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
			{
				// A player is still alive, therefore the game can go on
				return;
			}
		}
	}

	// Else if no players are left alive
	GameOver();
}

void ASGameMode::GameOver()
{
	EndWaveSpawning();
	// NEEDS TO DISPLAY GAME OVER SCREEN
	UE_LOG(LogTemp, Log, TEXT("Game Over..."));

	// Set wave state to game over
	SetWaveState(EWaveState::GameOver);
}

void ASGameMode::SetWaveState(EWaveState NewWaveState)
{
	// Dont need to cast because they are the same class
	ASGameState* GS = GetGameState<ASGameState>();
	// Breakpoint if there is no gamestate
	if (ensureAlways(GS))
	{
		// Assign new wave state
		//GS->WaveState = NewWaveState;
		GS->SetWaveState(NewWaveState);
	}
}

void ASGameMode::RespawnDeadPlayers()
{
	// Get all player controllers
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			RestartPlayer(PC);
		}

	}
}


