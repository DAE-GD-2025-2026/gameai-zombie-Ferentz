// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptor_IndeherbergeFerentz.generated.h"

class USurvBrain_IndeherbergeFerentz;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INDEHERBERGEFERENTZZOMBIERUNTIME_API UStudentPerceptor_IndeherbergeFerentz : public UActorComponent
{
	GENERATED_BODY()

	AActor* parent;
	USurvBrain_IndeherbergeFerentz* brain;
	UAIPerceptionComponent* parentPerception;

	FAISenseID sightId;
	FAISenseID hurtId;

public:
	// Sets default values for this component's properties
	UStudentPerceptor_IndeherbergeFerentz();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
