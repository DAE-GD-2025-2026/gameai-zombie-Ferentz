// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SurvivorSate.generated.h"

/**
 * 
 */
UCLASS()
class INDEHERBERGEFERENTZZOMBIERUNTIME_API USurvivorState : public UObject
{
	GENERATED_BODY()
public:
	virtual void OnEnter() {}
	virtual void OnExit() {}
	virtual USurvivorState* ExecuteState() { return NULL; }
};

class Loot final : public USurvivorState
{
public:
	virtual ~Loot() override {};
	virtual USurvivorState* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
class Search final : public USurvivorState
{
public:
	virtual ~Search() override {};
	virtual USurvivorState* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
class Hide final : public USurvivorState
{
public:
	virtual ~Hide() override {};
	virtual USurvivorState* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
