// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_BPInternalAsyncAction.generated.h"

/**
 * 
 */
UCLASS()
class EASYPOOLINGOBJECTDEV_API UK2Node_BPInternalAsyncAction : public UK2Node_AsyncAction
{
	GENERATED_BODY()
	
public:
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

	void SetProxyFactoryFunctionName(FName InName);
	void SetProxyFactoryClass(TObjectPtr<UClass> InClass);
	void SetProxyClass(TObjectPtr<UClass> InClass);
	void SetProxyActivateFunctionName(FName InName);
};
