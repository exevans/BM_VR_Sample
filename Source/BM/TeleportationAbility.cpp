// Fill out your copyright notice in the Description page of Project Settings.

#include "TeleportationAbility.h"
#include "BMCharacter.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "TeleportationActor.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystemComponent.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "GameFramework/PawnMovementComponent.h"


ATeleportationAbility::ATeleportationAbility()
{
	teleport_loc = FVector(0, 0, 0);
	valid_teleport = false;
	//teleporting = false;
	current_state = Teleport_State::NONE;

	climbLedge = false;

	manaConsumptionType = ManaConsumptionCostType::Single;
	manaCost = 10.f;

	TeleportRate = 6000.f;
	teleportPlacementComponent = nullptr;
}

ATeleportationAbility::~ATeleportationAbility()
{
}

void ATeleportationAbility::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ATeleportationAbility, teleporting);
	DOREPLIFETIME(ATeleportationAbility, locked_teleport_loc);
}

void ATeleportationAbility::PostInit()
{

}

void ATeleportationAbility::EnterAbility()
{
	//teleporting = false;
	valid_teleport = false;

	current_state = Teleport_State::NONE;
}

void ATeleportationAbility::ExitAbility()
{
	//deactivate the spot system
	//only deactivate if in a valid state
	if (teleportPlacementComponent)
	{
		teleportPlacementComponent->Deactivate();
		teleportPlacementComponent = nullptr;
	}

	current_state = Teleport_State::NONE;
}

void ATeleportationAbility::Tick(float DeltaTime)
{
	if (current_state == Teleport_State::TELEPORTING)
	{
		UpdateMovingToTeleportPosition(DeltaTime);
	}
	else if (current_state == Teleport_State::PENDING_RELEASE)
	{
		UpdateFindingTeleportPosition();
	}
}

void ATeleportationAbility::OnInteract(EInputEvent eventType)
{
	if (eventType == EInputEvent::IE_Pressed)
	{
		AttemptActivateAbility();
	}
	else if (eventType == EInputEvent::IE_Released)
	{
		if (current_state == Teleport_State::PENDING_RELEASE && valid_teleport)
		{
			StartTeleporting();
		}

		//deactivate the spot system
		//only deactivate if in a valid state
		if (teleportPlacementComponent)
		{
			teleportPlacementComponent->Deactivate();
			teleportPlacementComponent = nullptr;
		}
	}
}

void ATeleportationAbility::OnInteractSecondary()
{
		//offer ability to other hand
		abilityManager->OfferAbility(hand, ATeleportationActor::StaticClass());
}

void ATeleportationAbility::AbilityActivated()
{
	//can now select a location
	current_state = Teleport_State::PENDING_RELEASE;

	//create the selection particle effect
	if (character)
		teleportPlacementComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PS_SelectTeleportSpot, FTransform());
}

void ATeleportationAbility::AbilityDeactivated()
{
	
}

void ATeleportationAbility::StartTeleporting()
{
	//lock our teleport location
	locked_teleport_loc = teleport_loc;

	//make material of enemy invisible
	if (ai_character)
	{
		//ai_character->GetMesh()->SetMaterial(0, ai_character->Cloak_Mat);
		//particles
		UParticleSystemComponent* particle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PS_BeginTeleport, ai_character->GetActorLocation());
		//particle->InstanceParameters.Add(param);
		particle->SetActorParameter("VertSurfaceActor", baseCharacter);

		trail = UGameplayStatics::SpawnEmitterAttached(PS_DuringTeleport, baseCharacter->GetCapsuleComponent());
		trail->SetActorParameter("VertSurfaceActor", baseCharacter);
	}

	//move quickly to the position
	if (ai_character)
	{
		FVector footOffset = FVector(0, 0, baseCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		locked_teleport_loc += footOffset;
	}

	baseCharacter->SetActorEnableCollision(false);

	//now moving to our tlocation
	current_state = Teleport_State::TELEPORTING;
}

