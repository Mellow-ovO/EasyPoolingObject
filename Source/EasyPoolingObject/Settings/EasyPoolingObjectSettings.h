// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EasyPoolingObjectSettings.generated.h"

/**
 * 
 */

#define GET_MAX_OBJECT_POOLING_NUM(Class) (GetDefault<UEasyPoolingObjectSettings>()->MaxObjectPoolingNum.FindRef(Class));

UCLASS(config = Game ,DefaultConfig)
class EASYPOOLINGOBJECT_API UEasyPoolingObjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

public:
	UPROPERTY(Config,EditAnywhere,BlueprintReadWrite,meta=(MustImplement="/Script/EasyPoolingObject.EasyPoolingInterface"))
	TMap<TSubclassOf<UObject>,int32> MaxObjectPoolingNum;
};
