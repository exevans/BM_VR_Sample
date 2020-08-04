// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AiAbilityBase.h"
#include "Teleport_AiAbility.generated.h"

/**
 * 
 */
UCLASS()
class BM_API ATeleport_AiAbility : public AiAbilityBase
{
	GENERATED_BODY()

public:
	ATeleport_AiAbility();
	~ATeleport_AiAbility();

	virtual void BeginAbility() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndAbility() override;

private:
	float waitTime;
	bool teleportInProgress;

	FVector teleportPos;
	FName socket = "hand_lSocket";
};
