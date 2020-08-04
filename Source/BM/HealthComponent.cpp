// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Engine/GameEngine.h"
#include "Perception/AISense_Damage.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;

	// ...
	alive = false;
	max_health = 100;
	TotConstDmagPerSec = 0.f;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, current_health);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(false);

	current_health = max_health;

	//check initially not already dead
	if (current_health > 0)
	{
		alive = true;
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamage);
	}
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//update the damage (constnt damage)
	float frameDamage = TotConstDmagPerSec * DeltaTime;
	UpdateDamage(frameDamage);
}

void UHealthComponent::Reset()
{
	alive = true;
	current_health = max_health;
	OnHealthChanged.Broadcast(current_health);
}

void UHealthComponent::UpdateDamage(float damage)
{
	//only called when constant damage is applied or instant damage

	if (!alive)
		return;

	if ((current_health -= damage) <= 0)
	{
		SetDead();
	}

	//check if we hit a health bondary
	for (const TPair<int, FHealthComponentHealthBoundarySignature>& pair : boundaryDelagatesMap)
	{
		//have we crossed a boundary, comp new and old, has the sign changed 
		float healthPercentOld = 100 * (float(current_health + damage) / (float)max_health);
		float healthPercentNew = 100 * (float)current_health / (float)max_health;
		if ((pair.Key > healthPercentOld) != (pair.Key > healthPercentNew))
		{
			pair.Value.Broadcast(pair.Key, pair.Key > healthPercentNew);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("EXE: Dam health has changed %d"), (int)current_health);
	OnHealthChanged.Broadcast(current_health);

	//inform all clients of the health change
	//if (GetOwnerRole() == ROLE_Authority)
		//ClientUpdateHealth(current_health);
}

void UHealthComponent::SetDead()
{
	current_health = 0;
	alive = false;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Dead"));

	OnHealthDead.Broadcast(GetOwner());
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("EXE: Dam taken damage %d"), (int)Damage);
	
	UpdateDamage(Damage);
}

void UHealthComponent::AddConstantDamage(void* key, float dmgPerSec, AActor* DamageCauser)
{
	//add to damage to apply over each second
	//should not already be in map
	if (!constDmgMap.Contains(key))
	{
		constDmgMap.Add(key, dmgPerSec);
		TotConstDmagPerSec += dmgPerSec;

		//start ticking as there will be per frame changes
		if (constDmgMap.Num() == 1)
		{
			SetComponentTickEnabled(true);
		}

		//report that damage has started
		//UAISense_Damage::ReportDamageEvent(GetWorld(), GetOwner(), Instigator ? Instigator : nullptr, 0, GetActorLocation(), Hit.ImpactPoint);
	}
}

void UHealthComponent::RemoveConstantDamage(void* key)
{
	if (constDmgMap.Contains(key))
	{
		TotConstDmagPerSec -= constDmgMap[key];
		constDmgMap.Remove(key);

		if (constDmgMap.Num() == 0)
			SetComponentTickEnabled(false);
	}
}

void UHealthComponent::ClientUpdateHealth_Implementation(int newHealth)
{
	OnHealthChanged.Broadcast(newHealth);
}

float UHealthComponent::GetRemainingHealthProportion() const
{
	return float(current_health) / float(max_health);
}

FHealthComponentHealthBoundarySignature& UHealthComponent::GetHealthBoundaryDelagate(int percentBoundary)
{
	return boundaryDelagatesMap.FindOrAdd(percentBoundary);
}