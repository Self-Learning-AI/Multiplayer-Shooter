// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "SGameMode.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	MaxHealth = 100.0f;
	bDead = false;

	// Set default team number to the highest it can be
	TeamNumber = 255;

	// Better to call SetISReplicatedByDefault;
	SetIsReplicatedByDefault(true);
	//SetReplicates(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Only server may modify health values
	// Components don't have a role so we need to use GetOwnerRole for the owning actor's role
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}

	CurrentHealth = MaxHealth;
	
}

void USHealthComponent::OnRep_CurrentHealth(float PreviousHealth)
{
	float Damage = PreviousHealth - CurrentHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bDead)
	{
		return;
	}
	
	// Check if actor that was shot is friendly
	// And its not self inflicted
	// TO DO: ADD FRIENDLY FIRE VARIABLE TO TOGGLE
	if (DamageCauser != DamagedActor && IsFriendly(DamageCauser, DamagedActor))
	{
		return;
	}

	// Use maths clamp to make sure health does not drop below 0
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);

	// Output the health variable to the on-screen-console
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Health Changed to: %f"), CurrentHealth));
	
	// Do a boolean check to see if actor is dead after damage is dealt
	bDead = CurrentHealth <= 0.0f;

	// Broadcast the health of the actor to clients
	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);


	// Check if actor has been killed and broadcast it
	if (bDead)
	{
		ASGameMode* GM = Cast<ASGameMode>(GetWorld()->GetAuthGameMode()); // Only valid on server
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
		}
	}
	
}

void USHealthComponent::Heal(float HealthGained)
{
	// Prevent dead players from gaining health
	if (CurrentHealth <= 0.0f)
	{
		return;
	}
	
	// Add health but clamp it to the max health value
	CurrentHealth = FMath::Clamp(CurrentHealth + HealthGained, 0.0f, MaxHealth);
	// We send a negative damage to the health so that it heals instead of damage
	OnHealthChanged.Broadcast(this, CurrentHealth, -HealthGained, nullptr, nullptr, nullptr);
}

void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// Replicates to any relevant client connected to server
	DOREPLIFETIME(USHealthComponent, CurrentHealth);
}

float USHealthComponent::GetHealth() const
{
	return CurrentHealth;
}

bool USHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
	// Assume that if one actor is not an actor or empty, they are friendly
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	// Get the health component of both actors
	USHealthComponent* HealthCompOfA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompOfB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));
	
	// Assume that if one of the actors does not have a health component, they are friendly
	if (HealthCompOfA == nullptr || HealthCompOfB == nullptr)
	{
		return true;
	}

	// Once error checks have passed, return a boolean value of the comparison
	return HealthCompOfA->TeamNumber == HealthCompOfB->TeamNumber;
}

