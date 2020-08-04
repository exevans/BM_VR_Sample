// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlacementTrapActor.h"
#include "MotionControllerComponent.h"
#include "FireballTargetableInterface.h"
#include "BaseCharacter.generated.h"

UCLASS()
class BM_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	//TODO only used for summon ability, move to ability??
	UPROPERTY(VisibleDefaultsOnly, Category = Physics)
		class UPhysicsHandleComponent* physicsHandle_Left;
	UPROPERTY(VisibleDefaultsOnly, Category = Physics)
		class UPhysicsHandleComponent* physicsHandle_Right;

	//IK values set in code -> EventGraph Tick: move values to blueprint variables --> AnimGraph uses values to modify bones 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector leftHandIKTargetPos;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator leftHandIKTargetDir;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float leftHandAlpha;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector rightHandIKTargetPos;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator rightHandIKTargetDir;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float rightHandAlpha;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FRotator HeadRotator;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector headLook_loc;

	//used by ai move?
	AActor* actorToblock;

	/** Health */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	class UHealthComponent* HealthComponent;

	void StartRagdoll();
	void StopRagdoll();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	//stop ragdoll timer
	FTimerHandle RagdollTimerHandle;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;

	virtual FVector GetHandLocation(int hand) { return FVector(0, 0, 0); };
	virtual FVector GetHandForwardVector(int hand) { return FVector(0, 0, 0); };

	FORCEINLINE class UPhysicsHandleComponent* GetPhysicsHandle(int hand) const { if (hand == 0) return physicsHandle_Left; else return physicsHandle_Right; }

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);

	class UHealthComponent* GetHealthComponent() { return HealthComponent; }

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;
};
