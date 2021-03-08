// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

//OnHealthChanged event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, CurrentHealth, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(Shooter), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	float CurrentHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	float MaxHealth;

	bool bDead;

	UFUNCTION()
	void OnRep_CurrentHealth(float PreviousHealth);

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float HealthGained);

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HealthComponent")
	uint8 TeamNumber;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;
};
