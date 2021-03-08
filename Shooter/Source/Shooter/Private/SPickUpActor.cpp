// Fill out your copyright notice in the Description page of Project Settings.


#include "SPickUpActor.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "SBuffActor.h"
#include "TimerManager.h"

// Sets default values
ASPickUpActor::ASPickUpActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(70.0f);
	RootComponent = SphereComp;

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp->SetupAttachment(RootComponent);
	DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	DecalComp->DecalSize = FVector(60.0f, 75.0f, 75.0f);
	//this->AddInstanceComponent(SphereComp);
	//this->AddInstanceComponent(DecalComp);

	RespawnTime = 120.0f;

}

// Called when the game starts or when spawned
void ASPickUpActor::BeginPlay()
{
	Super::BeginPlay();

	// Spawn item on server
	if (HasAuthority())
	{
		Respawn();
	}
}

void ASPickUpActor::Respawn()
{
	if (BuffClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Powerup class is null for %s, set it up in bp"), *GetName());
		return;
	}

	// Spawn parameters to always spawn, even if an object is over it
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Spawn the buff instance
	BuffInstance = GetWorld()->SpawnActor<ASBuffActor>(BuffClass, GetTransform(), SpawnParams);
}

void ASPickUpActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// NEEDS TO GIVE PLAYER POWERS
	if (HasAuthority() && BuffInstance)
	{
		// Activates buff and sends the actor that activated it
		BuffInstance->ActivateBuff(OtherActor);
		BuffInstance = nullptr;

		// Set respawn time
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPickUpActor::Respawn, RespawnTime);
	}

}



