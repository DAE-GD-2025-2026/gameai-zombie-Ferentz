// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivorSate.h"

USurvivorState* Loot::ExecuteState()
{
	return NewObject<Loot>(GetOuter());
}

void Loot::OnEnter()
{
}

void Loot::OnExit()
{
}

USurvivorState* Search::ExecuteState()
{
	return NULL;
}

void Search::OnEnter()
{
}

void Search::OnExit()
{
}

USurvivorState* Hide::ExecuteState()
{
	return NULL;
}

void Hide::OnEnter()
{
}

void Hide::OnExit()
{
}