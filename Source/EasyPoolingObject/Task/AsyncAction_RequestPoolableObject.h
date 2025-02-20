// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasyPoolingObject/EasyPoolingActorDefine.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_RequestPoolableObject.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestObjectSuccess, UObject*, RequestedResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestActorSuccess, AActor*, RequestedResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRequestFailed);
/**
 * 
 */
UCLASS(BlueprintType,meta=(HasDedicatedAsyncNode))
class EASYPOOLINGOBJECT_API UAsyncAction_RequestPoolableObject : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	virtual void Activate() override;
	virtual void Cancel() override;

public:
	UFUNCTION(BlueprintCallable, Category = "EasyPooling|Tasks", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "TRUE"))
	static UAsyncAction_RequestPoolableObject* RequestPoolableObject(UObject* WorldContext, UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<UObject> Class, int32 InPriority = 1);

	void HandleRequestSuccess(UObject* ResultObject);
	
protected:
	UPROPERTY()
	TSubclassOf<UObject> RequestClass;

	UPROPERTY()
	FPoolingObjectRequestHandle RequestHandle;

	UPROPERTY()
	TWeakObjectPtr<UWorld> WeakWorld;

	int32 Priority = 1;

	bool bHasTrigger = false;
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestObjectSuccess OnRequestSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnRequestFailed OnRequestFailed;
	
};

UCLASS(BlueprintType,meta=(HasDedicatedAsyncNode))
class EASYPOOLINGOBJECT_API UAsyncAction_RequestPoolableActor : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	virtual void Activate() override;
	virtual void Cancel() override;

public:
	UFUNCTION(BlueprintCallable, Category = "EasyPooling|Tasks", meta = (WorldContext = "WorldContext", BlueprintInternalUseOnly = "TRUE"))
	static UAsyncAction_RequestPoolableActor* RequestPoolableActor(UObject* WorldContext, UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<AActor> Class, FTransform Transform);

protected:
	UPROPERTY()
	TSubclassOf<UObject> RequestClass;

	UPROPERTY()
	FPoolingObjectRequestHandle RequestHandle;

	UPROPERTY()
	FTransform RequiredTransform;
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestActorSuccess OnRequestSuccess;
	
};
