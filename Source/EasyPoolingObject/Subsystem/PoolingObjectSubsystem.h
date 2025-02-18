// Fill out your copyright notice in the Description page of Project Settings.

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
	TArray<UObject*> TotalPooingObject;

	UPROPERTY()
	TArray<UObject*> PooingObjectsCanUse;

	UPROPERTY()
	int32 Limit = -1;
};

USTRUCT()
struct FPooingObjectRequestQueue
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPoolingObjectRequest> PendingQueue;

	UPROPERTY()
	int32 Limit = -1;
};

UCLASS(BlueprintType)
class EASYPOOLINGOBJECT_API UPoolingObjectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UObject* TryGetObjectOfClass(TSubclassOf<UObject> Class);
	AActor* TryGetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform);

protected:
	UObject* Internal_GetObjectOfClass(TSubclassOf<UObject> Class);
	AActor* Internal_GetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform);

public:
	FPoolingObjectRequestHandle RequestPoolingObject(const FPoolingObjectRequest& Request);

	UFUNCTION(BlueprintCallable)
	void CancelRequestAndInvalidateHandle(UPARAM(ref)FPoolingObjectRequestHandle& Handle);
	
protected:
	UPROPERTY()
	TMap<TSubclassOf<UObject>,FPooingObjectDetail> PooingObjectDetailMap;
	
	UPROPERTY()
	TMap<TSubclassOf<UObject>,FPooingObjectRequestQueue> PendingQueueMap;

protected:
	
};
