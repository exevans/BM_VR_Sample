// Fill out your copyright notice in the Description page of Project Settings.

#include "Teleport_AiAbility.h"
#include "components/SkeletalMeshComponent.h"
#include "NavigationSystem/Public/NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PawnMovementComponent.h"
#include "DrawDebugHelpers.h"

ATeleport_AiAbility::ATeleport_AiAbility()
{
	teleportInProgress = false;
}

ATeleport_AiAbility::~ATeleport_AiAbility()
{
}

void ATeleport_AiAbility::BeginAbility()
{
	UE_LOG(LogTemp, Warning, TEXT("EXE_AI: Begin teleport ability"));

	if (controller)
	{
		AEnemy* self = Cast<AEnemy>(controller->GetPawn());

		bool validLocation = false;

		socket = self->GetHandSocket(hand);

		//decide should we aim for a ledge or not
		bool ledge = FMath::RandBool();

		//try get a ledge location
		if (ledge)
		{
			//TEMP TEST THE HIGH POINTS
			TArray<AActor*> FoundActors;
			//get all actors that we can go to
			UGameplayStatics::GetAllActorsWithTag(controller->GetWorld(), FName("AITeleportPoint"), FoundActors);

			//randomise the order
			for (int i = 0; i < FoundActors.Num() - 2; ++i)
			{
				int j = FMath::RandRange(i, FoundActors.Num() - 1);
				FoundActors.Swap(i, j);
			}
			//select out point
			for (AActor* act : FoundActors)
			{
				//line of sight to the point
				FHitResult hitRes;
				FCollisionQueryParams colParams;
				colParams.AddIgnoredActor(self);
				if (self->GetWorld()->LineTraceSingleByChannel(hitRes, self->GetMesh()->GetSocketLocation(socket), act->GetActorLocation(), ECollisionChannel::ECC_Visibility, colParams))
					continue; //try a different one
				else
				{
					teleportPos = act->GetActorLocation();
					validLocation = true;
				}
			}
		}
		
		//try a normal land spot
		if (!validLocation)
		{
			//get a point to teleport to 
			FVector targetZone = self->GetActorLocation() + self->GetActorForwardVector() * 250.f;
			if (controller->currentEnemy)
				targetZone = controller->currentEnemy->GetActorLocation();

			//DrawDebugLine(GetWorld(), targetZone, targetZone + FVector(0, 0, 500.f), FColor::Green, true, 2.f);

			FNavLocation newLoc;
			UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(self->GetWorld());
			bool success = NavSystem->GetRandomReachablePointInRadius(targetZone, 300, newLoc);
			if (success)
			{
				teleportPos = newLoc.Location;
			}
			else
			{
				//aimat the target loc
				teleportPos = self->GetMesh()->GetSocketLocation(socket) + (((self->GetActorForwardVector() * FVector(1, 1, 0)) + FVector(0, 0, -0.2)) * 5000);
			}
		}
		

		//aim towards the point
		FVector aimDir = teleportPos - self->GetMesh()->GetSocketLocation(socket);
		self->SetAimVector(hand, aimDir);

		//don't execute in the same frame
		//begin charging 
		self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Pressed);

		waitTime = 0.f;
	}
}

void ATeleport_AiAbility::Tick(float DeltaSeconds)
{
	//check we have equiped
	AEnemy* self = Cast<AEnemy>(controller->GetPawn());

	//keep aiming at the same point as we were initially
	FVector aimDir = teleportPos - self->GetMesh()->GetSocketLocation(socket);
	self->SetAimVector(hand, aimDir);

	//wait about 0.5 second before activating
	if (self->GetAbilityManager()->IsCurrentAbilityReady(hand) &&(waitTime += DeltaSeconds) > 0.5)
	{
		waitTime = 0.f;

		//use the teleport
		self->GetAbilityManager()->Interact(hand, EInputEvent::IE_Released);

		//needs to complete the teleportation first
		teleportInProgress = true;
	}
	else if (teleportInProgress && self->GetAbilityManager()->IsCurrentAbilityReady(hand))
	{
		//teleportation is complete
		teleportInProgress = false;

		//remove the teleport request
		controller->ClearAbilityRequest((int)Ability::TELEPORT);
	}
}

void ATeleport_AiAbility::EndAbility()
{

}
