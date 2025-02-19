// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EasyPoolingObjectDev/K2Node/GetPoolableObject/K2Node_GetPoolableObjectFromClass.h"
#include "K2Node_GetPoolableActorFromClass.generated.h"

/**
 * 
 */
UCLASS()
class EASYPOOLINGOBJECTDEV_API UK2Node_GetPoolableActorFromClass : public UK2Node_GetPoolableObjectFromClass
{
	GENERATED_BODY()

public:
	//~ Begin UEdGraphNode Interface.
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void EarlyValidation(class FCompilerResultsLog& MessageLog) const override;
	virtual bool IsSpawnVarPin(UEdGraphPin* Pin) const override;
	//~ End UEdGraphNode Interface.

protected:
	virtual UClass* GetClassPinBaseClass() const;
	
protected:
	virtual FText GetBaseNodeTitle() const;
	virtual FText GetDefaultNodeTitle() const;
	FText GetNodeTitleFormat() const;
};
