// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvBrain.h"

#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Items/Weapon.h"
#include "Village/House/House.h"
#include "Zombies/BaseZombie.h"
#include <SurvivorSate.h>
#include "Survivor/SurvivorPawn.h"

#include <AIController.h>


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
	executeGoal = true;

	blackboard->SetValueAsObject("brain", this);
	
}

// Called every frame
void USurvBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (health->GetHealth() < health->GetMaxHealth() / 2)
	{

	}

	houseTimer += DeltaTime;
	if (houseTimer >= houseTurnover * (visitedHouses.Num() + 1))
	{
		ClearHouses();
	}

	//health->HealDamage(10);
	if (executeGoal)
	{
		ExecuteGoal();
	}
	// ...
}


#pragma region input
void USurvBrain::GetHurt(AActor* Actor, FAIStimulus Stimulus)
{

	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("got hurt!")));

	if (auto zombie = Cast<ABaseZombie>(Actor))
	{
		knownZombies.Add(zombie);
	}

	EvaluateGoal();
}

void USurvBrain::SpotThing(AActor* Actor, FAIStimulus Stimulus)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw Something!")));
	ABaseItem* item = Cast<ABaseItem>(Actor);
	if (item != nullptr)
	{
		SpotItem(item);
		return;
	}
	else if (auto house = Cast<AHouse>(Actor))
	{

		SpotHouse(house);
	}
	else if (auto zombi = Cast<ABaseZombie>(Actor))
	{
		SpotZombie(zombi);
	}

	EvaluateGoal();
}

void USurvBrain::SpotItem(ABaseItem* item)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw item!")));

	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	if (Items.Contains(item)) return;

	if (item->GetItemType() == EItemType::Garbage) return;
	if (item->GetValue() == 0) return;

	PickupItem(item);
}

void USurvBrain::SpotHouse(AHouse* house)
{
	if (toVisitHouses.Contains(house) || visitedHouses.Contains(house)) return;

	toVisitHouses.Add(house);

	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw house!")));

}

void USurvBrain::SpotZombie(ABaseZombie* zombie)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw zombi!")));
	knownZombies.Add(zombie);

}

#pragma endregion input

void USurvBrain::EvaluateGoal()
{
	EGoalType newGoal = GetGoal();
	if (newGoal != goal)
	{
		goal = newGoal;
		executeGoal = true;
	}
}

EGoalType USurvBrain::GetGoal()
{
	bool hasWeapon{ SelectWeapon() };
	if (IsThreathed())
	{
		if (hasWeapon)
		{
			return EGoalType::Attack;
		}
		if (KnowsWeapon()) return EGoalType::Loot;

		return EGoalType::Flee;
	}
	if(toVisitHouses.Num() > 0) return EGoalType::Loot;

	if (invenorySlot < maxInventory && knownItems.Num() > 0) return EGoalType::Loot;

	return EGoalType::Search;
}

void USurvBrain::ExecuteGoal()
{
	executeGoal = false;

	EGoalType newGoal;
	switch (goal)
	{
	case EGoalType::Search:
		newGoal = Search();
		break;
	case EGoalType::Loot:
		newGoal = Loot();
		break;
	case EGoalType::Attack:
		newGoal = Attack();
		break;
	case EGoalType::Flee:
		newGoal = Flee();
		break;
	}

	if (newGoal != goal)
	{
		goal = newGoal;
		executeGoal = true;
	}
}

#pragma region JobsDone
void USurvBrain::JobsDone(bool completedGoal)
{
	if (IsGoalDone())
	{
		EvaluateGoal();
		executeGoal = true;
	}	
}

bool USurvBrain::IsGoalDone()
{
	switch (goal)
	{
	case EGoalType::Search:
		return CompleteSearch();
		break;
	case EGoalType::Loot:
		return CompleteLoot();
		break;
	case EGoalType::Attack:
		return CompleteAttack();
		break;
	case EGoalType::Flee:
		return CompleteFlee();
		break;
	default:
		break;
	}
	return false;
}

#pragma endregion JobsDone


#pragma region search

EGoalType USurvBrain::Search()
{
	FVector direction{ 0,0,0 };
	if (fleeDirection == direction)
	{
		direction = FMath::VRand();
	}
	else
	{
		direction = fleeDirection + FMath::VRand();
	}
	blackboard->SetValueAsVector("direction", direction);
	PassGoal();
	return EGoalType::Search;
}

bool USurvBrain::CompleteSearch()
{
	// if nothing triggered search to end, we continue searching;
	return true;
}

#pragma endregion search

