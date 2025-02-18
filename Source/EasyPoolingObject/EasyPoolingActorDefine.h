// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EasyPoolingActorDefine.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPoolingObjectRequestHandle
{
	GENERATED_BODY()

protected:
	int32 Handle = INDEX_NONE;

public:

	FPoolingObjectRequestHandle(int32 Index = INDEX_NONE)
		:Handle(Index)
	{
		
	}
	
	bool IsValid() const
	{
		return Handle != INDEX_NONE;
	}
};

