// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

// Contains info of a single hitscan weapon linetrace
// Needed for networking
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	// Enums such as EPhysicalSurface cant be directly replicated
	// It needs to be converted to a byte first
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class SHOOTER_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* HitDefaultEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* HitBodyEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Damage;

	/* Bullet Spread in Degrees*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bFireModeAuto;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float CritMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float ReloadSpeed;

	float TimeSinceLastShot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int MaxAmmo;

	int CurrentAmmo;

	//FTimerHandle TimerHandle_TimeBetweenShots;

	//FTimerHandle TimerHandle_TimeBetweenReload;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<UCameraShake> CamShake;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Fire();

	void PlayFireEffects(FVector TraceEnd);

	void PlaySurfaceHitEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:

	FTimerHandle TimerHandle_TimeBetweenShots;

	FTimerHandle TimerHandle_TimeBetweenReload;

	UFUNCTION()
	virtual void StartFire();

	UFUNCTION()
	void StopFire();

	void Reload();

	void StartReloading();
};
