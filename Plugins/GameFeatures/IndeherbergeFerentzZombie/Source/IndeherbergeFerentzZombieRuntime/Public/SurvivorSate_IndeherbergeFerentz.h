// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SurvivorSate_IndeherbergeFerentz.generated.h"

/**
 * 
 */
UCLASS()
class INDEHERBERGEFERENTZZOMBIERUNTIME_API USurvivorState_IndeherbergeFerentz : public UObject
{
	GENERATED_BODY()
public:
	virtual void OnEnter() {}
	virtual void OnExit() {}
	virtual USurvivorState_IndeherbergeFerentz* ExecuteState() { return NULL; }
};

class Loot final : public USurvivorState_IndeherbergeFerentz
{
public:
	virtual ~Loot() override {};
	virtual USurvivorState_IndeherbergeFerentz* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
class Search final : public USurvivorState_IndeherbergeFerentz
{
public:
	virtual ~Search() override {};
	virtual USurvivorState_IndeherbergeFerentz* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
class Hide final : public USurvivorState_IndeherbergeFerentz
{
public:
	virtual ~Hide() override {};
	virtual USurvivorState_IndeherbergeFerentz* ExecuteState() override;
	virtual void OnEnter() override;
	virtual void OnExit() override;
};
