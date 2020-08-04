// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/ActorComponent.h"
#include "RouteFollowerComponent.generated.h"

UENUM(BlueprintType)
enum MovementType { CONSTANT, WAYPOINT, COUNT };

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BM_API URouteFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URouteFollowerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		int32 startingId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		float movementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		TEnumAsByte<MovementType> movementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		bool updateLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		bool updateRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		bool loop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement)
		AActor* RouteActor;


	void SetTargetWaypoint(int32 pointid);
private:
	//how far along spline have we travelled
	float distanceTravelled;

	//which waypoint is the target
	int32 targetWaypoint;

	USplineComponent* route;
};
