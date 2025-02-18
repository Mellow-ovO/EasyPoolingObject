// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EasyPoolingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEasyPoolingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EASYPOOLINGOBJECT_API IEasyPoolingInterface : public IInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void PrepareForPooling(){};
	virtual void OnPoolingObjectConstruct(){};
	virtual void OnPoolingObjectActivate(){};
	virtual void OnPoolingObjectDeactivate(){};
	virtual void OnPoolingObjectDestruct(){};
};
