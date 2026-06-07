// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "SurvBrain.generated.h"

class UHealthComponent;
class UStaminaComponent;
class UInventoryComponent;
class UFloatingPawnMovement;
class ABaseItem;
class AWeapon;
class AHouse;
class ABaseZombie;
//class UBlackboardComponent;
class USurvivorState;
class ASurvivorPawn;

UENUM(BlueprintType)
enum class EGoalType : uint8
{

	Search UMETA(DisplayName = "Search"),
	Loot UMETA(DisplayName = "Loot"),
	Attack UMETA(DisplayName = "Attack"),
	Flee UMETA(DisplayName = "Flee"),
};

enum class ELootType : uint8
{
	House,
	Item,
	Weapon
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INDEHERBERGEFERENTZZOMBIERUNTIME_API USurvBrain : public UActorComponent
{
	GENERATED_BODY()

	//INPUT
	void SpotItem(ABaseItem* item);
	void SpotHouse(AHouse* item);
	void SpotZombie(ABaseZombie* item);


	// INVENTORY
	bool PickupItem(ABaseItem* itme);
	bool SelectWeapon();
	int HasSpace();
	bool HasWeapon();
	bool ClearUpInventory();

	//GOALS
	EGoalType goal;
	void PassGoal() { blackboard->SetValueAsEnum("goal", (uint8)goal); }
	bool IsGoalDone();
	bool executeGoal;
	void ExecuteGoal();
	void EvaluateGoal();
	EGoalType GetGoal();

	//search
	EGoalType Search();
	bool CompleteSearch();
	//loot
	EGoalType Loot();
	bool CompleteLoot();
	//attack
	EGoalType Attack();
	bool CompleteAttack();
	//flee
	EGoalType Flee();
	bool CompleteFlee();

	//STATE
	int selectedWeaponIdx{ -1 };
	bool IsThreathed();
	bool isThreathened{false};
	FVector fleeDirection;
	ABaseZombie* closestZombie{ NULL };
	float toloratedDistance{ 500 };

	bool KnowsWeapon();
	ABaseItem* knownWeapon{ NULL };

	ELootType lootObjectType;
	AActor* lootObject;

	float threatcounter{0.f};
	float threatUpdate{1.f};

	//DATA
	void TryHeal();
	void ClearHouses();
	TDoubleLinkedList<AHouse*> toVisitHouses;
	TDoubleLinkedList<AHouse*> visitedHouses;
	TArray<ABaseZombie*> knownZombies;
	TArray<ABaseItem*> knownItems;
	
	float houseTimer{};
	float const houseTurnover{20.0f};

public:	
	// Sets default values for this component's properties
	USurvBrain();
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	
	
	AActor* parent;
	ASurvivorPawn* Survivor;
	UInventoryComponent* inventory;
	UHealthComponent* health;
	UStaminaComponent* stamina;

	UBlackboardComponent* blackboard;

	UFloatingPawnMovement* floatingMovement{NULL};
	float normalMovement{};
	float fleeMovement{};

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void JobsDone(bool completedGoal);

	UFUNCTION(BlueprintCallable)
	void Shoot();

	void GetHurt(AActor* Actor, FAIStimulus Stimulus);
	void SpotThing(AActor* Actor, FAIStimulus Stimulus);
		
};

