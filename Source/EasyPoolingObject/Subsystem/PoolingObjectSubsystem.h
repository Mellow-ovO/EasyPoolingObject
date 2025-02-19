﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EasyPoolingObject/EasyPoolingActorDefine.h"
#include "PoolingObjectSubsystem.generated.h"

/**
 * 
 */

USTRUCT()
struct FPooingObjectDetail
{
	GENERATED_BODY()

	UPROPERTY()
	TSet<UObject*> TotalPooingObject;

	UPROPERTY()
	TArray<UObject*> PooingObjectsCanUse;

	UPROPERTY()
	int32 Limit = -1;

	FPooingObjectDetail(int32 InLimit = -1)
		:Limit(InLimit)
	{
		
	}
};

USTRUCT()
struct FPooingObjectRequestQueue
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPoolingObjectRequest> PendingQueue;

	UPROPERTY()
	int32 Limit = -1;

	FPooingObjectRequestQueue(int32 InLimit = -1)
		:Limit(InLimit)
	{
		
	}
};

UCLASS(BlueprintType)
class EASYPOOLINGOBJECT_API UPoolingObjectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	UPROPERTY()
	TMap<TSubclassOf<UObject>,FPooingObjectDetail> PooingObjectDetailMap;
	
	UPROPERTY()
	TMap<TSubclassOf<UObject>,FPooingObjectRequestQueue> PendingQueueMap;

protected:
	void InitObjectDetail(TSubclassOf<UObject> Class);
	void InitObjectRequestQueue(TSubclassOf<UObject> Class);

public:
	UFUNCTION(BlueprintCallable,meta=(DeterminesOutputType = "Class"))
	UObject* TryGetObjectOfClass( UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<UObject> Class, bool bDelayActive = false);

	UFUNCTION(BlueprintCallable,meta=(DeterminesOutputType = "Class"))
	AActor* TryGetActorOfClass( UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<AActor> Class, FTransform Transform, bool bDelayActive = false);

	UFUNCTION(BlueprintCallable)
	void ReleaseObjectToPool(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void ActiveObject(UObject* Object);

	UFUNCTION(BlueprintCallable)
	void ActiveActor(AActor* Actor, FTransform Transform);

protected:
	UObject* Internal_GetObjectOfClass(TSubclassOf<UObject> Class, bool bDelayActive = false);
	AActor* Internal_GetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform, bool bDelayActive = false);

	void TryProcessRequest(TSubclassOf<UObject> Class);

public:
	FPoolingObjectRequestHandle RequestPoolingObject(FPoolingObjectRequest& Request);

	UFUNCTION(BlueprintCallable)
	void CancelRequestAndInvalidateHandle(TSubclassOf<UObject> Class, UPARAM(ref)FPoolingObjectRequestHandle& Handle);


protected:
	uint64 HandleIndex = 0;

protected:
	FPoolingObjectRequestHandle GenerateNewHandle();
	
};
