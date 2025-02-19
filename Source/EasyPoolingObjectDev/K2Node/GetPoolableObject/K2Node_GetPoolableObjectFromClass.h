// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_GetPoolableObjectFromClass.generated.h"

/**
 * 
 */

class UK2Node_CallFunction;

UCLASS()
class EASYPOOLINGOBJECTDEV_API UK2Node_GetPoolableObjectFromClass : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY()

public:
	virtual bool UseOuter() const override;
	virtual bool UseWorldContext() const override;

public:
	//~ Begin UEdGraphNode Interface.
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void EarlyValidation(class FCompilerResultsLog& MessageLog) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;
	virtual FString GetPinMetaData(FName InPinName, FName InKey) override;
	//~ End UEdGraphNode Interface.

protected:
	bool ExpandDefaultToSelfPin(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, UK2Node_CallFunction* CallCreateNode);

protected:
	virtual FText GetBaseNodeTitle() const;
	virtual FText GetDefaultNodeTitle() const;
	FText GetNodeTitleFormat() const;
	virtual UClass* GetClassPinBaseClass() const;
};
