// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"

#include "SurvBrain.generated.h"

class UHealthComponent;
class UStaminaComponent;
class UInventoryComponent;
class ABaseItem;
class AWeapon;
class AHouse;
class ABaseZombie;
class UBlackboardComponent;
class USurvivorState;
class ASurvivorPawn;

UENUM(BlueprintType)
enum class EGoalType : uint8
{
	Loot UMETA(DisplayName = "Loot"),
	Search UMETA(DisplayName = "Search"),
	Hide UMETA(DisplayName = "Hide")
};


UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INDEHERBERGEFERENTZZOMBIERUNTIME_API USurvBrain : public UActorComponent
{
	GENERATED_BODY()

	
	void PickupItem(ABaseItem* itme);
	int maxInventory;
	int invenorySlot{0};

	
	TArray<AHouse*> toVisitHouses;
	TArray<AHouse*> visitedHouses;
	TArray<ABaseZombie*> knownZombies;
	AWeapon* selectedWeapon;

public:	
	// Sets default values for this component's properties
	USurvBrain();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGoalType goal;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void CompleteGoal();
	void CompleteLoot();
	bool executeState;
	void ExecuteState();
	EGoalType Search();
	EGoalType Loot();
	EGoalType Hide();
	bool SelectWeapon();
	
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

