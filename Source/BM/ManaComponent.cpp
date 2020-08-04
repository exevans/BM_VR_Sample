// Fill out your copyright notice in the Description page of Project Settings.

#include "ManaComponent.h"
#include "BMAbility.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
UManaComponent::UManaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	max_mana = 100;
	mana_regenRate = 3;
	drainPerSec = 0.f;
}


// Called when the game starts
void UManaComponent::BeginPlay()
{
	Super::BeginPlay();

	current_mana = max_mana;
}

void UManaComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UManaComponent, max_mana);
	DOREPLIFETIME(UManaComponent, current_mana);
	//DOREPLIFETIME(UManaComponent, OnManaChanged);
}


// Called every frame
void UManaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (currentDrains.Num() > 0)
	{
		SpendMana(drainPerSec * DeltaTime);
	} else
	//try regenerating
	if (mana_regenRate > 0 && current_mana < max_mana)
	{
		float regenTick = mana_regenRate * DeltaTime;
		current_mana += regenTick;

		if (current_mana > max_mana)
			current_mana = max_mana;

		//broadcast there has been a change
		OnManaChanged.Broadcast(current_mana);
	}
}

void UManaComponent::SpendMana(float cost)
{
	//ensure we are actually spending mana
	if (cost <= 0)
		return;

	current_mana -= cost;

	if (current_mana < 0)
	{
		current_mana = 0;
		//broadcast we are out of mana
		OnManaEmpty.Broadcast();
	}

	OnManaChanged.Broadcast(current_mana);
}

void UManaComponent::BeginManaDrain(ABMAbility* id, float manaPerSec)
{
	currentDrains.Add(id, manaPerSec);
	drainPerSec += manaPerSec;
}

void UManaComponent::EndManaDrain(ABMAbility* id)
{
	if (currentDrains.Contains(id))
	{
		//get the amount this drained
		int amnt = currentDrains[id];
		drainPerSec -= amnt;

		//remove from map
		currentDrains.Remove(id);
	}

}