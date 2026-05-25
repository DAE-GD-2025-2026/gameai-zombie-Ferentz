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

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INDEHERBERGEFERENTZZOMBIERUNTIME_API USurvBrain : public UActorComponent
{
	GENERATED_BODY()

	//INPUT
	void SpotItem(ABaseItem* item);
	void SpotHouse(AHouse* item);
	void SpotZombie(ABaseZombie* item);


	// INVENTORY
	void PickupItem(ABaseItem* itme);
	bool SelectWeapon();

	int maxInventory;
	int invenorySlot{0};

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
	bool KnowsWeapon();
	ABaseItem* knownWeapon{ NULL };

	//DATA
	void TryHeal();
	void ClearHouses();
	TArray<AHouse*> toVisitHouses;
	TArray<AHouse*> visitedHouses;
	TArray<ABaseZombie*> knownZombies;
	TArray<ABaseItem*> knownItems;
	FVector fleeDirection;
	ABaseZombie* closestZombie{NULL};
	float toloratedDistance{ 10000 };
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

