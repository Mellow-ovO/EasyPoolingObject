// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_BPInternalAsyncAction.h"

void UK2Node_BPInternalAsyncAction::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	
}

void UK2Node_BPInternalAsyncAction::SetProxyFactoryFunctionName(FName InName)
{
	ProxyFactoryFunctionName = InName;
}

void UK2Node_BPInternalAsyncAction::SetProxyFactoryClass(TObjectPtr<UClass> InClass)
{
	ProxyFactoryClass = InClass;
}

void UK2Node_BPInternalAsyncAction::SetProxyClass(TObjectPtr<UClass> InClass)
{
	ProxyClass = InClass;
}

void UK2Node_BPInternalAsyncAction::SetProxyActivateFunctionName(FName InName)
{
	ProxyActivateFunctionName = InName;
}
