// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EasyPoolingObjectSettings.generated.h"

/**
 * 
 */

#define GET_EASY_POOLING_OBJECT_SETTING_PROPERTY(Property) (GetDefault<UEasyPoolingObjectSettings>()->Property);

UCLASS(config = Game ,DefaultConfig)
class EASYPOOLINGOBJECT_API UEasyPoolingObjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

public:
	UPROPERTY(Config,EditAnywhere,BlueprintReadWrite,meta=(MustImplement="/Script/EasyPoolingObject.EasyPoolingInterface"))
	TMap<TSubclassOf<UObject>,int32> MaxObjectPoolingNum;

	UPROPERTY(Config,EditAnywhere,BlueprintReadWrite,meta=(MustImplement="/Script/EasyPoolingObject.EasyPoolingInterface"))
	TMap<TSubclassOf<UObject>,int32> ObjectRequestQueueLengthLimit;
};
