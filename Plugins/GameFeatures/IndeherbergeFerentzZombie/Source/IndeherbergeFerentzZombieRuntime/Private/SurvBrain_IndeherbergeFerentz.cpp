// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvBrain_IndeherbergeFerentz.h"

#include "Common/InventoryComponent.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "Items/Weapon.h"
#include "Village/House/House.h"
#include "Zombies/BaseZombie.h"
#include <SurvivorSate_IndeherbergeFerentz.h>
#include "Survivor/SurvivorPawn.h"

#include "GameFramework/FloatingPawnMovement.h"

#include <AIController.h>


// Sets default values for this component's properties
USurvBrain_IndeherbergeFerentz::USurvBrain_IndeherbergeFerentz()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USurvBrain_IndeherbergeFerentz::BeginPlay()
{
	Super::BeginPlay();

	// ...

	parent = GetOwner();
	inventory = parent->GetComponentByClass<UInventoryComponent>();
	health = parent->GetComponentByClass<UHealthComponent>();
	stamina = parent->GetComponentByClass<UStaminaComponent>();

	if (APawn* Pawn = Cast<APawn>(parent))
	{
		if (ASurvivorPawn* surv = Cast<ASurvivorPawn>(parent))
		{
			Survivor = surv;
			auto movement{ surv->GetMovementComponent() };
			if (auto flyingMovement = Cast<UFloatingPawnMovement>(movement))
			{
				floatingMovement = flyingMovement;
				normalMovement = floatingMovement->GetMaxSpeed();
				fleeMovement = normalMovement * 2;
			}
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
	goal = EGoalType_IndeherbergeFerentz::Search;
	executeGoal = true;

	blackboard->SetValueAsObject("brain", this);
	
}

// Called every frame
void USurvBrain_IndeherbergeFerentz::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (health->GetHealth() < health->GetMaxHealth())
	{
		TryHeal();
	}

	if (stamina->GetCurrentStamina() < stamina->GetMaxStamina())
	{
		TryEat();
	}

	houseTimer += DeltaTime;
	if (houseTimer >= houseTurnover * (visitedHouses.Num() + 1))
	{
		if(toVisitHouses.Num() <= 0)
		ClearHouses();
		houseTimer = 0;
	}

	//health->HealDamage(10);
	if (executeGoal)
	{
		ExecuteGoal();
	}

	if (goal == EGoalType_IndeherbergeFerentz::Loot)
	{
		threatcounter += DeltaTime;
		if (threatcounter >= threatUpdate)
		{
			threatcounter = 0.f;
			IsThreathed();
			blackboard->SetValueAsVector("direction", fleeDirection);
		}
		
	}
	// ...
}


#pragma region input
void USurvBrain_IndeherbergeFerentz::GetHurt(AActor* Actor, FAIStimulus Stimulus)
{

	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("got hurt!")));

	if (auto zombie = Cast<ABaseZombie>(Actor))
	{
		if(!knownZombies.Contains(zombie))
			knownZombies.Add(zombie);
	}

	EvaluateGoal();
}

void USurvBrain_IndeherbergeFerentz::SpotThing(AActor* Actor, FAIStimulus Stimulus)
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

	
}

void USurvBrain_IndeherbergeFerentz::SpotItem(ABaseItem* item)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw item!")));

	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	if (Items.Contains(item)) return;

	if (item->GetItemType() == EItemType::Garbage)
	{
		TrashTrash(item);
		return;
	}
	if (item->GetValue() == 0) return;

	if(PickupItem(item))
		EvaluateGoal();
}

void USurvBrain_IndeherbergeFerentz::SpotHouse(AHouse* house)
{
	if (toVisitHouses.Contains(house) || visitedHouses.Contains(house)) return;

	toVisitHouses.AddHead(house);// Add(house);
	EvaluateGoal();
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw house!")));

}

