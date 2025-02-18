// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
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
};

UCLASS(BlueprintType)
class EASYPOOLINGOBJECT_API UPoolingObjectSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UObject* GetOrCreateObjectOfClass(TSubclassOf<UObject> Class);
	AActor* GetOrCreateActorOfClass()

	
public:
	UPROPERTY()
	TMap<TSubclassOf<UObject>,FPooingObjectDetail> PooingObjectDetailMap;
	
};
