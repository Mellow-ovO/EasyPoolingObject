// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EasyPoolingObjectFuncLib.generated.h"

/**
 * 
 */
UCLASS()
class EASYPOOLINGOBJECT_API UEasyPoolingObjectFuncLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable,meta=(WorldContext = "WorldContext",DeterminesOutputType = "Class"))
	static UObject* TryGetObjectOfClass(UObject* WorldContext, UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<UObject> Class, bool bDelayActive = false);

	UFUNCTION(BlueprintCallable,meta=(WorldContext = "WorldContext",DeterminesOutputType = "Class"))
	static AActor* TryGetActorOfClass(UObject* WorldContext, UPARAM(meta = (MustImplement = "/Script/EasyPoolingObject.EasyPoolingInterface")) TSubclassOf<AActor> Class, FTransform Transform, bool bDelayActive = false);

	UFUNCTION(BlueprintCallable,meta=(WorldContext = "WorldContext"))
	static void ReleaseObjectToPool(UObject* WorldContext, UObject* Object);

	UFUNCTION(BlueprintCallable,meta=(WorldContext = "WorldContext"))
	static void ActiveObject(UObject* WorldContext, UObject* Object);

	UFUNCTION(BlueprintCallable,meta=(WorldContext = "WorldContext"))
	static void ActiveActor(UObject* WorldContext, AActor* Actor, FTransform Transform);
};
