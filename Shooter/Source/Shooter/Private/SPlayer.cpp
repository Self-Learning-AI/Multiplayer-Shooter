// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "..\Public\SPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shooter/Shooter.h"
#include "SHealthComponent.h"
#include "SWeapon.h"


// Sets default values
ASPlayer::ASPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Allows engine's crouching system
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// Creates camera spring arm
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

	// Sets Aim Speed
	AimSpeed = 15.0f;

	// Creates camera and attaches to spring arm, makes it control the character rotation
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->AttachToComponent(SpringArmComp, FAttachmentTransformRules::KeepWorldTransform);
	CameraComp->bUsePawnControlRotation = true;

	// Sets socket name for spawning weapons
	WeaponSocket = "WeaponSocket_r";

	// Set Alive status to true
	bAlive = true;

	// Set up health component function for when player takes damage
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASPlayer::OnHealthChanged);
}

// Called when the game starts or when spawned
void ASPlayer::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;

	// Run spawning on server
	if (HasAuthority())
	{
		// Spawning Weapon
		// Get spawning parameters
		FActorSpawnParameters SpawnParams;
		// Make it always spawn, even if colliding
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// Spawn starting weapon
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(PrimaryWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocket);
		}
	}
}

void ASPlayer::MoveForward(float x)
{
	AddMovementInput(GetActorForwardVector() * x);
}

void ASPlayer::MoveRight(float x)
{
	AddMovementInput(GetActorRightVector() * x);
}

bool ASPlayer::GetPlayerIsAlive()
{
	return bAlive;
}

void ASPlayer::BeginCrouch()
{
	if (!bToggleCrouch)
	{
		Crouch();
	}
	else if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
	
}

void ASPlayer::EndCrouch()
{
	UnCrouch();
}

// Called every frame
void ASPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Changes the current FOV depending on the state of bZoom
	// Interpolates current FOV to target FOV
	float TargetFOV = bZoom ? AimFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, AimSpeed);
	CameraComp->SetFieldOfView(NewFOV);
}

void ASPlayer::Aim()
{
	if (bZoom)
	{
		bZoom = false;
	}
	else
	{
		bZoom = true;
	}
}

void ASPlayer::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASPlayer::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASPlayer::Reload()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Player Pressed R"));
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

void ASPlayer::OnHealthChanged(USHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// Check if health is 0 or below
	if (CurrentHealth <= 0.0f && bAlive)
	{
		// Set player as dead
		bAlive = false;
		// Stop player movement
		GetMovementComponent()->StopMovementImmediately();
		// Prevent further collisions with player
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Detach player controller from character, which will be destroyed
		DetachFromControllerPendingDestroy();
		// Set despawn time in seconds
		SetLifeSpan(20.0f);
	}
}

// Called to bind functionality to input
void ASPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//***KEY BINDINGS***
	// Move & Rotate
	PlayerInputComponent->BindAxis("MoveForward", this, &ASPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASPlayer::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASPlayer::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &ASPlayer::AddControllerYawInput);
	// Crouch
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASPlayer::BeginCrouch);
	if (!bToggleCrouch)
	{
		PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASPlayer::EndCrouch);
	}
	// Jump
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	// Aim
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASPlayer::Aim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASPlayer::Aim);
	// Fire
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASPlayer::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASPlayer::StopFire);
	// Reload
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASPlayer::Reload);
}

FVector ASPlayer::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	else
	{
		return Super::GetPawnViewLocation();
	}
	
}

void ASPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicates to any relevant client connected to server
	DOREPLIFETIME(ASPlayer, CurrentWeapon);
	DOREPLIFETIME(ASPlayer, bAlive);
}

