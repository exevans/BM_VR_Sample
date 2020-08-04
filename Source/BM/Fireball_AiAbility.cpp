// Fill out your copyright notice in the Description page of Project Settings.

#include "Fireball_AiAbility.h"
#include "Enemy.h"
#include "Engine/GameEngine.h"
#include "components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AFireball_AiAbility::AFireball_AiAbility()
{
	waitTime = 0.f;
}

AFireball_AiAbility::~AFireball_AiAbility()
{
}

void AFireball_AiAbility::BeginAbility()
{
	UE_LOG(LogTemp, Warning, TEXT("EXE_AI: Begin fireball ability"));
	if (controller)
	{
		AEnemy* self = Cast<AEnemy>(controller->GetPawn());

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Purple, "started executing attack task");
		self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Pressed);

	}
}

void AFireball_AiAbility::Tick(float DeltaSeconds)
{
	AEnemy* self = Cast<AEnemy>(controller->GetPawn());
	if (!self->GetAbilityManager()->IsCurrentAbilityReady(hand) || !controller->currentEnemy || abilityUseType == ABILITY_USE_TYPE::COMBINED)
		return;

	//wait 0.5 seconds before actually throwing it
	if ((waitTime += DeltaSeconds) < 0.3)
		return;

	FVector targetPos;
	AActor* targetActor = controller->currentEnemy;
	if (targetActor)
		targetPos = targetActor->GetActorLocation();//GetTargetLocation(self);
	else
		targetPos = controller->targetLocation;

	//add offset to the aiming
	FVector offset(0, 0, 0);
	offset.X = FMath::FRandRange(-20, 20);
	offset.Y = FMath::FRandRange(-20, 20);
	offset.Z = FMath::FRandRange(-30, 30);
	targetPos += offset;

	FVector aimVector = targetPos - self->GetHandLocation(hand);
	self->SetAimVector(hand, aimVector);

	UE_LOG(LogTemp, Warning, TEXT("EXE: fireball aimvec set by attack tick"));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("attack fireball setAimVector")));

	self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Released);

	//start charging again
	self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Pressed);

	//reset out wait time
	waitTime = 0.f;

	//loop forever we will abort if the state changes
}

void AFireball_AiAbility::EndAbility()
{

}