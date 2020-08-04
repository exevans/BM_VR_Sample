// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMAbilityManager.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_AbilityEquipped.generated.h"

/**
 * 
 */
UCLASS()
class BM_API UBTDecorator_AbilityEquipped : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const override;

public:
	UBTDecorator_AbilityEquipped();

	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BlackboardKeys)
	Ability ability;

	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector CurrentAbilityL;

	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector CurrentAbilityR;
};
