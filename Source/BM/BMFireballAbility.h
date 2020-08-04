// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMAbility.h"
#include "BMFireballAbility.generated.h"

class ABMProjectile;
class AAttachedAbilityActor;
/**
 * 
 */
enum class FireballState : uint8 { EMPTY, CHARGING_SINGLE, READY_SINGLE, COMBINED_SECONDARY, CHARGING_COMBINED, READY_COMBINED, COUNT };

UCLASS()
class BM_API ABMFireballAbility : public ABMAbility
{
	GENERATED_BODY()

public:
	ABMFireballAbility();
	virtual ~ABMFireballAbility();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override {};

	void PostInit();
	void EnterAbility();
	void ExitAbility();
	void Tick(float);

	void OnInteract(EInputEvent eventType);
	void OnInteractSecondary();

	void AddAttachedAbility(UClass* attClass);

	void SpawnFireball();
	void ReadyFireball();

	//combining
	virtual void OnCombinedCast() override;
	virtual void OnEndCombinedCast() override;
	virtual void PrepareForCombine() override;
	virtual bool CanAbilityCombine() override;

	virtual bool IsAbilityReady() override;

	/*UFUNCTION(reliable, server, WithValidation)
		virtual void ServerAbilityActivated();*/
protected:
	void AbilityActivated() override;
	void AbilityDeactivated() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class ABMProjectile> ProjectileClass;

	float initialHandDist;
	ABMProjectile* proj;
	AAttachedAbilityActor* attachedAbility;

	float respawnDelay;
	FTimerHandle spawnDelayTimerHandle;
	FireballState current_state;

	void UpdateSingleFireball();
	void UpdateCombinedFireball();
	void ActivateCombinedFireball();

	class UOscilateComponent* OscilateComp;
};
