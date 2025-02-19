// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasyPoolingObject/EasyPoolingActorDefine.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_RequestPoolableObject.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestSuccess,UObject*, RequestedObject);

/**
 * 
 */
UCLASS()
class EASYPOOLINGOBJECT_API UAsyncAction_RequestPoolableObject : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	virtual void Activate() override;
	virtual void Cancel() override;

public:
	UFUNCTION(BlueprintCallable, Category = "EasyPooling|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UAsyncAction_RequestPoolableObject* RequestPoolableObject(UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<UObject> Class);

	UFUNCTION(BlueprintCallable, Category = "EasyPooling|Tasks", meta = (BlueprintInternalUseOnly = "TRUE"))
	static UAsyncAction_RequestPoolableObject* RequestPoolableActor(UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<AActor> Class, FTransform Transform);

protected:
	UPROPERTY()
	TSubclassOf<UObject> RequestClass;

	UPROPERTY()
	FPoolingObjectRequestHandle RequestHandle;

	UPROPERTY()
	FTransform RequiredTransform;
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnRequestSuccess OnRequestSuccess;
	
};
