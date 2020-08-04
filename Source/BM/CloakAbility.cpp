// Fill out your copyright notice in the Description page of Project Settings.

#include "CloakAbility.h"
#include "BMCharacter.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "TeleportationActor.h"
#include "Engine/GameEngine.h"
#include "Enemy.h"
#include "PlacementTrapActor.h"
#include "BMCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInterface.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"

ACloakAbility::ACloakAbility()
{
	manaConsumptionType = ManaConsumptionCostType::Constant;
	manaCost = 10.f;

	dynamicMatL = nullptr;
	dynamicMatR = nullptr;

	enabled = false;
	localCanBeSeen = true;
}

void ACloakAbility::PostInit()
{

}

void ACloakAbility::EnterAbility()
{
	
}

void ACloakAbility::ExitAbility()
{
	AbilityDeactivated();
}

void ACloakAbility::Tick(float DeltaTime)
{
	if (enabled)
	{
		//calculate the amount of movement taking place
		float movement;
		
		if (character)
			movement = character->LeftConRelVel.Size() + character->RightConRelVel.Size();
		else
			movement = ai_character->GetVelocity().Size();

		float targetRefrac;
		if (character)
			targetRefrac = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 2.5f), FVector2D(1.012, 1.6), movement);
		else
			targetRefrac = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 200), FVector2D(1.012, 1.1f), movement);

		currentRefraction = FMath::FInterpTo(currentRefraction, targetRefrac, DeltaTime, 12);

		UE_LOG(LogTemp, Warning, TEXT("EXE: movement =%f"), movement);

		if (dynamicMatL != nullptr)
			dynamicMatL->SetScalarParameterValue("refraction", currentRefraction);
		if (dynamicMatR != nullptr)
			dynamicMatL->SetScalarParameterValue("refraction", currentRefraction);

		if (character)
		{
			//update whether the ai can view us
			float minVisible = 0.45;
			if (movement > minVisible && !localCanBeSeen)
			{
				localCanBeSeen = true;
				character->SetCanBeSeen(localCanBeSeen);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Player can now be seen!"));
			}
			else if (movement <= minVisible && localCanBeSeen)
			{
				localCanBeSeen = false;
				character->SetCanBeSeen(localCanBeSeen);
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Player Invisible !"));
			}
		}
	}
}

void ACloakAbility::OnInteract(EInputEvent eventType)
{
	//check if we are touching an object that can be cloaked
	bool itemFound = false;

	if (character)
	{
		TArray<FHitResult> outRes;
		USphereComponent* handInteractComp = character->HandInteract_Left;
		if (hand == 1)
			handInteractComp = character->HandInteract_Right;

		FCollisionQueryParams queryParams;
		queryParams.AddIgnoredActor(baseCharacter);

		GetWorld()->SweepMultiByChannel(outRes, handInteractComp->GetComponentLocation(), handInteractComp->GetComponentLocation(), FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(handInteractComp->GetScaledSphereRadius()), queryParams);
		//find suitable match
		for (FHitResult& res : outRes)
		{
			AActor* actor = res.GetActor();
			UPrimitiveComponent* comp = res.GetComponent();

			//check if we can cloak the object (for now see if it's a trap)
			if (Cast<APlacementTrapActor>(actor))
			{
				//match now cloak it
				APlacementTrapActor* trap = Cast<APlacementTrapActor>(actor);
				trap->SetActorHiddenInGame(true);
				itemFound = true;
				break;
			}
		}
	}

	if (!itemFound)
	{
		if (eventType == EInputEvent::IE_Pressed)
		{
			if (!enabled)
			{
				AttemptActivateAbility();
			}
			else
			{
				AbilityDeactivated();
			}
		}
	}
}

void ACloakAbility::OnInteractSecondary()
{

}

void ACloakAbility::AbilityActivated()
{
	enabled = true;
	//turn invisible

	if (character)
	{
		character->SetCanBeSeen(false);
		//add material to hands
		//character->HandMesh_Left->SetMaterial(0, invisibility);
		//character->HandMesh_Right->SetMaterial(0, invisibility);

		dynamicMatL = character->HandMesh_Left->CreateDynamicMaterialInstance(0, invisibility);
		dynamicMatR = character->HandMesh_Right->CreateDynamicMaterialInstance(0, invisibility);

		HighlightObjectsOfInterest(true);
	}
	else
	{
		dynamicMatL = ai_character->GetMesh()->CreateDynamicMaterialInstance(0, invisibility);
	}

}

void ACloakAbility::AbilityDeactivated()
{
	ABMAbility::AbilityDeactivated();
	
	enabled = false;

	if (character)
	{
		//can be seen by ai
		character->SetCanBeSeen(true);

		//rset material to hands
		character->HandMesh_Left->SetMaterial(0, character->default_mat);
		character->HandMesh_Right->SetMaterial(0, character->default_mat);

		HighlightObjectsOfInterest(false);

		dynamicMatL = nullptr;
		dynamicMatR = nullptr;
	}
	else
	{
		ai_character->GetMesh()->SetMaterial(0, ai_character->Default_Mat);
		dynamicMatL = nullptr;
	}

	currentRefraction = 0;
}

void ACloakAbility::HighlightObjectsOfInterest(bool newState)
{
	//test hide all enemies //todo add delegate for when a new object of interest spawn in
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), FoundActors);
	for (AActor* act : FoundActors)
	{
		AEnemy* enemy = Cast<AEnemy>(act);
		enemy->GetMesh()->SetRenderCustomDepth(newState);
	}
}