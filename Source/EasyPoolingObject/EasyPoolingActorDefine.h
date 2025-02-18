// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EasyPoolingActorDefine.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_OneParam(FPostPoolingActorRequestSuccess, UObject*, ResultOject);


USTRUCT(BlueprintType)
struct FPoolingObjectRequestHandle
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	uint64 Handle = 0;

public:
	FPoolingObjectRequestHandle(int64 Index = 0)
		:Handle(Index)
	{
		
	}
	
	bool IsValid() const
	{
		return Handle != 0;
	}
};

USTRUCT(BlueprintType)
struct FPoolingObjectRequest
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TSubclassOf<UObject> RequestObjectClass;

	int32 Priority = 1;

	UPROPERTY()
	FPostPoolingActorRequestSuccess RequestSuccessDelegate;

	UPROPERTY()
	FPoolingObjectRequestHandle Handle;

public:
	FPoolingObjectRequest()
	{
		
	}

	FPoolingObjectRequest(TSubclassOf<UObject> RequestClass, int32 InPriority)
		:RequestObjectClass(RequestClass)
		,Priority(InPriority)
	{
		
	}
};


