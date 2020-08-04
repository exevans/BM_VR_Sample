// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AiAbilityBase.h"
#include "Fireball_AiAbility.generated.h"
/**
 * 
 */
UCLASS()
class BM_API AFireball_AiAbility : public AiAbilityBase
{
	GENERATED_BODY()

public:
	AFireball_AiAbility();
	~AFireball_AiAbility();

	virtual void BeginAbility() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndAbility() override;

private:
	float waitTime;
};
