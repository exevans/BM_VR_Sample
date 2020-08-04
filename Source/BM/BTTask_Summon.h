// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Summon.generated.h"

/**
 * 
 */
UCLASS()
class BM_API UBTTask_Summon : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_Summon();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	float waitTime;
	int hand;
	bool valid;
	FVector TargetDir;
	bool done;
};
