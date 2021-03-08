// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "Shooter/Shooter.h"

int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing, 
	TEXT("Draw debug lines for weapon"), 
	ECVF_Cheat);

// Sets default values
ASWeapon::ASWeapon()
{
	// Create mesh component
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	//MeshComp->AddRelativeRotation(FRotator(90.0f, 90.0f, 0.0f));

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	CritMultiplier = 2.0f;

	FireRate = 0.1f;

	BulletSpread = 2.0f;

	MaxAmmo = 200;

	ReloadSpeed = 1.0f;

	bFireModeAuto = true;

	// Replicate weapon for clients
	SetReplicates(true);
	// Set network update frequency (Default: 100, min 2)
	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = 30.0f;
}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	CurrentAmmo = MaxAmmo;

	DebugWeaponDrawing = 0.0f;
}

void ASWeapon::Fire()
{
	// Check if this is a client
	if (!HasAuthority())
	{
		// Call this function on the server and continue through code on client
		ServerFire();
	}

	// Trace world from pawn eyes to crosshair location
	AActor* WeaponOwner = GetOwner();

	if (WeaponOwner)
	{
		// Get pawn's eye location and look direction to create a trace
		FVector EyeLocation;
		FRotator EyeRotation;
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		// Add bullet spread
		float HalfRadian = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRadian, HalfRadian);
		// Create Weapon trace
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		// Collision properties that we want to know
		FCollisionQueryParams QueryParams; // Struct that holds collision info
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(WeaponOwner);
		QueryParams.bTraceComplex = true; // So that we know exactly where we are hitting on an object
		QueryParams.bReturnPhysicalMaterial = true;
		//QueryParams.bFindInitialOverlaps = true;

		// Tracer particle effect "Target"
		FVector TracerEndPoint = TraceEnd;

		// Set or Reset surface type
		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		// Checks if linetrace hits something / use ECC_Visibility to check if blocked by something visible)
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			// Get actor hit
			AActor* HitActor = Hit.GetActor();

			// Get type of surface hit by the bullet
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			// Play surface hit effects
			PlaySurfaceHitEffects(SurfaceType, Hit.ImpactPoint);

			// Check if headshot
			float AppliedDamage = Damage;
			if (SurfaceType == SurfaceType2)
			{
				AppliedDamage *= CritMultiplier;
			}

			// Apply Damage
			UGameplayStatics::ApplyPointDamage(HitActor, AppliedDamage, ShotDirection, Hit, WeaponOwner->GetInstigatorController(), WeaponOwner, DamageType);

			TracerEndPoint = Hit.ImpactPoint;


		}
		
		// Draw debug line to show linetrace
		if (DebugWeaponDrawing > 0)
		{
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.0f, 0, 1.0f);
		}

		// Play effects
		PlayFireEffects(TracerEndPoint);

		// Replicate hitscan tracepoint on all clients
		if (HasAuthority())
		{
			// Update struct 
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}
		
		// Add a timer for when the last shot happened
		TimeSinceLastShot = GetWorld()->TimeSeconds;

		// Reduce ammo from clip
		CurrentAmmo -= 1;
	}
	
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	// Spawn muzzle effects, check if assigned first
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	// Spawn bullet smoke trail effects
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	// Shake camera when firing
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(CamShake);
		}
	}
}

void ASWeapon::PlaySurfaceHitEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	// Switch effects depending on surface hit
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_BODYDEFAULT:
		SelectedEffect = HitBodyEffect;
		break;
	case SURFACE_BODYCRIT:
		SelectedEffect = HitBodyEffect;
		break;
	default:
		SelectedEffect = HitDefaultEffect;
		break;
	}

	// Spawn hit effect at hit location
	if (SelectedEffect)
	{
		// Get the location of the muzzle
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		// Get the direction of the shot
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		// Spawn surface effect
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	// Play effects
	PlayFireEffects(HitScanTrace.TraceTo);
	// Play surface hit effects
	PlaySurfaceHitEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// Server functions require _Implementation
void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

// Server functions might require _Implementation
bool ASWeapon::ServerFire_Validate()
{
	// Anti-cheat and validation checks are put here
	return true;
}


void ASWeapon::StartFire()
{
	// Check if we have ammo first
	if (CurrentAmmo > 0)
	{
		// Gets the max value for the firt delay to prevent delay from being < 0
		float FirstDelay = FMath::Max(TimeSinceLastShot + FireRate - GetWorld()->TimeSeconds, 0.0f);
		// After a delay of float FireRate, call the function Fire() on ASWeapon.
		GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, FireRate, true, FirstDelay);
	}
	
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::Reload()
{
	// needs to play reload animation**
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Reloading.."));
	// 
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenReload, this, &ASWeapon::StartReloading, ReloadSpeed, false, ReloadSpeed);
}

void ASWeapon::StartReloading()
{
	CurrentAmmo = MaxAmmo;
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenReload);
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicates to any relevant client connected to server
	// Uses a condition so that this is not replicated to the client calling it
	// That's because the effects already play on clients
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}


