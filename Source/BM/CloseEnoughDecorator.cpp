// Fill out your copyright notice in the Description page of Project Settings.

#include "CloseEnoughDecorator.h"
#include "Enemy_AI_con.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"

UCloseEnoughDecorator::UCloseEnoughDecorator()
{
	AcceptableDistance = 1000.f;
	ClosestDistance = 200.f;
}

bool UCloseEnoughDecorator::CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const
{
	AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());

	if (enemy_con)
	{
		UBlackboardComponent* CurrentBlackboard = OwnerComp.GetBlackboardComponent();

		AActor* targetAct = Cast<AActor>(CurrentBlackboard->GetValue<UBlackboardKeyType_Object>(CurrentBlackboard->GetKeyID("TargetToFollow")));
		FVector diff = enemy_con->GetPawn()->GetActorLocation() - targetAct->GetActorLocation();

		float dist = diff.Size();
		bool CloseEnough = dist <= AcceptableDistance;

		//temp if we need to back off ad a location
		/*if (dist <  ClosestDistance)
		{
			//FVector backOffLoc = targetAct->GetActorLocation() + (diff.Normalize() * AcceptableDistance / 2);
			//enemy_con->targetLocation = backOffLoc;
			//CurrentBlackboard->SetValue<UBlackboardKeyType_Vector>(CurrentBlackboard->GetKeyID("TargetLocation"), backOffLoc);
		}*/

		return CloseEnough;
	}

	return false;
}