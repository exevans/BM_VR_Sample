// Fill out your copyright notice in the Description page of Project Settings.

#include "RouteFollowerComponent.h"

// Sets default values for this component's properties
URouteFollowerComponent::URouteFollowerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// ...
	movementSpeed = 200.f;
	distanceTravelled = 0;
	targetWaypoint = 0;
	startingId = 0;

	updateLocation = true;
	updateRotation = false;
	loop = false;

	route = nullptr;

	movementType = MovementType::CONSTANT;
}


// Called when the game starts
void URouteFollowerComponent::BeginPlay()
{
	Super::BeginPlay();

	//check the actor has a spline component
	if (RouteActor)
		route = Cast<USplineComponent>(RouteActor->GetComponentByClass(USplineComponent::StaticClass()));
	
	if (route)
	{
		//set initial point
		GetOwner()->SetActorLocation(route->GetLocationAtSplinePoint(startingId, ESplineCoordinateSpace::World));
		distanceTravelled = route->GetDistanceAlongSplineAtSplinePoint(startingId);

		//alliw ticking 
		SetComponentTickEnabled(true);
	}
}


// Called every frame
void URouteFollowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!route)
		return;

	//have the actor move along the mesh
	distanceTravelled += (DeltaTime * movementSpeed);

	//if we have done a cycle reset
	if (loop && distanceTravelled >= route->GetSplineLength())
		distanceTravelled -= route->GetSplineLength();

	//if waypoint based check if the loc has been reached
	if (movementType == MovementType::WAYPOINT)
	{
		float distanceAtDesiredPoint = route->GetDistanceAlongSplineAtSplinePoint(targetWaypoint);
		if (distanceTravelled >= distanceAtDesiredPoint) //overshot/reached
		{
			distanceTravelled = distanceAtDesiredPoint;

			//stop ticking
			SetComponentTickEnabled(false);

			//broadcast that a waypoint has been reached
		}
	}

	//get the transform at the spline distance
	if (updateLocation)
	{
		FVector newPos = route->GetWorldLocationAtDistanceAlongSpline(distanceTravelled);
		GetOwner()->SetActorLocation(newPos);
	}
	if (updateRotation)
	{
		FRotator newDir = route->GetWorldRotationAtDistanceAlongSpline(distanceTravelled);
		newDir.Roll = 0.f;
		newDir.Pitch = 0.f;
		GetOwner()->SetActorRotation(newDir);
	}
}

void URouteFollowerComponent::SetTargetWaypoint(int32 pointid)
{
	if (pointid == targetWaypoint)
		return;

	targetWaypoint = pointid;

	if (route)
		SetComponentTickEnabled(true);
}