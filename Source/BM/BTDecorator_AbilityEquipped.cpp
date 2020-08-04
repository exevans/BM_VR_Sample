// Fill out your copyright notice in the Description page of Project Settings.

#include "BTDecorator_AbilityEquipped.h"
#include "Enemy_AI_con.h"
#include "Enemy.h"
#include "BMAbilityManager.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_AbilityEquipped::UBTDecorator_AbilityEquipped()
{
	//bAllowAbortLowerPri = true;
	//bAllowAbortChildNodes = true;
	//FlowAbortMode = EBTFlowAbortMode::Self;
}

bool UBTDecorator_AbilityEquipped::CalculateRawConditionValue(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory) const
{
	AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
	if (enemy_con)
	{
		UBlackboardComponent* CurrentBlackboard = OwnerComp.GetBlackboardComponent();
		AEnemy* self = Cast<AEnemy>(enemy_con->GetPawn());
		
		bool abilityEquipped = false;

		if (ability == self->GetAbilityManager()->GetAbility(0) || ability == self->GetAbilityManager()->GetAbility(1))
			abilityEquipped = true;

		return abilityEquipped;

	}

	return false;
}

EBlackboardNotificationResult UBTDecorator_AbilityEquipped::OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
	if ((ChangedKeyID == CurrentAbilityL.GetSelectedKeyID()) || (ChangedKeyID == CurrentAbilityR.GetSelectedKeyID()))
	{
		Ability currL = static_cast<Ability>(Blackboard.GetValue<UBlackboardKeyType_Enum>(CurrentAbilityL.GetSelectedKeyID()));
		Ability currR = static_cast<Ability>(Blackboard.GetValue<UBlackboardKeyType_Enum>(CurrentAbilityR.GetSelectedKeyID()));

		if ((currL != ability) && (currR != ability))
			return EBlackboardNotificationResult::RemoveObserver;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}
