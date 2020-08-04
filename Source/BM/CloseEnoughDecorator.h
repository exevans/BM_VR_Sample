// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "CloseEnoughDecorator.generated.h"

/**
 * 
 */
UCLASS()
class BM_API UCloseEnoughDecorator : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const override;

public:
	UCloseEnoughDecorator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BlackboardKeys)
	float AcceptableDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BlackboardKeys)
	float ClosestDistance;
};