void USurvBrain_IndeherbergeFerentz::SpotZombie(ABaseZombie* zombie)
{
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green,
		FString::Printf(TEXT("Saw zombi!")));
	if (!knownZombies.Contains(zombie))
	knownZombies.Add(zombie);
	EvaluateGoal();
}

#pragma endregion input

void USurvBrain_IndeherbergeFerentz::EvaluateGoal()
{
	EGoalType_IndeherbergeFerentz newGoal = GetGoal();
	if (newGoal != goal)
	{
		goal = newGoal;
		executeGoal = true;
		if (floatingMovement != NULL)
		{
			if (newGoal == EGoalType_IndeherbergeFerentz::Flee)
			{
				floatingMovement->MaxSpeed = fleeMovement;
			}
			else
			{
				floatingMovement->MaxSpeed = normalMovement;
			}
		}
	}

	
}

EGoalType_IndeherbergeFerentz USurvBrain_IndeherbergeFerentz::GetGoal()
{
	bool hasWeapon{ SelectWeapon() };
	if (IsThreathed())
	{
		if (hasWeapon)
		{
			return EGoalType_IndeherbergeFerentz::Attack;
		}
		if (KnowsWeapon()) return EGoalType_IndeherbergeFerentz::Loot;

		return EGoalType_IndeherbergeFerentz::Flee;
	}
	if(toVisitHouses.Num() > 0) return EGoalType_IndeherbergeFerentz::Loot;

	if (HasSpace() > 0 && knownItems.Num() > 0) return EGoalType_IndeherbergeFerentz::Loot;

	return EGoalType_IndeherbergeFerentz::Search;
}

void USurvBrain_IndeherbergeFerentz::ExecuteGoal()
{
	executeGoal = false;

	EGoalType_IndeherbergeFerentz newGoal;
	switch (goal)
	{
	case EGoalType_IndeherbergeFerentz::Search:
		newGoal = Search();
		break;
	case EGoalType_IndeherbergeFerentz::Loot:
		newGoal = Loot();
		break;
	case EGoalType_IndeherbergeFerentz::Attack:
		newGoal = Attack();
		break;
	case EGoalType_IndeherbergeFerentz::Flee:
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
void USurvBrain_IndeherbergeFerentz::JobsDone(bool completedGoal)
{
	if (IsGoalDone())
	{
		EvaluateGoal();
		executeGoal = true;
	}	
}

bool USurvBrain_IndeherbergeFerentz::IsGoalDone()
{
	switch (goal)
	{
	case EGoalType_IndeherbergeFerentz::Search:
		return CompleteSearch();
		break;
	case EGoalType_IndeherbergeFerentz::Loot:
		return CompleteLoot();
		break;
	case EGoalType_IndeherbergeFerentz::Attack:
		return CompleteAttack();
		break;
	case EGoalType_IndeherbergeFerentz::Flee:
		return CompleteFlee();
		break;
	default:
		break;
	}
	return false;
}

#pragma endregion JobsDone


#pragma region search

EGoalType_IndeherbergeFerentz USurvBrain_IndeherbergeFerentz::Search()
{
	FVector direction{ 0,0,0 };
	if (fleeDirection == direction)
	{
		direction = FMath::VRand();
	}
	else
	{
		fleeDirection.Normalize();
		direction = fleeDirection + FMath::VRand();
	}
	blackboard->SetValueAsVector("direction", direction);
	PassGoal();
	return EGoalType_IndeherbergeFerentz::Search;
}

bool USurvBrain_IndeherbergeFerentz::CompleteSearch()
{
	// if nothing triggered search to end, we continue searching;
	return true;
}

#pragma endregion search

#pragma region Loot
EGoalType_IndeherbergeFerentz USurvBrain_IndeherbergeFerentz::Loot()
{
	if (isThreathened && knownWeapon != NULL)
	{
		lootObject = knownWeapon;
		lootObjectType = ELootType_IndeherbergeFerentz::Weapon;
		GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
			FString::Printf(TEXT("looking for weapon")));
	}
	else
	{
		if (toVisitHouses.Num() == 0)
		{
			if(knownItems.Num() == 0) return EGoalType_IndeherbergeFerentz::Search;

			lootObject = knownItems[0];
			lootObjectType = ELootType_IndeherbergeFerentz::Item;
			GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
				FString::Printf(TEXT("looking for item")));
		}
		else
		{
			lootObject = toVisitHouses.GetHead()->GetValue();
			lootObjectType = ELootType_IndeherbergeFerentz::House;
			GEngine->AddOnScreenDebugMessage(5, 10.f, FColor::Red,
				FString::Printf(TEXT("looking for house")));
		}
	}

	blackboard->SetValueAsVector("direction", fleeDirection);
	FVector goalpos = lootObject->GetActorTransform().GetLocation();
	blackboard->SetValueAsVector("goalPos", goalpos);
	PassGoal();
	return EGoalType_IndeherbergeFerentz::Loot;
}

