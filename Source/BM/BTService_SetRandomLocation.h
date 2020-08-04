// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetRandomLocation.generated.h"

/**
 * 
 */
UCLASS()
class BM_API UBTService_SetRandomLocation : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_SetRandomLocation(const FObjectInitializer& ObjectInitializer);
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
};
