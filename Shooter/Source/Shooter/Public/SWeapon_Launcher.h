// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SWeapon_Launcher.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API ASWeapon_Launcher : public ASWeapon
{
	GENERATED_BODY()

protected:

	virtual void StartFire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
};
