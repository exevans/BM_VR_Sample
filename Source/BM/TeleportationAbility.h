// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMAbility.h"
#include "TeleportationAbility.generated.h"
/**
 * 
 */

enum class Teleport_State : uint8 { NONE, PENDING_RELEASE, TELEPORTING };

UCLASS()
class BM_API ATeleportationAbility : public ABMAbility
{
	GENERATED_BODY()

public:
	ATeleportationAbility();
	virtual ~ATeleportationAbility();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	void PostInit();
	void EnterAbility();
	void ExitAbility();
	void Tick(float DeltaTime);

	void OnInteract(EInputEvent eventType);
	void OnInteractSecondary();

protected:
	void AbilityActivated() override;
	void AbilityDeactivated() override;

private:
	//UPROPERTY(Replicated)
	//bool teleporting;
	void StartTeleporting();
	void UpdateFindingTeleportPosition();
	void UpdateMovingToTeleportPosition(float DeltaTime);

	Teleport_State current_state;


	bool valid_teleport;
	FVector teleport_loc;

	bool climbLedge;
	FVector ledgeDirection;

	UPROPERTY(Replicated)
	FVector locked_teleport_loc;

	virtual bool IsAbilityReady()  override {
		return current_state != Teleport_State::TELEPORTING;}//!teleporting; }

	class UParticleSystemComponent* trail;
	class UParticleSystemComponent* teleportPlacementComponent;

	UPROPERTY(EditAnywhere, Category = ParticleSystems, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* PS_SelectTeleportSpot;
	UPROPERTY(EditAnywhere, Category = ParticleSystems, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* PS_BeginTeleport;
	UPROPERTY(EditAnywhere, Category = ParticleSystems, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* PS_DuringTeleport;
	UPROPERTY(EditAnywhere, Category = ParticleSystems, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* PS_EndTeleport;
	UPROPERTY(EditAnywhere, Category = Speed, meta = (AllowPrivateAccess = "true"))
		float TeleportRate;
};
