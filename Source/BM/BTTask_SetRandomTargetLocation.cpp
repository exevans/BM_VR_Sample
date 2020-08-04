// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_SetRandomTargetLocation.h"
#include "Enemy_AI_con.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"

UBTTask_SetRandomTargetLocation::UBTTask_SetRandomTargetLocation()
{
	NodeName = "SetRandomTargetLocation";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_SetRandomTargetLocation::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	FNavLocation newLoc;
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	bool success = NavSystem->GetRandomReachablePointInRadius(OwnerComp.GetOwner()->GetActorLocation(), 900, newLoc);
	if (success)
	{
		UBlackboardComponent* CurrentBlackboard = OwnerComp.GetBlackboardComponent();
		AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
		enemy_con->targetLocation = newLoc.Location;

		//new, may break stuff otherwise have to wait for an update so location is set which takes ages
		CurrentBlackboard->SetValue<UBlackboardKeyType_Vector>(CurrentBlackboard->GetKeyID("TargetLocation"), newLoc.Location);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}