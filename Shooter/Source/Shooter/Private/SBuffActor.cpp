// Fill out your copyright notice in the Description page of Project Settings.


#include "SBuffActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASBuffActor::ASBuffActor()
{
	BuffTickRate = 0.0f;
	BuffMaxTicks = 0;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	//MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetReplicates(true);
}

void ASBuffActor::OnTickBuff()
{
	BuffTicksCount++;

	OnBuffTicked();

	// Check if max ticks is reached
	if (BuffMaxTicks <= BuffTicksCount)
	{
		// End buff
		OnExpired();

		// Replicating to clients
		bIsBuffActive = false;
		OnRep_BuffActive();

		// Clear buff timer
		GetWorldTimerManager().ClearTimer(TimerHandle_BuffTick);
	}
}

void ASBuffActor::ActivateBuff(AActor* BuffInstigator)
{
	// NOTE: This function is called on server

	OnActivated(BuffInstigator);
	// Replicates buff state to clients
	bIsBuffActive = true;
	// Manually call replication on to also play on server
	OnRep_BuffActive();

	if (BuffTickRate > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_BuffTick, this, &ASBuffActor::OnTickBuff, BuffTickRate, true);
	}
	else
	{
		OnTickBuff();
	}
}

void ASBuffActor::OnRep_BuffActive()
{
	OnBuffStateChanged(bIsBuffActive);
}

void ASBuffActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASBuffActor, bIsBuffActive);
}


