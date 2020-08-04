// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseCharacter.h"
#include "BMProjectile.h"
#include "HealthComponent.h"
#include "components/SkeletalMeshComponent.h"
#include "components/CapsuleComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/GameEngine.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "GameFramework/DamageType.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TimerManager.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Damage.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("Health_comp"));

	physicsHandle_Left = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandleL"));
	physicsHandle_Right = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandleR"));
	
	/*physicsHandle_Left->bSoftLinearConstraint = false;
	physicsHandle_Right->bSoftLinearConstraint = false;
	physicsHandle_Left->bSoftAngularConstraint = false;
	physicsHandle_Right->bSoftAngularConstraint = false;*/

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABaseCharacter::OnHit);		// set up a notification for when this component hits something blocking
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::OnBeginOverlap);

	//ai oriented -- move to ai??
	actorToblock = nullptr;

	//blend amount for shield hand location -- move to ai??
	leftHandAlpha = 0.f;
	rightHandAlpha = 0.f;

	//fireballs lock
	Tags.Add(FName("Targetable"));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Location = GetMesh()->GetSocketLocation("headSocket");
	Rotation = GetMesh()->GetSocketTransform("headSocket").Rotator();
}

void ABaseCharacter::StartRagdoll()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetAllBodiesSimulatePhysics(true);

	//stop from still animating
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationCustomMode);
}

void ABaseCharacter::StopRagdoll()
{
	//check we arn't dead or we bring back a corpse
	if (!HealthComponent->IsAlive())
		return;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GetMesh()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);
	GetMesh()->SetAllBodiesPhysicsBlendWeight(0);

	//move the mesh so its back in the capsule
	GetCapsuleComponent()->SetWorldLocation(GetMesh()->GetComponentLocation() + FVector(0,0,GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
	GetMesh()->AttachTo(GetCapsuleComponent(), NAME_None, EAttachLocation::SnapToTarget);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -85));
	GetMesh()->SetRelativeRotation(FRotator(0,-90,0));

	//start animating again
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

void ABaseCharacter::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("EXE: baseChar hit"));
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if (OtherActor->IsA(ABMProjectile::StaticClass()))
		{
			//this is handled in the projectiles
		}
		else {

			//do we notify the perception system
			if (Controller != nullptr)
			{
				if (Cast<ACharacter>(OtherActor) != nullptr)
				{
					UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this);
					if (PerceptionSystem)
					{
						FAITouchEvent Event(Controller, OtherActor, Hit.ImpactPoint);
						PerceptionSystem->OnEvent(Event);
					}
				}
			}

			//apply damage from objects
			//get the speed of the hit
			float mass = OtherComp->CalculateMass();
			FVector dir = Hit.ImpactNormal * OtherActor->GetVelocity();
			float size = dir.Size();

			size -= 100;

			float Damage = FMath::Clamp(size, 0.f, 20.f);// size > 0 ? 20 : 0;

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("hit with some object %f %f"), size, Hit.ImpactNormal.X));


			//test ragdoll on collisions
			if (Damage > 10)
			{
				StartRagdoll();

				//begin countdown to stop ragdoll
				FTimerDelegate TimerDel;
				TimerDel.BindUObject(this, &ABaseCharacter::StopRagdoll);
				GetWorld()->GetTimerManager().SetTimer(RagdollTimerHandle, TimerDel, 3, false);
			}


			TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
			FDamageEvent DamageEvent(ValidDamageTypeClass);
			TakeDamage(Damage, DamageEvent, nullptr, this);
		}
	}
}

void ABaseCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("EXE: baseChar Overlap"));
}

float ABaseCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
	//report the damage event
	AActor* hitActor = this;
	AActor* hitInstigator = nullptr;
	if (EventInstigator)
		hitInstigator = EventInstigator->GetPawn();
	FHitResult outHitRes;
	FVector outDir;

	DamageEvent.GetBestHitInfo(hitActor, hitInstigator, outHitRes, outDir);

	UAISense_Damage::ReportDamageEvent(GetWorld(), hitActor, hitInstigator ? hitInstigator : nullptr, DamageAmount, GetActorLocation(), outHitRes.ImpactPoint);

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}