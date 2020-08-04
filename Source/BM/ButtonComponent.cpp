// Fill out your copyright notice in the Description page of Project Settings.

#include "ButtonComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"  
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "ElectricalInterface.h"


UButtonComponent::UButtonComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetGenerateOverlapEvents(true);

	SetCollisionResponseToAllChannels(ECR_Overlap);

	pressed = false;
	inActivatedZone = false;
	enabled = true;
	active = false;
	maxPressDistance = 30.f;
	activationDistance = 30.f;
	depressSpeed = 10.f;
	button_type = BUTTON_TYPE::RETURN;
}

UButtonComponent::~UButtonComponent()
{

}

void UButtonComponent::BeginPlay()
{
	Super::BeginPlay();

	//the initial transform of the button component to parent actor
	InitialRelativeTransform = GetRelativeTransform();
	OriginalBaseTransform = CalcNewComponentToWorld(InitialRelativeTransform);

	OnComponentBeginOverlap.AddDynamic(this, &UButtonComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UButtonComponent::OnOverlapEnd);

	//dynamic material
	matInstance = CreateDynamicMaterialInstance(0, GetMaterial(0));
}

// every frame
void UButtonComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//local version of pressed
	bool newPressed = pressed;
	//if something on the button
	if (interactingComponent)
	{
		//get relative change in location of the interacting (compared to when it first touched, clamped between 0 and max press
		float relChange = initialInteractingCompRelLocation.Z - OriginalBaseTransform.InverseTransformPosition(interactingComponent->GetComponentLocation()).Z;

		//calculate the overall change of depth to be applied
		float CheckDepth = FMath::Clamp(relChange + (InitialRelativeTransform.GetTranslation().Z - initialCompRelLocation.Z), 0.f, maxPressDistance);

		if (CheckDepth > 0)
		{
			float ClampMinDepth = 0.0f;

			float NewDepth = InitialRelativeTransform.GetTranslation().Z + (-CheckDepth);

			//set using the relative space
			SetRelativeLocation(InitialRelativeTransform.TransformPosition(FVector(0,0, NewDepth)));
		}
	}
	else 
	{
		if (RelativeLocation.Equals(GetButtonTargetLocation()))
		{
			newPressed = false;
			SetComponentTickEnabled(false);
		}
		else
		{
			SetRelativeLocation(FMath::VInterpConstantTo(RelativeLocation, GetButtonTargetLocation(), DeltaTime, depressSpeed));
		}
	}


	//check if the activation state has changed
	float ButtonDepth = InitialRelativeTransform.GetTranslation().Z - GetRelativeTransform().GetTranslation().Z;
	bool currentActivatedState = (ButtonDepth > activationDistance);
	if (currentActivatedState)
		newPressed = true;

	bool activatedChanged = currentActivatedState != inActivatedZone;
	inActivatedZone = currentActivatedState;

	bool pressedChanged = newPressed != pressed;
	pressed = newPressed;

	//change in pressed state
	if (button_type == BUTTON_TYPE::TOGGLE_STAY && pressedChanged)
	{
		if (pressed)
		{
			active = !active;
			SetButtonActiveState(active);
		}
	}
	else if (button_type != BUTTON_TYPE::TOGGLE_STAY && activatedChanged)
	{
		active = !active;
		SetButtonActiveState(active);
	}
}

void UButtonComponent::SetButtonActiveState(bool nowActive)
{
	active = nowActive;
	OnButtonChangedActivate.Broadcast(nowActive);
	//set the colour
	FLinearColor colour = FLinearColor::Red;
	if (nowActive)
		colour = FLinearColor::Green;

	matInstance->SetVectorParameterValue("ButtonColour", colour);

	//send signal
	for (AActor* obj : electricalObjects)
	{
		IElectricalInterface* interfaceImp = Cast<IElectricalInterface>(obj);
		if (obj->GetClass()->ImplementsInterface(UElectricalInterface::StaticClass()))
		{
			interfaceImp->Execute_OnStateChanged(obj, nowActive);
		}
	}
}

FVector UButtonComponent::GetButtonTargetLocation()
{
	if (button_type == BUTTON_TYPE::TOGGLE_STAY && active)
	{
		return InitialRelativeTransform.GetTranslation() - FVector(0, 0, stayDistance);
	}
	else {
		return InitialRelativeTransform.GetTranslation();
	}
}

void UButtonComponent::OnComponentHitBegin(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

void UButtonComponent::OnComponentHitEnd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

void UButtonComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//check not ourself, early exit
	if (OtherComp == this || OtherActor == GetOwner())
		return;

	//set the interacting component
	interactingComponent = OtherComp;
	//get the transform from component to the world
	//interacting comp and main comp location in space relative to the initial button location
	initialInteractingCompRelLocation = OriginalBaseTransform.InverseTransformPosition(interactingComponent->GetComponentLocation());
	initialCompRelLocation = OriginalBaseTransform.InverseTransformPosition(this->GetComponentLocation());

	//start ticking
	SetComponentTickEnabled(true);
}

void UButtonComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherComp == interactingComponent)
		interactingComponent = nullptr;
}