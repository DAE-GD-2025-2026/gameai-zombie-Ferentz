// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvBrain.h"

#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Items/Weapon.h"
#include "Village/House/House.h"
#include <AIController.h>
#include "Zombies/BaseZombie.h"
#include <SurvivorSate.h>
#include "Survivor/SurvivorPawn.h"

void USurvBrain::PickupItem(ABaseItem* itme)
{
	GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
		FString::Printf(TEXT("Can Enemy Shoot? %i"), maxInventory));
	if (invenorySlot >= maxInventory)
	{
		GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
			FString::Printf(TEXT("inventory full")));
		return;
	}
	inventory->GrabItem(invenorySlot,itme);
	invenorySlot++;
}

// Sets default values for this component's properties
USurvBrain::USurvBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USurvBrain::BeginPlay()
{
	Super::BeginPlay();

	// ...

	parent = GetOwner();
	inventory = parent->GetComponentByClass<UInventoryComponent>();
	maxInventory = inventory->GetInventoryCapacity();
	health = parent->GetComponentByClass<UHealthComponent>();
	stamina = parent->GetComponentByClass<UStaminaComponent>();

	if (APawn* Pawn = Cast<APawn>(parent))
	{
		if (ASurvivorPawn* surv = Cast<ASurvivorPawn>(parent))
		{
			Survivor = surv;
		}
		if (AAIController* controller = Cast<AAIController>(Pawn->GetController()))
		{
			blackboard = controller->GetBlackboardComponent();

		}
		else
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				TEXT("BLACKBOARD IS NULL")
			);
			return;
		}
	}
	goal = EGoalType::Search;
	executeState = true;

	blackboard->SetValueAsObject("brain", this);
	
}

// Called every frame
void USurvBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (executeState)
	{
		ExecuteState();
	}
	// ...
}

void USurvBrain::JobsDone(bool completedGoal)
{
	health->HealDamage(10);
	CompleteGoal();
	executeState = true;
}

void USurvBrain::GetHurt(AActor* Actor, FAIStimulus Stimulus)
{

	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, 
	FString::Printf(TEXT("Saw Something!")));
	/*if (auto controller = Cast<AAIController>(parent->GetInstigatorController()))
	{
		if (auto blackboard = controller->GetBlackboardComponent())
		{
			blackboard->SetValueAsBool("hasSeenZom", true);
			blackboard->SetValueAsObject("stimuli", Actor);
		}
	}*/
	if (auto zombie = Cast<ABaseZombie>(Actor))
	{
		blackboard->SetValueAsBool("hasSeenZom", true);
		blackboard->SetValueAsObject("zombie", Actor);
	}
	

}

void USurvBrain::SpotThing(AActor* Actor, FAIStimulus Stimulus)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
	FString::Printf(TEXT("Saw Something!")));
	ABaseItem* item = Cast<ABaseItem>(Actor);
	if (item != nullptr)
	{
		const TArray<ABaseItem*>& Items = inventory->GetInventory();
		if (Items.Contains(item)) return;
		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
			FString::Printf(TEXT("Saw item!")));

		if (item->GetItemType() == EItemType::Garbage) return;
		/*if (item->GetValue() == 0)
		{
			GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
				FString::Printf(TEXT("empty item")));
			return;
		}*/

		//blackboard->SetValueAsObject("item", item);

		
		PickupItem(item);
		

		for (ABaseItem* InvItem : Items)
		{
			if (IsValid(InvItem))
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.f,
					FColor::Green,
					FString::Printf(TEXT("Inventory item: %s"), *InvItem->GetName())
				);
			}
		}
		GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
			FString::Printf(TEXT("end inventory")));
	}
	else if (auto house = Cast<AHouse>(Actor))
	{
		
		if (toVisitHouses.Contains(house) || visitedHouses.Contains(house)) return;

		toVisitHouses.Add(house);

		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
			FString::Printf(TEXT("Saw house!")));

		goal = EGoalType::Loot;
		executeState = true;
	}
	else if (auto zombi = Cast<ABaseZombie>(Actor))
	{
		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
			FString::Printf(TEXT("Saw zombi!")));
		knownZombies.Add(zombi);
		goal = EGoalType::Hide;
		executeState = true;
	}
}

void USurvBrain::CompleteGoal()
{
	switch (goal)
	{
	case EGoalType::Search:
		break;
	case EGoalType::Loot:
		CompleteLoot();
		break;
	case EGoalType::Hide:
		break;
	}
}

void USurvBrain::CompleteLoot()
{
	visitedHouses.Add(toVisitHouses[0]);
	toVisitHouses.RemoveSingle(toVisitHouses[0]);
}


void USurvBrain::ExecuteState()
{
	executeState = false;

	EGoalType newGoal;
	switch (goal)
	{
	case EGoalType::Search:
		newGoal = Search();
		break;
	case EGoalType::Loot:
		newGoal = Loot();
		break;
	case EGoalType::Hide:
		newGoal = Hide();
		break;
	}

	if (newGoal != goal)
	{
		goal = newGoal;
		executeState = true;
	}
}

EGoalType USurvBrain::Search()
{
	if (toVisitHouses.Num() > 0) 
		return EGoalType::Loot;

	blackboard->SetValueAsEnum("goal", (uint8)goal);

	return EGoalType::Search;
}

EGoalType USurvBrain::Loot()
{
	if (toVisitHouses.Num() == 0)
		return EGoalType::Search;

	blackboard->SetValueAsVector("housePos", toVisitHouses[0]->GetActorTransform().GetLocation());
	blackboard->SetValueAsEnum("goal", (uint8)goal);

	return EGoalType::Loot;
}

EGoalType USurvBrain::Hide()
{
	if (knownZombies.Num() == 0) return EGoalType::Loot;

	blackboard->SetValueAsObject("zombie", knownZombies[0]);

	if (SelectWeapon())
	{
		blackboard->SetValueAsObject("weapon", selectedWeapon);
	}
	blackboard->SetValueAsEnum("goal", (uint8)goal);
	return EGoalType::Hide;
}

bool USurvBrain::SelectWeapon()
{
	selectedWeapon = NULL;
	int potentialDamage{};
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (ABaseItem* InvItem : Items)
	{
		if (IsValid(InvItem))
		{
			if (auto weapon = Cast<AWeapon>(InvItem))
			{
				int totaldamage{ weapon->GetValue() + weapon->GetDamage() };
				if (totaldamage > potentialDamage)
				{
					selectedWeapon = weapon;
					potentialDamage = totaldamage;
				}
			}
		}
	}
	if (selectedWeapon == NULL) return false;
	else return true;
}

void USurvBrain::Shoot()
{
	selectedWeapon->UseItem(*Survivor);
	if (selectedWeapon->GetValue() == 0) SelectWeapon();
}


//
//
//USurvBrain::SurvivorState USurvBrain::Loot::ExecuteState()
//{
//	return USurvBrain::Loot();
//}
//
//void USurvBrain::Loot::OnEnter()
//{
//}
//
//void USurvBrain::Loot::OnExit()
//{
//}
//
//USurvBrain::SurvivorState USurvBrain::Search::ExecuteState()
//{
//	return USurvBrain::Search();
//}
//
//void USurvBrain::Search::OnEnter()
//{
//}
//
//void USurvBrain::Search::OnExit()
//{
//}
//
//USurvBrain::SurvivorState USurvBrain::Hide::ExecuteState()
//{
//	return USurvBrain::Hide();
//}
//
//void USurvBrain::Hide::OnEnter()
//{
//}
//
//void USurvBrain::Hide::OnExit()
//{
//}