void ATeleportationAbility::UpdateFindingTeleportPosition()
{
	//trace to where we can teleport
	FHitResult hitResult;
	FVector startTrace = GetCharacterHandLocation();
	FVector forwardVector = GetCharacterHandForwardVector();

	FVector endTrace = startTrace + (forwardVector * 2000.0f);
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(baseCharacter);

	//is the location pointed at valid
	valid_teleport = false;
	//will we need to climb a ledge
	climbLedge = false;
	//where we visually place teleport
	FVector visualTargetLoc;

	if (baseCharacter->GetWorld()->LineTraceSingleByChannel(hitResult, startTrace, endTrace, ECollisionChannel::ECC_Visibility, traceParams))
	{

		//check its a horizontal plane
		const FVector surfaceNormal = hitResult.Normal;
		if (FVector::DotProduct(surfaceNormal, FVector(0, 0, 1)) > 0.9)
		{
			//only show to controller
			if (baseCharacter->IsLocallyControlled())
			{
				visualTargetLoc = hitResult.Location;

				//DrawDebugCylinder(baseCharacter->GetWorld(), hitResult.Location, hitResult.Location + FVector(0, 0, 10), 60.0f, 16, FColor::Green, false);
				//DrawDebugLine(baseCharacter->GetWorld(), startTrace, hitResult.Location, FColor::Yellow);
			}

			if (character)
				teleport_loc = character->GetTeleportLocation(hitResult.Location);
			else
				teleport_loc = hitResult.Location;

			valid_teleport = true;
		}
		//new testing (condence down so all handled in character code) only allow walls if we know we can get on a ledge
		else if (FMath::Abs(FVector::DotProduct(surfaceNormal, FVector(0, 1, 0))) > 0.6 || FMath::Abs(FVector::DotProduct(surfaceNormal, FVector(1, 0, 0))) > 0.6)//check for ledges
		{
			FHitResult hitRes;
			FCollisionQueryParams colParams;

			FVector start = hitResult.Location - (surfaceNormal * 50) + FVector(0, 0, 40);
			FVector end = start - FVector(0, 0, 19);
			DrawDebugLine(baseCharacter->GetWorld(), start, end, FColor::Red);
			if (GetWorld()->SweepSingleByChannel(hitRes, start, end, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeSphere(20), colParams))
			{
				//check collided from top surface
				if (FVector::DotProduct(hitRes.Normal, FVector(0, 0, 1)) > 0.9)
				{
					//only show to controller
					if (baseCharacter->IsLocallyControlled())
					{
						//to look better always draw the citcle at the top so it's consistant
						FVector cylinderStart = FVector(hitResult.Location.X, hitResult.Location.Y, hitRes.ImpactPoint.Z);
						FVector cylinderEnd = cylinderStart - (surfaceNormal * 10.f);

						//DrawDebugCylinder(baseCharacter->GetWorld(), cylinderStart, cylinderEnd, 60.0f, 16, FColor::Green, false);
						//DrawDebugLine(baseCharacter->GetWorld(), startTrace, hitResult.Location, FColor::Red);

						visualTargetLoc = cylinderStart - FVector(0,0,20);
					}
					//teleport_loc = hitResult.Location;
					//UE_LOG(LogTemp, Warning, TEXT("EXE: raw loc =%d, %d, %d"), teleport_loc.X, teleport_loc.Y, teleport_loc.Z);
					teleport_loc = character->GetTeleportLocation(hitRes.Location);
					//UE_LOG(LogTemp, Warning, TEXT("EXE: converted loc =%d, %d, %d"), teleport_loc.X, teleport_loc.Y, teleport_loc.Z);

					valid_teleport = true;
					ledgeDirection = -hitRes.Normal;
					climbLedge = true;
				}
			}

			//only show to controller
			if (baseCharacter->IsLocallyControlled())
				DrawDebugLine(baseCharacter->GetWorld(), startTrace, hitResult.Location, FColor::Yellow);
		}

	}

	//process visual effect
	if (valid_teleport && teleportPlacementComponent)
	{
		teleportPlacementComponent->SetWorldLocation(visualTargetLoc);

		if (!teleportPlacementComponent->IsVisible())
			teleportPlacementComponent->SetVisibility(true);
	} else
	{
		//Don't show the visuals
		if (teleportPlacementComponent)
		{
			teleportPlacementComponent->SetVisibility(false);
		}
	}
}

void ATeleportationAbility::UpdateMovingToTeleportPosition(float DeltaTime)
{
	FVector newPos = FMath::VInterpConstantTo(baseCharacter->GetActorLocation(), locked_teleport_loc, DeltaTime, TeleportRate);
	baseCharacter->SetActorLocation(newPos);

	//check particle state
	if (ai_character && FVector::Distance(newPos, locked_teleport_loc) < 50)
	{
		//close enough to trigger the particel effect
		UParticleSystemComponent* particle = UGameplayStatics::SpawnEmitterAttached(PS_EndTeleport, baseCharacter->GetCapsuleComponent());
		particle->SetActorParameter("VertSurfaceActor", baseCharacter);
	}

	//reached edd point
	if (newPos == locked_teleport_loc)
	{
		current_state = Teleport_State::NONE;

		//turn collision back on
		baseCharacter->SetActorEnableCollision(true);

		//make visible again
		if (ai_character)
		{
			//ai_character->GetMesh()->SetMaterial(0, ai_character->Default_Mat);
		}
		//stop any trail effect
		if (trail)
		{
			trail->Deactivate();
		}

		//check if the character needs to climb
		/*if (climbLedge)
		{
			if (character)
				character->AttemptToClimbLedge(ledgeDirection);
			else
				ai_character->AttemptToClimbLedge(ledgeDirection);
			climbLedge = false;
		}*/
	}
}