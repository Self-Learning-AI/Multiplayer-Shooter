// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickUpActor.generated.h"

class USphereComponent;
class UDecalComponent;
class ASBuffActor;

UCLASS()
class SHOOTER_API ASPickUpActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickUpActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Pawn detections
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASBuffActor> BuffClass;

	ASBuffActor* BuffInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float RespawnTime;

	FTimerHandle TimerHandle_RespawnTimer;

	void Respawn();

public:	

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
