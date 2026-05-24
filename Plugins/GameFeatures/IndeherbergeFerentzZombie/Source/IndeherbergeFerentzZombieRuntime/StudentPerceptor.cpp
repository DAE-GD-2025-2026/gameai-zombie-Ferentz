// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"
#include <AIController.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Zombies/BaseZombie.h"
#include "Village/House/House.h"

#include "SurvBrain.h"


UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = true;
	sightId = UAISense::GetSenseID(UAISense_Sight::StaticClass());
	hurtId = UAISense::GetSenseID(UAISense_Damage::StaticClass());
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();
	
	parent = GetOwner();
	if (auto PerceptionComp = parent->GetComponentByClass<UAIPerceptionComponent>())
	{
		parentPerception = PerceptionComp;
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
		PerceptionComp->ProcessStimuli();
	}
	if (APawn* Pawn = Cast<APawn>(parent))
	{
		
		if (auto brainComp = parent->GetComponentByClass<USurvBrain>())
		{
			brain = brainComp;
		}
	}

	auto ConfigSight = Cast<UAISenseConfig_Sight>(parentPerception->GetSenseConfig(sightId));
	ConfigSight->SightRadius = 10000;
	
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == sightId)
	{
		brain->SpotThing(Actor, Stimulus);
	}
	else if (Stimulus.Type == hurtId)
	{
		brain->GetHurt(Actor, Stimulus);
	}


	//GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, 
	//FString::Printf(TEXT("Saw Something!")));
	///*if (auto controller = Cast<AAIController>(parent->GetInstigatorController()))
	//{
	//	if (auto blackboard = controller->GetBlackboardComponent())
	//	{
	//		blackboard->SetValueAsBool("hasSeenZom", true);
	//		blackboard->SetValueAsObject("stimuli", Actor);
	//	}
	//}*/
	//if (auto zombie = Cast<ABaseZombie>(Actor))
	//{
	//	blackboard->SetValueAsBool("hasSeenZom", true);
	//	blackboard->SetValueAsObject("zombie", Actor);
	//}
	//else if (auto item = Cast<ABaseItem>(Actor))
	//{
	//	blackboard->SetValueAsObject("item", Actor);
	//}
	//else if (auto house = Cast<AHouse>(Actor))
	//{
	//	blackboard->SetValueAsObject("house", Actor);
	//}
}
