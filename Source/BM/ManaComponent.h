// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ManaComponent.generated.h"

class BMAbility;

/** Broadcasts when health is changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FManaComponentManaChangedSignature, float, current_health);

/** Broadcasts when health is changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FManaComponentManaEmptySignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BM_API UManaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UManaComponent();

	bool CanAffordAbility(int cost) const { return cost < current_mana; };
	void SpendMana(float cost);
	void BeginManaDrain(ABMAbility* id, float manaPerSec);
	void EndManaDrain(ABMAbility* id);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Gameplay)
		int32 max_mana;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Mana")
		float current_mana;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mana")
		int32 mana_regenRate;

	UPROPERTY(BlueprintAssignable, Replicated, Category = "EventDispatchers")
		FManaComponentManaChangedSignature OnManaChanged;

	UPROPERTY()//UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FManaComponentManaEmptySignature OnManaEmpty;

private:
	float drainPerSec;
	TMap<ABMAbility*, float> currentDrains;
};
