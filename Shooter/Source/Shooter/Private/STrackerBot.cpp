// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SPlayer.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set up mesh component
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);

	// Set up health component
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	// Speed of movement
	MovementForce = 600.0f;

	// Movement
	bUseVelocityChange = true;
	TargetPathDistance = 50.0f;

	// Combat
	ExplosionDamage = 50.0f;
	ExplosionRadius = 250.0f;
	SelfDestructDelay = 0.3f;

	// Self destruct trigger to detect if player is within kaboom range
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereConp"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetSphereRadius(ExplosionRadius);
	// Set the sphere to trigger and not physical collisions
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// Ignore object collisions with the sphere other than the player pawn
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		// Get the first point to move to in the path
		NextPathPoint = GetNextPathPoint();
	}
	
}

void ASTrackerBot::HandleTakeDamage(USHealthComponent* InHealthComp, float CurrentHealth, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	// Check if there is a material instance and create one if there isnt
	// This is used to create the blinking effect
	if (MaterialInstance == nullptr)
	{
		// The material needs to be dynamic so that not all blueprints are affected by a change
		MaterialInstance = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue("LastTimeHit", GetWorld()->TimeSeconds);
	}

	// Explode when health is 0
	if (CurrentHealth <= 0.0f)
	{
		Explode();
	}
	
}

void ASTrackerBot::Explode()
{
	// NOTE that on multiplayer, destroying the object instantly prevents clients
	// from playing the explosion effect as the server updates the condition before
	// the code is executed
	if (bExploded)
	{
		return;
	}

	bExploded = true;

	// Spawn explosion effect
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	// Play explosion sound effect
	UGameplayStatics::SpawnSoundAtLocation(this, ExplosionSound, GetActorLocation());

	// Disable the mesh
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (HasAuthority())
	{
		// Set actors ignored by explosion, eg friendly units
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Apply damage in a radius
		UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 3.0f, 0, 1.0f);

		// Destroy the trackerbot.. after 1.5 seconds
		//Destroy();
		SetLifeSpan(1.5f);
	}
	
}

void ASTrackerBot::DamageSelf()
{
	bSelfDestructing = true;

	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

FVector ASTrackerBot::GetNextPathPoint()
{
	// Get player position
	//ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	AActor* OptimalTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		// Ignore all pawns that are null or is player controlled
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		// Check if one of the pawns has health
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			// Get distance between actor and AI
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance)
			{
				OptimalTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
			
		}
	}

	if (OptimalTarget)
	{
		// Get navigation path from this actor, to the player actor
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), OptimalTarget);
		
		// Reset timer
		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		// Set a timer to refresh pathfind in case bot gets stuck
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath, 5.0f, false);

		if (NavPath)
		{
			if (NavPath->PathPoints.Num() > 1)
			{
				// Return the next path location in the route
				return NavPath->PathPoints[1];
			}
		}
	}

	
	// Failed to find path
	return GetActorLocation();
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && !bExploded)
	{
		// Get the distance from the target location
		float DistanceToPoint = (GetActorLocation() - NextPathPoint).Size();


		if (DistanceToPoint <= TargetPathDistance)
		{
			NextPathPoint = GetNextPathPoint();
		}
		else
		{

			// Get direction from this actor to the path point location
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			// Apply speed
			ForceDirection *= MovementForce;
			// Add force to this actor in a direction
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
			DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
		}

		DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Blue, false, 0.0f, 1.0f);
	}
}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// Check if not already self destructing
	if (!bSelfDestructing && !bExploded)
	{
		// Check if the overlapped actor is a player
		ASPlayer* PlayerPawn = Cast<ASPlayer>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
		{
			if (HasAuthority())
			{
				// Start a timer that inflicts damage to the bot until it explodes
				// This timer calls a function every 0.2f seconds and repeats
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDestructDelay, true, 0.0f);
			}
			
			// Play sound effect
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}


}