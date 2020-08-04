// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestComponent.h"
#include "Engine/GameEngine.h"
#include "Kismet/GameplayStatics.h"
#include "Quest.h"
#include "BMGameInstance.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

// Sets default values for this component's properties
UQuestComponent::UQuestComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	currentQuest = nullptr;
	questIndex = 0;
	QuestObjectiveId = 0;
}


// Called when the game starts
void UQuestComponent::BeginPlay()
{
	Super::BeginPlay();

	UBMGameInstance* instance = Cast<UBMGameInstance>(GetWorld()->GetGameInstance());
	questIndex = instance->Ins_CurrentQuestId;
	QuestObjectiveId = instance->Ins_CurrentObjective;

}


// Called every frame
void UQuestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UQuestComponent::QuestCompleted()
{
	//end updates (shouldn't get any anyway
	currentQuest->OnQuestComplete.RemoveDynamic(this, &UQuestComponent::QuestCompleted);

	//if there is another quest in line start that one
	StartQuest(++questIndex);	
}

void UQuestComponent::StartQuest(int questIndex)
{
	//check we are in bounds
	if (quests.Num() > questIndex)
	{
		currentQuest = quests[questIndex];

		if (currentQuest)
		{
			//receive update when quest finishes
			currentQuest->OnQuestComplete.AddDynamic(this, &UQuestComponent::QuestCompleted);

			//begin the quest on the chosen objective
			currentQuest->SetNewObjective2(QuestObjectiveId);
			//reset for the future
			QuestObjectiveId = 0;
		}
	}
}

void UQuestComponent::BeginQuests()
{
	//can't be in beginplay or will get called before the abilitymanager has been created

	//find all quest actors in the level
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AQuest::StaticClass(), actors);
	//convert to quest actors
	//TArray<AQuest*> quests;
	for (AActor* act : actors)
	{
		quests.Add(Cast<AQuest>(act));
	}

	//order in terms of id
	quests.Sort([](const AQuest& questA, const AQuest& questB)
	{
		return questA.Questid < questB.Questid;
	});

	StartQuest(questIndex);
}