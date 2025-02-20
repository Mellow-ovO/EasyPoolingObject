// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_RequestPoolableObject.h"

#include "EasyPoolingObject/Subsystem/PoolingObjectSubsystem.h"

void UAsyncAction_RequestPoolableObject::Activate()
{
	Super::Activate();
	UWorld* World = WeakWorld.Get();
	if(!IsValid(World))
	{
		OnRequestFailed.Broadcast();
		SetReadyToDestroy();
		return;
	}
	UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
	FPoolingObjectRequest Request = FPoolingObjectRequest(RequestClass,Priority,true);
	Request.RequestSuccessDelegate = FPostPoolingActorRequestSuccess::CreateUObject(this,&UAsyncAction_RequestPoolableObject::HandleRequestSuccess);
	RequestHandle = PoolingObjectSubsystem->RequestPoolingObject(Request);
	if(!RequestHandle.IsValid() && !bHasTrigger)
	{
		OnRequestFailed.Broadcast();
		SetReadyToDestroy();
	}
}

void UAsyncAction_RequestPoolableObject::Cancel()
{
	UWorld* World = WeakWorld.Get();
	if(IsValid(World))
	{
		UPoolingObjectSubsystem* PoolingObjectSubsystem = World->GetSubsystem<UPoolingObjectSubsystem>();
		PoolingObjectSubsystem->CancelRequestAndInvalidateHandle(RequestClass,RequestHandle);
	}
	Super::Cancel();
}

UAsyncAction_RequestPoolableObject* UAsyncAction_RequestPoolableObject::RequestPoolableObject(UObject* WorldContext, 
	TSubclassOf<UObject> Class, int32 InPriority)
{
	UAsyncAction_RequestPoolableObject* Obj = NewObject<UAsyncAction_RequestPoolableObject>();
	Obj->RequestClass = Class;
	Obj->WeakWorld = WorldContext->GetWorld();
	Obj->Priority = InPriority;
	return Obj;
}

void UAsyncAction_RequestPoolableObject::HandleRequestSuccess(UObject* ResultObject)
{
	if(IsValid(ResultObject))
	{
		OnRequestSuccess.Broadcast(ResultObject);
	}
	bHasTrigger = true;
	SetReadyToDestroy();
}

void UAsyncAction_RequestPoolableActor::Activate()
{
	Super::Activate();
}

void UAsyncAction_RequestPoolableActor::Cancel()
{
	Super::Cancel();
}

UAsyncAction_RequestPoolableActor* UAsyncAction_RequestPoolableActor::RequestPoolableActor(UObject* WorldContext, TSubclassOf<AActor> Class,
                                                                                           FTransform Transform)
{
	UAsyncAction_RequestPoolableActor* Obj = NewObject<UAsyncAction_RequestPoolableActor>();
	Obj->RequestClass = Class;
	Obj->RequiredTransform = Transform;
	return Obj;
}
