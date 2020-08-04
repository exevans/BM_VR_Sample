// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BMAbility.h"
#include "CloakAbility.generated.h"

/**
 * 
 */
UCLASS()
class BM_API ACloakAbility : public ABMAbility
{
	GENERATED_BODY()
public:
	ACloakAbility();

	void PostInit();
	void EnterAbility();
	void ExitAbility();
	void Tick(float DeltaTime);

	void OnInteract(EInputEvent eventType);
	void OnInteractSecondary();

protected:
	virtual void AbilityActivated() override;
	virtual void AbilityDeactivated() override;

private:
	void HighlightObjectsOfInterest(bool newState);

	bool enabled;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
		UMaterialInterface* invisibility;

	//character hands
	UMaterialInstanceDynamic* dynamicMatL;
	UMaterialInstanceDynamic* dynamicMatR;

	float currentRefraction;
	bool localCanBeSeen;

};
