// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EasyPoolingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class EASYPOOLINGOBJECT_API UEasyPoolingInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class EASYPOOLINGOBJECT_API IEasyPoolingInterface : public IInterface
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent)
	void PostObjectConstruct();

	UFUNCTION(BlueprintNativeEvent)
	void PrepareForActive();
	
	UFUNCTION(BlueprintNativeEvent)
	void PrepareForPooling();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnObjectActivate();
	
	UFUNCTION(BlueprintNativeEvent)
	void OnObjectPooled();
};
