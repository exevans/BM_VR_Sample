// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetRandomTargetLocation.generated.h"

/**
 * 
 */
UCLASS()
class BM_API UBTTask_SetRandomTargetLocation : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SetRandomTargetLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) override;
};
