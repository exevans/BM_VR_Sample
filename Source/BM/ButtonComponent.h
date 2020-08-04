// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "ButtonComponent.generated.h"

UENUM(BlueprintType)
enum class BUTTON_TYPE : uint8 {
	RETURN			UMETA(DisplayName = "Return"),
	TOGGLE_STAY		UMETA(DisplayName = "Toggle"),
	TOGGLE_RETURN	UMETA(DisplayName = "Toggle")
};

/** Broadcasts whenever the button changes from activated to deactivatd */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FButtonChangedActivatedSignature, bool, nowActivated);

/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class BM_API UButtonComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	UButtonComponent(const FObjectInitializer& ObjectInitializer);
	~UButtonComponent();

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
		FButtonChangedActivatedSignature OnButtonChangedActivate;

private:
	void SetButtonActiveState(bool nowActive);
	FVector GetButtonTargetLocation();

	UFUNCTION()
		void OnComponentHitBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void OnComponentHitEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		bool enabled;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		bool active;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		float depressSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		float maxPressDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		float activationDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		float stayDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		BUTTON_TYPE button_type;

	bool pressed;
	bool inActivatedZone;

	FTransform OriginalBaseTransform;
	UPrimitiveComponent* interactingComponent;
	FTransform InitialRelativeTransform;
	FVector initialInteractingCompRelLocation;
	FVector initialCompRelLocation;

	class UMaterialInstanceDynamic* matInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"), Category = "ButtonComp")
		TArray<AActor*> electricalObjects;
};
