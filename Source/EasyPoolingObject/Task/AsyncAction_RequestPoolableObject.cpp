// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_RequestPoolableObject.h"

void UAsyncAction_RequestPoolableObject::Activate()
{
	Super::Activate();
}

void UAsyncAction_RequestPoolableObject::Cancel()
{
	Super::Cancel();
}

UAsyncAction_RequestPoolableObject* UAsyncAction_RequestPoolableObject::RequestPoolableObject(
	TSubclassOf<UObject> Class)
{
	UAsyncAction_RequestPoolableObject* Obj = NewObject<UAsyncAction_RequestPoolableObject>();
	Obj->RequestClass = Class;
	return Obj;
}

UAsyncAction_RequestPoolableObject* UAsyncAction_RequestPoolableObject::RequestPoolableActor(TSubclassOf<AActor> Class,
	FTransform Transform)
{
	UAsyncAction_RequestPoolableObject* Obj = NewObject<UAsyncAction_RequestPoolableObject>();
	Obj->RequestClass = Class;
	Obj->RequiredTransform = Transform;
	return Obj;
}
