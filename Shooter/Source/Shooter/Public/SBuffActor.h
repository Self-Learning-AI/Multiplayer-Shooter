// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SBuffActor.generated.h"

UCLASS()
class SHOOTER_API ASBuffActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASBuffActor();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	/* Time between calling the buff ticks */
	UPROPERTY(EditDefaultsOnly, Category = "Buffs")
	float BuffTickRate;

	// Is used by clients to check if buff is active
	UPROPERTY(ReplicatedUsing = OnRep_BuffActive)
	bool bIsBuffActive;

	/* Total number of ticks that we apply the effects */
	UPROPERTY(EditDefaultsOnly, Category = "Buffs")
	int32 BuffMaxTicks;

	// Number of ticks already applied
	int32 BuffTicksCount;

	FTimerHandle TimerHandle_BuffTick;

	UFUNCTION()
	void OnTickBuff();

	// Used to replicate buff status to clients
	UFUNCTION()
	void OnRep_BuffActive();

	UFUNCTION(BluePrintImplementableEvent, Category = "Buffs")
	void OnBuffStateChanged(bool bNewIsActive);

public:	

	void ActivateBuff(AActor* BuffInstigator);

	UFUNCTION(BluePrintImplementableEvent, Category = "Buffs")
	void OnActivated(AActor* BuffInstigator);

	UFUNCTION(BluePrintImplementableEvent, Category = "Buffs")
	void OnBuffTicked();

	UFUNCTION(BluePrintImplementableEvent, Category = "Buffs")
	void OnExpired();


};
