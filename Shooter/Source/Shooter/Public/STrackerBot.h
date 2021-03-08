// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class SHOOTER_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	ASTrackerBot(); 

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	//UFUNCTION()
	void Explode();

	void DamageSelf();

	void RefreshPath();

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundCue* ExplosionSound;

	FVector GetNextPathPoint();

	// The next point in the navigation path
	FVector NextPathPoint;

	// Timer to rest the pathing of the bot
	FTimerHandle TimerHandle_RefreshPath;

	// Used for the speed of the movement
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MovementForce;

	// Used for the speed of the movement
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float TargetPathDistance;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	bool bUseVelocityChange;

	// Dynamic pulsing material
	UMaterialInstanceDynamic* MaterialInstance;
	
	// Explosion effect
	UPROPERTY(EditDefaultsOnly, Category = "Components")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDestructDelay;

	//UPROPERTY(Replicated)
	bool bExploded;

	//UPROPERTY(Replicated)
	bool bSelfDestructing;

	FTimerHandle TimerHandle_SelfDamage;

public:	

	virtual void Tick(float DeltaTime) override;
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};
