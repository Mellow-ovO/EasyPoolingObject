// Fill out your copyright notice in the Description page of Project Settings.


#include "EasyPoolingObjectFuncLib.h"

#include "EasyPoolingObject/Subsystem/PoolingObjectSubsystem.h"

UObject* UEasyPoolingObjectFuncLib::TryGetObjectOfClass(UObject* WorldContext, TSubclassOf<UObject> Class,
                                                        bool bDelayActive)
{
	if(!IsValid(WorldContext))
	{
		return nullptr;
	}
	UWorld* World = WorldContext->GetWorld();
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	if(PoolingObjectSubsystem)
	{
		return PoolingObjectSubsystem->TryGetObjectOfClass(Class,bDelayActive);
	}
	return nullptr;
}

AActor* UEasyPoolingObjectFuncLib::TryGetActorOfClass(UObject* WorldContext, TSubclassOf<AActor> Class,
	FTransform Transform, bool bDelayActive)
{
	if(!IsValid(WorldContext))
	{
		return nullptr;
	}
	UWorld* World = WorldContext->GetWorld();
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	if(PoolingObjectSubsystem)
	{
		return PoolingObjectSubsystem->TryGetActorOfClass(Class,Transform,bDelayActive);
	}
	return nullptr;
}

void UEasyPoolingObjectFuncLib::ReleaseObjectToPool(UObject* WorldContext, UObject* Object)
{
	if(!IsValid(WorldContext))
	{
		return;
	}
	UWorld* World = WorldContext->GetWorld();
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	if(PoolingObjectSubsystem)
	{
		PoolingObjectSubsystem->ReleaseObjectToPool(Object);
	}
}

void UEasyPoolingObjectFuncLib::ActiveObject(UObject* WorldContext, UObject* Object)
{
	if(!IsValid(WorldContext))
	{
		return;
	}
	UWorld* World = WorldContext->GetWorld();
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	if(PoolingObjectSubsystem)
	{
		PoolingObjectSubsystem->ActiveObject(Object);
	}
}

void UEasyPoolingObjectFuncLib::ActiveActor(UObject* WorldContext, AActor* Actor, FTransform Transform)
{
	if(!IsValid(WorldContext))
	{
		return;
	}
	UWorld* World = WorldContext->GetWorld();
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	if(PoolingObjectSubsystem)
	{
		PoolingObjectSubsystem->ActiveActor(Actor,Transform);
	}
}
