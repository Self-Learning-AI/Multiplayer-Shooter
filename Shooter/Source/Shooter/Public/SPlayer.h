// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SPlayer.generated.h"

class UCameraComponent; // Tells the compiler this is a class
class USpringArmComponent;
class USHealthComponent;
class ASWeapon;

UCLASS()
class SHOOTER_API ASPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ***FUNCTIONS***
	void MoveForward(float x);

	void MoveRight(float x);

	UFUNCTION(BlueprintCallable)
	bool GetPlayerIsAlive();

	void BeginCrouch();
	
	void EndCrouch();

	void Aim();

	void Reload();

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, Category = "Player")
	//ASWeapon* PrimaryWeapon;
	TSubclassOf<ASWeapon> PrimaryWeapon;

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponSocket;

	// ***COMPONENTS***
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	// ***Variables***
	bool bZoom;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Player")
	bool bAlive;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (ClampMin = 0.1, ClampMax = 100))
	float AimSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float AimFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float DefaultFOV;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controls")
	bool bToggleCrouch;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();
};
