// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthComponent;


/** Broadcasts whenever we lose all health */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthComponentDeadSignature, AActor*, killedActor);
/** Broadcasts when health is changed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthComponentHealthChangedSignature, int32, current_health);
/** Brodcast when health crosses a boundry */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthComponentHealthBoundarySignature, int32, current_Health_percent, bool, changed_under);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BM_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Reset();

	UFUNCTION()
		void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser );

	void AddConstantDamage(void* key, float dmgPerSec, AActor* DamageCauser);
	void RemoveConstantDamage(void* key);

	UFUNCTION(Client, Reliable)
		void ClientUpdateHealth(int newHealth);

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FHealthComponentHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FHealthComponentDeadSignature OnHealthDead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	int32 max_health;

	float GetRemainingHealthProportion() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Health")
	float current_health;

	FHealthComponentHealthBoundarySignature& GetHealthBoundaryDelagate(int percentBoundary);
private:
	bool alive;

	//damage that should be applied everysecond
	float TotConstDmagPerSec;
	//so we can remove the constant damage when needed
	TMap<void*, float> constDmgMap;

	//can be called both from one off damage and constant
	void UpdateDamage(float damage);

	//health has reached 0
	void SetDead();

	//boundary checks
	TMap<int, FHealthComponentHealthBoundarySignature> boundaryDelagatesMap;
};
