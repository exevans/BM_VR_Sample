// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest.h"
#include "Components/ActorComponent.h"
#include "QuestComponent.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveCompleteSignature, int, ObjectiveIdx);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BM_API UQuestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TSubclassOf<AQuest> questClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	AQuest* currentQuest;

	int questIndex;
	
	void BeginQuests();
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventDispatcher)
	//FObjectiveCompleteSignature ObjectiveComplete;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void QuestCompleted();
	void StartQuest(int questIndex);

	TArray<AQuest*> quests;

	int QuestObjectiveId;
	

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
