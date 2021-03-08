// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon_Launcher.h"

void ASWeapon_Launcher::StartFire()
{
	AActor* WeaponOwner = GetOwner();
	if (WeaponOwner && ProjectileClass)
	{
		// Get pawn's eye location and look direction to create a trace
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation);
	}
}