#pragma region Loot
EGoalType USurvBrain::Loot()
{
	FVector goalpos{};
	if (isThreathened && knownWeapon != NULL)
	{
		goalpos = knownWeapon->GetActorTransform().GetLocation();
		GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
			FString::Printf(TEXT("looking for weapon")));
	}
	else
	{
		if (toVisitHouses.Num() == 0)
		{
			if(knownItems.Num() == 0) return EGoalType::Search;

			goalpos = knownItems[0]->GetActorTransform().GetLocation();
			GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
				FString::Printf(TEXT("looking for item")));
		}
		else
		{
			goalpos = toVisitHouses[0]->GetActorTransform().GetLocation();
			GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
				FString::Printf(TEXT("looking for house")));
		}
	}
	
	blackboard->SetValueAsVector("goalPos", goalpos);
	PassGoal();
	return EGoalType::Loot;
}

bool USurvBrain::CompleteLoot()
{
	visitedHouses.Add(toVisitHouses[0]);
	toVisitHouses.RemoveSingle(toVisitHouses[0]);
	return true;
}

#pragma endregion Loot

#pragma region Attack
EGoalType USurvBrain::Attack()
{
	if (knownZombies.Num() == 0) return EGoalType::Loot;
	if (!SelectWeapon()) return EGoalType::Flee;

	blackboard->SetValueAsObject("zombie", closestZombie);
	PassGoal();

	return EGoalType::Attack;
}

bool USurvBrain::CompleteAttack()
{
	if (!IsValid(closestZombie))
	{
		// if the zombie is dead, always evaluate and update;
		knownZombies.RemoveSingle(closestZombie);
		return true;
	}
	// if zombie isnt dead, dont change directive;
	return false;
}
#pragma endregion Attack

#pragma region Flee
EGoalType USurvBrain::Flee()
{
	if (!IsThreathed()) return EGoalType::Loot;
	blackboard->SetValueAsVector("direction", fleeDirection);
	PassGoal();
	return EGoalType::Flee;
}

bool USurvBrain::CompleteFlee()
{
	//always evaluate goal and update blackboard
	return true;
}

#pragma endregion Flee



#pragma region acions

void USurvBrain::PickupItem(ABaseItem* itme)
{
	GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
		FString::Printf(TEXT("Can Enemy Shoot? %i"), maxInventory));
	if (invenorySlot >= maxInventory)
	{
		knownItems.Add(itme);
		return;
	}
	knownItems.RemoveSingle(itme);
	inventory->GrabItem(invenorySlot, itme);
	invenorySlot++;
}

bool USurvBrain::SelectWeapon()
{
	selectedWeaponIdx = -1;
	int potentialDamage{};
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	int toRemove{-1};
	for (int i{}; i < Items.Num() ; i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (auto weapon = Cast<AWeapon>(InvItem))
			{
				int totaldamage{ weapon->GetValue() + weapon->GetDamage() };
				if (totaldamage > potentialDamage)
				{
					selectedWeaponIdx = i;
					potentialDamage = totaldamage;
				}
			}
		}
	}
	if (toRemove >= 0) inventory->RemoveItem(toRemove);
	if (selectedWeaponIdx == -1)
	{
		blackboard->SetValueAsBool("weapon", false);
		return false;
	}
	else
	{
		blackboard->SetValueAsBool("weapon", true);
		return true;
	}
}

void USurvBrain::Shoot()
{
	SelectWeapon();
	if (selectedWeaponIdx < 0) return;
	inventory->UseItem(selectedWeaponIdx);
	const TArray<ABaseItem*>& Items = inventory->GetInventory();

	if (Items[selectedWeaponIdx]->GetValue() == 0)
	{
		inventory->RemoveItem(selectedWeaponIdx);
		selectedWeaponIdx = -1;
		SelectWeapon();
	}
}

bool USurvBrain::IsThreathed()
{
	if (knownZombies.Num() == 0) return false;
	bool threathened{false};
	FVector direction{};
	float closestZomDistance{ toloratedDistance };
	for (auto zombie : knownZombies)
	{
		auto zombiDirection{ parent->GetActorLocation() - zombie->GetActorLocation()  };
		auto distance{ zombiDirection.Size() };

		// if zombie closer than tolerated
		if (toloratedDistance > distance)
		{
			threathened = true;
			if (closestZomDistance > distance)
			{
				// safe cosest zombie
				closestZomDistance = distance;
				closestZombie = zombie;
			}
		}
		// calculate best flee direction;
		zombiDirection.Normalize();
		direction += zombiDirection;
	}

	fleeDirection = direction;
	isThreathened = threathened;
	return threathened;
}

bool USurvBrain::KnowsWeapon()
{
	for (auto item : knownItems)
	{
		if (item->GetItemType() == EItemType::Pistol || item->GetItemType() == EItemType::Shotgun)
		{
			knownWeapon = item;
			return true;
		}
	}
	return false;
}

void USurvBrain::ClearHouses()
{
	for (auto house : visitedHouses)
	{
		toVisitHouses.Add(house);
	}
	visitedHouses.Empty();
}

void USurvBrain::TryHeal()
{
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (int i{}; i < Items.Num(); i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (InvItem->GetItemType() == EItemType::Medkit)
			{
				inventory->UseItem(i);
				return;
			}
		}
	}
	health->HealDamage(1);
}

#pragma endregion acions

