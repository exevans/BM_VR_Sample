// Fill out your copyright notice in the Description page of Project Settings.

#include "BMFireballAbility.h"
#include "BMCharacter.h"
#include "BMProjectile.h"
#include "Enemy.h"
#include "MotionControllerComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Camera/CameraComponent.h"       
#include "Components/SphereComponent.h"
#include "OscilateComponent.h"
#include "TimerManager.h"
#include "ManaComponent.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "components/SkeletalMeshComponent.h"
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "TimeDilationFieldActor.h"
#include "DrawDebugHelpers.h"
#include "FireballTargetableInterface.h"
#include "FollowingMine.h"
#include "Components/CapsuleComponent.h"
#include "BMShield.h"

ABMFireballAbility::ABMFireballAbility()
{
	proj = nullptr;
	attachedAbility = nullptr;
	respawnDelay = 0.25f;
	current_state = FireballState::EMPTY;
	layeredAbility = false;

	manaConsumptionType = ManaConsumptionCostType::Single;
	manaCost = 8.f;

	bReplicates = true;

	OscilateComp = nullptr;

	//waht abilities can we accept offers from
	validAbilitiesToAcceptOffers.Add(Ability::TIME);
	validAbilitiesToAcceptOffers.Add(Ability::SUMMON);
	validAbilitiesToAcceptOffers.Add(Ability::TELEPORT);
	validAbilitiesToAcceptOffers.Add(Ability::TRAP);
}

ABMFireballAbility::~ABMFireballAbility()
{
	/*if (proj)
	{
		proj->Destroy();
		proj = nullptr;
	}*/
}

void ABMFireballAbility::PostInit()
{

}

void ABMFireballAbility::EnterAbility()
{
	current_state = FireballState::EMPTY;
}

void ABMFireballAbility::ExitAbility()
{
	if (proj)
	{
		proj->Destroy();
		proj = nullptr;
	}

	baseCharacter->GetWorld()->GetTimerManager().ClearTimer(spawnDelayTimerHandle);

	current_state = FireballState::EMPTY;
}

void ABMFireballAbility::Tick(float DeltaTime)
{
	if (current_state == FireballState::CHARGING_SINGLE)
	{
		UpdateSingleFireball();
	}
	else if (current_state == FireballState::CHARGING_COMBINED || current_state == FireballState::READY_COMBINED)
	{
		UpdateCombinedFireball();
	}
}

void ABMFireballAbility::OnInteract(EInputEvent eventType)
{
	if (eventType == IE_Pressed)
	{
		AttemptActivateAbility();
	}
	else if (eventType == IE_Released)
	{
		if (proj == nullptr /*&& midProj == nullptr*/)
		{
			if (current_state == FireballState::COMBINED_SECONDARY)
			{
				abilityManager->StopCombining();
			}
			return;
		}
		else if (current_state == FireballState::READY_COMBINED)//midProj)
		{
			ActivateCombinedFireball();
			return;
		}

		if (current_state == FireballState::READY_SINGLE)//fireballReady)
		{
			//stop the oscilating
			OscilateComp->DestroyComponent();

			proj->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

			//colliding
			proj->GetCollisionComp()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			proj->SetLifeSpan(8.0f);
			
			//testing for throwable fireballs //NEW
			FVector linearVel(0,0,0); 

			if (character)
			{
				if (hand == 0)
					linearVel = character->LeftConRelVelRaw;
				else
					linearVel = character->RightConRelVelRaw;
			}
			else
			{
				linearVel = GetCharacterHandForwardVector();
			}

			UE_LOG(LogTemp, Warning, TEXT("linear vel b: %f, %f, %f"), linearVel.X, linearVel.Y, linearVel.Z);

			linearVel.Normalize();

			UE_LOG(LogTemp, Warning, TEXT("linear vel a: %f, %f, %f"), linearVel.X, linearVel.Y, linearVel.Z);

			UProjectileMovementComponent* movComp = proj->GetProjectileMovement();
			movComp->Velocity = linearVel * movComp->MaxSpeed;
			movComp->ProjectileGravityScale = 0.0f;

			//send to clients the initial velocity so they can start moving now
			proj->Client_UpdateVelocity(proj->GetActorLocation(), movComp->Velocity);


			TArray<AActor*> FoundActors;
			//get all actors that we can home onto
			UGameplayStatics::GetAllActorsWithTag(baseCharacter->GetWorld(), FName("Targetable"), FoundActors);
			
			float bestAngle = 360.f;
			AActor* bestAct = nullptr;
			for (AActor* act : FoundActors)
			{
				//check we arn't checking ourselves
				if (act == baseCharacter)/* || act->Instigator == baseCharacter)*/
					continue;

				//check we can actually see any part of the actor
				if (character && !act->WasRecentlyRendered(1.f))
					continue;

				FVector dirToPawn = act->GetTargetLocation() - proj->GetActorLocation();
				dirToPawn.Normalize();
					//check the angle 
				float dot = FVector::DotProduct(dirToPawn, linearVel);
				float length = dirToPawn.Size() * linearVel.Size();
				float deg = FMath::RadiansToDegrees(FMath::Acos(dot/length));

				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("get angle betweeb: %d"), (int)deg));
				if (deg < bestAngle && deg < 10.f)
				{
					bestAct = act;
					bestAngle = deg;
				}
			}

			if (bestAct)
			{
				movComp->HomingTargetComponent = bestAct->GetRootComponent();
				movComp->HomingAccelerationMagnitude = 4000.f;
			}
			//stop any feedback
			//UGameplayStatics::GetPlayerController(character->GetWorld(), 0)->ClientStopForceFeedback(character->feedbackClass, "Test");

			//activate any actors attached to the projectile
			if (attachedAbility)
			{
				attachedAbility->OnActivated();
			}
		}
		else {
			proj->Destroy();
		}


		baseCharacter->GetWorld()->GetTimerManager().ClearTimer(spawnDelayTimerHandle);
		proj = nullptr;
		OscilateComp = nullptr;
		attachedAbility = nullptr;
		layeredAbility = false;
		current_state = FireballState::EMPTY;
	}
}

