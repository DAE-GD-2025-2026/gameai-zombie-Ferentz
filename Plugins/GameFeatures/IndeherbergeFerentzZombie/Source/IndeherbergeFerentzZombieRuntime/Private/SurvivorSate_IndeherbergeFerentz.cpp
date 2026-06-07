// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivorSate_IndeherbergeFerentz.h"

USurvivorState_IndeherbergeFerentz* Loot::ExecuteState()
{
	return NewObject<Loot>(GetOuter());
}

void Loot::OnEnter()
{
}

void Loot::OnExit()
{
}

USurvivorState_IndeherbergeFerentz* Search::ExecuteState()
{
	return NULL;
}

void Search::OnEnter()
{
}

void Search::OnExit()
{
}

USurvivorState_IndeherbergeFerentz* Hide::ExecuteState()
{
	return NULL;
}

void Hide::OnEnter()
{
}

void Hide::OnExit()
{
}