bool USurvBrain_IndeherbergeFerentz::CompleteLoot()
{
	if (!IsValid(lootObject)) return true;

	switch (lootObjectType)
	{
	case ELootType_IndeherbergeFerentz::House:
	{
		auto house = Cast<AHouse>(lootObject);
		visitedHouses.AddTail(house);
		toVisitHouses.RemoveNode(house);
		break;
	}
	case ELootType_IndeherbergeFerentz::Item:
		// if the item got picked up, pickup ite will take care of this.
		
		//auto item = Cast<ABaseItem>(lootObject);
		//knownItems.RemoveSingle(item);
		break;
	case ELootType_IndeherbergeFerentz::Weapon:
		// if the item got picked up, pickup ite will take care of this.
		
		//auto item = Cast<ABaseItem>(lootObject);
		//knownItems.RemoveSingle(item);
		break;
	default:
		break;
	}
	
	return true;
}

#pragma endregion Loot

#pragma region Attack
EGoalType_IndeherbergeFerentz USurvBrain_IndeherbergeFerentz::Attack()
{
	if (knownZombies.Num() == 0) return EGoalType_IndeherbergeFerentz::Loot;
	if (!SelectWeapon()) return EGoalType_IndeherbergeFerentz::Flee;

	if (!IsValid(closestZombie))
	{
		// if the zombie is dead, always evaluate and update;
		knownZombies.RemoveSingle(closestZombie);
		if (!IsThreathed())
		{
			return EGoalType_IndeherbergeFerentz::Loot;
		}
	}

	blackboard->SetValueAsObject("zombie", closestZombie);
	PassGoal();

	return EGoalType_IndeherbergeFerentz::Attack;
}

bool USurvBrain_IndeherbergeFerentz::CompleteAttack()
{
	if (!IsValid(closestZombie))
	{
		// if the zombie is dead, always evaluate and update;
		knownZombies.RemoveSingle(closestZombie);
		return true;
	}
	if (!HasWeapon())
	{
		return true;
	}
	// if zombie isnt dead, dont change directive;
	return false;
}
#pragma endregion Attack

#pragma region Flee
EGoalType_IndeherbergeFerentz USurvBrain_IndeherbergeFerentz::Flee()
{
	if (!IsThreathed()) return EGoalType_IndeherbergeFerentz::Loot;
	blackboard->SetValueAsVector("direction", fleeDirection);
	PassGoal();
	return EGoalType_IndeherbergeFerentz::Flee;
}

bool USurvBrain_IndeherbergeFerentz::CompleteFlee()
{
	//always evaluate goal and update blackboard
	return true;
}

#pragma endregion Flee



#pragma region acions

void USurvBrain_IndeherbergeFerentz::TrashTrash(ABaseItem* itme)
{
	int space{ HasSpace() };
	if (space >= 0)
	{
		knownItems.RemoveSingle(itme);
		inventory->GrabItem(space, itme);
		inventory->RemoveItem(space);
	}
}

