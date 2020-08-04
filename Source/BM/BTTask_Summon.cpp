// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Summon.h"
#include "Enemy_AI_con.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "Enemy.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Blueprint/AIAsyncTaskBlueprintProxy.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "components/SkeletalMeshComponent.h"

UBTTask_Summon::UBTTask_Summon()
{
	NodeName = "Summon";
	bNotifyTick = true;

	valid = true;
	waitTime = 0.f;
	done = false;
}

EBTNodeResult::Type UBTTask_Summon::ExecuteTask(UBehaviorTreeComponent & OwnerComp, uint8 * NodeMemory)
{
	AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
	if (enemy_con)
	{
		AEnemy* self = Cast<AEnemy>(enemy_con->GetPawn());

		//done = false;
		//waitTime = 0.f;
		//valid = true;

		//see if there is object in the area we can throw
		FHitResult hitResult;
		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(self);
		if (self->GetWorld()->SweepSingleByChannel(hitResult, self->GetActorLocation(), self->GetActorLocation(), FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel2, FCollisionShape::MakeSphere(500.f), queryParams))
		{
			//for now just use first result
			AActor* actor = hitResult.GetActor();
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Purple, FString::Printf(TEXT("%s set ai found %s, comp = %s"), *actor->GetName(), *actor->GetName(), *hitResult.GetComponent()->GetName()));

			//point our hand towards it 
			hand = enemy_con->GetHandWithAbility((int)Ability::SUMMON);

			if (hand == -1)
				return EBTNodeResult::Failed;
			FName socket = "hand_lSocket";
			if (hand == 1)
				socket = "hand_rSocket";

			//aim at the target loc
			FVector aimDir = actor->GetActorLocation() - self->GetMesh()->GetSocketLocation(socket);
			self->SetAimVector(hand, aimDir);

			TargetDir = aimDir;
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Purple, "set ai ability to summon!");
			UE_LOG(LogTemp, Warning, TEXT("EXE: the summon task just set the aimVector in execute: %s"), *actor->GetName());

			//we are now grabbing the object move it
			//self->GetAbilityManager()->Interact(1, EInputEvent::IE_Pressed);
			//self->GetAbilityManager()->Interact(1, EInputEvent::IE_Released);

			//aim upwards
			//aimDir += FVector(0, 0, 100);
			//self->SetAimVector(hand, aimDir);
			//prevent being interuptted
			enemy_con->lockChanges = true;

			return EBTNodeResult::InProgress;
		}
	}
	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_Summon::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//if we are curently holding then release it
	/*if (!valid)
	{
		AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
		AEnemy* self = Cast<AEnemy>(enemy_con->GetPawn());
		self->GetAbilityManager()->InteractSecondary(hand);
	}*/
	return EBTNodeResult::Succeeded;
}

void UBTTask_Summon::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (done)
		return;

	AEnemy_AI_con* enemy_con = Cast<AEnemy_AI_con>(OwnerComp.GetAIOwner());
	AEnemy* self = Cast<AEnemy>(enemy_con->GetPawn());
	UBlackboardComponent* CurrentBlackboard = OwnerComp.GetBlackboardComponent();

	//check we have equiped 
	//if (self->GetAbilityManager()->GetAbility(hand) != Ability::SUMMON)
		//return;

	//wait about 1 second before activating
	if (valid && (waitTime += DeltaSeconds) > 0.5)
	{
		valid = false;
		waitTime = 0.f;

		//we are now grabbing the object move it
		self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Pressed);
		UE_LOG(LogTemp, Warning, TEXT("EXE: the summon task is grabbing"));

		//pull it in
		self->GetAbilityManager()->InteractSecondary(hand);

		TargetDir = self->GetActorForwardVector();
		AActor* targetActor = Cast<AActor>(CurrentBlackboard->GetValue<UBlackboardKeyType_Object>(CurrentBlackboard->GetKeyID("TargetToFollow")));
		if (targetActor)
		{
			TargetDir = targetActor->GetActorLocation() - self->GetHandLocation(hand);
			TargetDir.Normalize();
		}
	}
	
	if (!valid)
	{
		FVector curr = FMath::VInterpTo(self->GetHandForwardVector(hand), TargetDir, DeltaSeconds, 3);
		self->SetAimVector(hand, curr);

		if (FVector::PointsAreNear(curr, TargetDir, 0.1))
		{
			//if ((waitTime += DeltaSeconds) > 0.5)
			{
				self->GetAbilityManager()->InteractSecondary(hand);
				//self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Pressed);
				//done = true;0
				UE_LOG(LogTemp, Warning, TEXT("EXE: the summon task reached the exit point"));

				valid = true;
				waitTime = 0.f;
				done = false;

				enemy_con->lockChanges = false;

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}

