// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_SetRandomLocation.h"
#include "Enemy_AI_con.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTService_SetRandomLocation::UBTService_SetRandomLocation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "SetRandomLocation";
	bCreateNodeInstance = true;
	//bNotifyBecomeRelevant = true; 

	Interval = 2.f;    // Any value.
	RandomDeviation = 0.f;

}

void UBTService_SetRandomLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FNavLocation newLoc;
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	bool success = NavSystem->GetRandomReachablePointInRadius(OwnerComp.GetOwner()->GetActorLocation(), 900, newLoc);
	if (success)
	{
		UBlackboardComponent* CurrentBlackboard = OwnerComp.GetBlackboardComponent();
		AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
		enemy_con->targetLocation = newLoc.Location;


	}
}