void ABMFireballAbility::OnInteractSecondary()
{
	// offer the ability
	abilityManager->OfferAbility(hand, ABMProjectile::StaticClass());
}

void ABMFireballAbility::AddAttachedAbility(UClass* attClass)
{
	if (proj && !layeredAbility)
	{
		if (attClass->IsChildOf(AFollowingMine::StaticClass()))
		{
			//no need to bother with the projectile just spawn the follower
			FVector handLoc = GetCharacterHandLocation();

			FActorSpawnParameters spawnParams;
			spawnParams.Instigator = baseCharacter;
			AFollowingMine* follower = GetWorld()->SpawnActor<AFollowingMine>(attClass, handLoc, FRotator::ZeroRotator, spawnParams);
		}
		else
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AActor* dil = GetWorld()->SpawnActor<AActor>(attClass, proj->GetActorLocation(), proj->GetActorRotation(), ActorSpawnParams);
			dil->Instigator = baseCharacter;
			dil->AttachToActor(proj, FAttachmentTransformRules::KeepWorldTransform);

			//bind destroy function function
			attachedAbility = Cast<AAttachedAbilityActor>(dil);
			proj->OnDestroyed().AddUObject(attachedAbility, &AAttachedAbilityActor::OnParentDestroyed);

			//attached->InitAttach(Ability::TRAP);*/
		}

		layeredAbility = true;
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("attached a thing on it"));
	}
}

void ABMFireballAbility::ReadyFireball()
{
	current_state = FireballState::READY_SINGLE;
	//UGameplayStatics::GetPlayerController(character->GetWorld(), 0)->ClientPlayForceFeedback(character->feedbackClass, false, true, FName("Test"));
}