bool USurvBrain_IndeherbergeFerentz::PickupItem(ABaseItem* itme)
{
	int space{ HasSpace() };
	
	// if i dont have a weapon yet, dont pick up
	if(HasWeapon() && space >= 0)
	{
		knownItems.RemoveSingle(itme);
		inventory->GrabItem(space, itme);
		return true;
	}
	//if i dont have a weapon && it's a weapon, pick up. previous should ensure there is space.
	else if (!HasWeapon() && (itme->GetItemType() == EItemType::Shotgun || itme->GetItemType() == EItemType::Pistol))
	{
		knownItems.RemoveSingle(itme);
		inventory->GrabItem(space, itme);
		return true;
	}
	// if not picked up, save.
	if(!knownItems.Contains(itme))
	knownItems.Add(itme);
	return false;
}

bool USurvBrain_IndeherbergeFerentz::SelectWeapon()
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
		return false;
	}
	else
	{
		return true;
	}
}

bool USurvBrain_IndeherbergeFerentz::HasWeapon()
{
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	int toRemove{ -1 };
	for (int i{}; i < Items.Num(); i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (InvItem->GetItemType() == EItemType::Shotgun || InvItem->GetItemType() == EItemType::Pistol)
			{
				return true;
			}
		}
	}
	return false;
}

void USurvBrain_IndeherbergeFerentz::Shoot()
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

bool USurvBrain_IndeherbergeFerentz::IsThreathed()
{
	closestZombie = NULL;
	if (knownZombies.Num() == 0) return false;

	knownZombies.RemoveAll([](const AActor* Actor)
		{
			// Invalid / destroyed / GC'd
			if (!IsValid(Actor))
			{
				return true;
			}

			// Illegal name check
			const FString Name = Actor->GetName();

			if (Name.Contains(TEXT("Illegal")) ||
				Name.IsEmpty())
			{
				return true;
			}

			return false;
		});


	bool threathened{false};
	FVector direction{};
	float closestZomDistance{ toloratedDistance };
	for (auto zombie : knownZombies)
	{
		if (!IsValid(zombie))
		{
			continue;
		}
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

bool USurvBrain_IndeherbergeFerentz::KnowsWeapon()
{
	knownWeapon = NULL;
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

void USurvBrain_IndeherbergeFerentz::ClearHouses()
{
	for (auto house : visitedHouses)
	{
		toVisitHouses.AddTail(house);
	}
	visitedHouses.Empty();
}

void USurvBrain_IndeherbergeFerentz::TryEat()
{
	int missingStamina = stamina->GetCurrentStamina() - stamina->GetMaxStamina();
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (int i{}; i < Items.Num(); i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (InvItem->GetItemType() == EItemType::Food && InvItem->GetValue() <= missingStamina)
			{
				inventory->UseItem(i);
				return;
			}
		}
	}
	stamina->AddStamina(10);
}

void USurvBrain_IndeherbergeFerentz::TryHeal()
{
	int missingHealth = health->GetMaxHealth() - health->GetHealth();
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (int i{}; i < Items.Num(); i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (InvItem->GetItemType() == EItemType::Medkit && InvItem->GetValue() <= missingHealth)
			{
				inventory->UseItem(i);
				return;
			}
		}
	}
	health->HealDamage(10);
}

int USurvBrain_IndeherbergeFerentz::HasSpace()
{
	bool InventoryClean{ false };
	while (!InventoryClean)
	{
		InventoryClean = ClearUpInventory();
	}
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (int i{}; i < Items.Num(); i++)
	{
		if (Items[i] == nullptr)
		{
			return i;
		}
	}
	return -1;
}

bool USurvBrain_IndeherbergeFerentz::ClearUpInventory()
{
	const TArray<ABaseItem*>& Items = inventory->GetInventory();
	for (int i{}; i < Items.Num(); i++)
	{
		ABaseItem* InvItem = Items[i];
		if (IsValid(InvItem))
		{
			if (InvItem->GetValue() == 0)
			{
				inventory->RemoveItem(i);
				return false;
			}
		}
	}
	return true;
}

#pragma endregion acions