void ABMFireballAbility::SpawnFireball()
{
	//spawn the projectile above the hand
	if (ProjectileClass != NULL)
	{
		UWorld* const World = baseCharacter->GetWorld();
		if (World != NULL)
		{
			//spawn the fireball
			const FRotator SpawnRotation = baseCharacter->GetControlRotation();
			const FVector SpawnLocation = GetCharacterHandLocation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.Instigator = baseCharacter;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			proj = World->SpawnActor<ABMProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);


			//add an oscilation component for a little jiggle
			OscilateComp = NewObject<UOscilateComponent>(proj, UOscilateComponent::StaticClass());
			OscilateComp->RegisterComponent();
			OscilateComp->maxMovementDistance = FVector(0, 0, 1);
			OscilateComp->OscilateSpeed = 3.f;


			if (proj)
			{
				//deactivate colision
				proj->GetCollisionComp()->IgnoreActorWhenMoving(baseCharacter, true);
				proj->GetCollisionComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				baseCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(proj, true);
				//ignore all actor children (shield) too
				TArray<AActor*> children;
				baseCharacter->GetAllChildActors(children);
				for (AActor* child : children)
				{
					proj->GetCollisionComp()->IgnoreActorWhenMoving(child, true);
				}

				//test - get all shield instances
				TArray<AActor*> shields;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABMShield::StaticClass(), shields);
				for (AActor* shield : shields)
				{
					//belongs to us so don't collide our shields
					if (shield->GetOwner() == baseCharacter)
					{
						proj->GetCollisionComp()->IgnoreActorWhenMoving(shield, true);
						ABMShield* shieldInst = Cast<ABMShield>(shield);
						shieldInst->MeshComp->IgnoreActorWhenMoving(proj, true);
					}
				}


				UProjectileMovementComponent* movComp = proj->GetProjectileMovement();

				//attach to hand
				if (movComp != nullptr)
				{
					movComp->Velocity = FVector(0, 0, 0);
					movComp->ProjectileGravityScale = 0.0f;
					proj->SetLifeSpan(0);

					if (character)
					{
						proj->AttachToComponent(character->GetMotionController(hand), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						proj->SetActorRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
					}
					else 
					{
						proj->AttachToComponent(ai_character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, ai_character->GetHandSocket(hand));
						UE_LOG(LogTemp, Warning, TEXT("the socket exists"));
					}
				}
			}
		}
	}
}

void ABMFireballAbility::OnCombinedCast()
{
	//check it doesn't already exist
	if (current_state == FireballState::CHARGING_COMBINED)
		return;

	//get middle position between hands
	FVector midSpot = (character->GetHandLocation(0) + character->GetHandLocation(1)) / 2;

	initialHandDist = FVector(character->GetHandLocation(0) - character->GetHandLocation(1)).Size();

	//spawn the ball
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActorSpawnParams.Instigator = character;
	proj = character->GetWorld()->SpawnActor<ABMProjectile>(ProjectileClass, midSpot, character->GetActorRotation(), ActorSpawnParams);
	proj->GetCollisionComp()->SetWorldScale3D(FVector(0.1, 0.1, 0.1));

	//deaactivate colision
	proj->GetCollisionComp()->IgnoreActorWhenMoving(character, true);
	proj->GetCollisionComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	character->GetCapsuleComponent()->IgnoreActorWhenMoving(proj, true);

	//stop any movement
	UProjectileMovementComponent* movComp = proj->GetProjectileMovement();
	if (movComp != nullptr)
	{
		movComp->Velocity = FVector(0, 0, 0);
		movComp->ProjectileGravityScale = 0.0f;
		proj->SetLifeSpan(0);
	}

	current_state = FireballState::CHARGING_COMBINED;
}

void ABMFireballAbility::OnEndCombinedCast()
{
	if (proj)
	{
		proj->Destroy();
		proj = nullptr;
	}

	//if we are full size then we should split into completed fireballs TEMP
	SpawnFireball();
	current_state = FireballState::CHARGING_SINGLE;

	//begin charge countdown
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &ABMFireballAbility::ReadyFireball);
	baseCharacter->GetWorld()->GetTimerManager().SetTimer(spawnDelayTimerHandle, TimerDel, respawnDelay, false); 
}

void ABMFireballAbility::PrepareForCombine()
{
	if (proj)
	{
		proj->Destroy();
		proj = nullptr;
	}

	current_state = FireballState::EMPTY;
	if (hand == 1)
		current_state = FireballState::COMBINED_SECONDARY;
	character->GetWorld()->GetTimerManager().ClearTimer(spawnDelayTimerHandle);
}

bool ABMFireballAbility::CanAbilityCombine()
{
	//need to have a proj existing
	if (proj)
		return true;
	else
		return false;
}

void ABMFireballAbility::UpdateCombinedFireball()
{
	//check if we are combining
	if (!proj)
		return;

	//get the dist between controllers
	float dist = FVector(character->GetHandLocation(0) - character->GetHandLocation(1)).Size();
	if (dist > 80)
	{
		//distance is too big
		abilityManager->StopCombining();
		return;
	}

	//move to the correct midpoint position
	FVector midSpot = (character->GetHandLocation(0) + character->GetHandLocation(1)) / 2;
	proj->SetActorLocation(midSpot);

	if (dist < initialHandDist)
		initialHandDist = dist;

	float deltaDist = dist - initialHandDist;
	if (deltaDist < 0)
		deltaDist = 0;

	float size = deltaDist / 25;
	size += 0.1;

	if (size > 1) {
		size = 1;
		current_state = FireballState::READY_COMBINED;
	}

	if (size < 1) {
		current_state = FireballState::CHARGING_COMBINED;
	}

	if (current_state != FireballState::READY_COMBINED) //!fireballReady)
	{
		//size fireball based on distance between hands
		proj->GetCollisionComp()->SetWorldScale3D(FVector(size, size, size));
		proj->GetCollisionComp()->SetRelativeScale3D(FVector(size, size, size));
	}
}

void ABMFireballAbility::UpdateSingleFireball()
{
	float elapsed = baseCharacter->GetWorld()->GetTimerManager().GetTimerElapsed(spawnDelayTimerHandle);
	float size = elapsed / respawnDelay;

	proj->GetCollisionComp()->SetWorldScale3D(FVector(size, size, size));
	proj->GetCollisionComp()->SetRelativeScale3D(FVector(size, size, size));
}

void ABMFireballAbility::ActivateCombinedFireball()
{
	if (current_state == FireballState::READY_COMBINED)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("entered the ready bit"));
		//full size
		proj->GetCollisionComp()->SetWorldScale3D(FVector(1, 1, 1));
		proj->GetCollisionComp()->SetRelativeScale3D(FVector(1, 1, 1));

		//release the fireball if its fully charged
		UProjectileMovementComponent* movComp = proj->GetProjectileMovement();
		movComp->ProjectileGravityScale = 0.f;
		proj->SetLifeSpan(5.0f);

		proj->GetCollisionComp()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		//temp just the player
		if (character->IsA<ABMCharacter>())
		{
			FVector dir = Cast<ABMCharacter>(character)->GetMotionController(0)->GetForwardVector();
			dir.Normalize();
			dir = dir.RotateAngleAxis(90, character->GetHandForwardVector(0));

			FVector dir2 = Cast<ABMCharacter>(character)->GetMotionController(1)->GetForwardVector();
			dir2.Normalize();
			dir2 = dir2.RotateAngleAxis(-90, character->GetHandForwardVector(1));
			FVector dirAv = (dir + dir2) / 2;
			//dirAv.Normalize();
			movComp->Velocity = dirAv * movComp->MaxSpeed;

			//stop any feedback
			//UGameplayStatics::GetPlayerController(character->GetWorld(), 0)->ClientStopForceFeedback(character->feedbackClass, "Test");



			//fire the beam test
			TArray<FHitResult> hitResults;
			FVector start = proj->GetActorLocation();
			FVector end = start + dirAv * 5000.f;
			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActor(baseCharacter);
			bool hit = GetWorld()->LineTraceMultiByChannel(hitResults, start, end, ECollisionChannel::ECC_WorldStatic, queryParams);

			if (hit)
				end = hitResults[hitResults.Num() - 1].ImpactPoint;
			//do the beam to the max point
			//lightningComponent->SetBeamTargetPoint(0, endLoc, 0);
			//only stays for 0.5 seconds
		}
	}
	else {
		proj->Destroy();
	}

	character->GetWorld()->GetTimerManager().ClearTimer(spawnDelayTimerHandle);
	proj = nullptr;
	current_state = FireballState::EMPTY;
	layeredAbility = false;
}

bool ABMFireballAbility::IsAbilityReady()
{
	return current_state == FireballState::READY_COMBINED || current_state == FireballState::READY_SINGLE;
}

void ABMFireballAbility::AbilityActivated()
{
	//check we arn't already charging
	if (current_state != FireballState::EMPTY)
		return;

	//begins charging the fireball
	SpawnFireball();
	current_state = FireballState::CHARGING_SINGLE;

	//begin charge countdown
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &ABMFireballAbility::ReadyFireball);
	baseCharacter->GetWorld()->GetTimerManager().SetTimer(spawnDelayTimerHandle, TimerDel, respawnDelay, false);

	//check if we can combine hands
	if (abilityManager->CanHandsCombine())
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Hands are combining the spell"));
		abilityManager->StartCombining();
	}
}

void ABMFireballAbility::AbilityDeactivated()
{
	//call the base method
	ABMAbility::AbilityDeactivated();

}

