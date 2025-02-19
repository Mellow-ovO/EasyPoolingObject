// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_GetPoolableObjectFromClass.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Self.h"
#include "KismetCompiler.h"
#include "EasyPoolingObject/Interface/EasyPoolingInterface.h"
#include "EasyPoolingObject/Lib/EasyPoolingObjectFuncLib.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "K2Node_GetPoolableObjectFromClass"

struct FK2Node_GetPoolableObjectFromClass_Utils
{
	static bool CanSpawnObjectOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract)
	{
		// Initially include types that meet the basic requirements.
		// Note: CLASS_Deprecated is an inherited class flag, so any subclass of an explicitly-deprecated class also cannot be spawned.
		bool bCanSpawnObject = (nullptr != *ObjectClass)
			&& (bAllowAbstract || !ObjectClass->HasAnyClassFlags(CLASS_Abstract))
			&& !ObjectClass->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists);

		// UObject is a special case where if we are allowing abstract we are going to allow it through even though it doesn't have BlueprintType on it
		if (bCanSpawnObject && (!bAllowAbstract || (*ObjectClass != UObject::StaticClass())))
		{
			static const FName BlueprintTypeName(TEXT("BlueprintType"));
			static const FName NotBlueprintTypeName(TEXT("NotBlueprintType"));
			static const FName DontUseGenericSpawnObjectName(TEXT("DontUseGenericSpawnObject"));

			auto IsClassAllowedLambda = [](const UClass* InClass)
			{
				return InClass != AActor::StaticClass()
					&& InClass != UActorComponent::StaticClass();
			};

			// Exclude all types in the initial set by default.
			bCanSpawnObject = false;
			const UClass* CurrentClass = ObjectClass;

			// Climb up the class hierarchy and look for "BlueprintType." If "NotBlueprintType" is seen first, or if the class is not allowed, then stop searching.
			while (!bCanSpawnObject && CurrentClass != nullptr && !CurrentClass->GetBoolMetaData(NotBlueprintTypeName) && IsClassAllowedLambda(CurrentClass))
			{
				// Include any type that either includes or inherits 'BlueprintType'
				bCanSpawnObject = CurrentClass->GetBoolMetaData(BlueprintTypeName);

				// Stop searching if we encounter 'BlueprintType' with 'DontUseGenericSpawnObject'
				if (bCanSpawnObject && CurrentClass->GetBoolMetaData(DontUseGenericSpawnObjectName))
				{
					bCanSpawnObject = false;
					break;
				}

				CurrentClass = CurrentClass->GetSuperClass();
			}

			// If we validated the given class, continue walking up the hierarchy to make sure we exclude it if it's an Actor or ActorComponent derivative.
			while (bCanSpawnObject && CurrentClass != nullptr)
			{
				bCanSpawnObject &= IsClassAllowedLambda(CurrentClass);

				CurrentClass = CurrentClass->GetSuperClass();
			}
		}

		if(!ObjectClass->ImplementsInterface(UEasyPoolingInterface::StaticClass()))
		{
			return false;
		}

		return bCanSpawnObject;
	}
};

bool UK2Node_GetPoolableObjectFromClass::UseOuter() const
{
	return false;
}

bool UK2Node_GetPoolableObjectFromClass::UseWorldContext() const
{
	UBlueprint* BP = GetBlueprint();
	const UClass* ParentClass = BP ? BP->ParentClass : nullptr;
	return ParentClass ? ParentClass->HasMetaDataHierarchical(FBlueprintMetadata::MD_ShowWorldContextPin) != nullptr : false;
}

void UK2Node_GetPoolableObjectFromClass::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	
	UK2Node_CallFunction* CallCreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UEasyPoolingObjectFuncLib, TryGetObjectOfClass), UEasyPoolingObjectFuncLib::StaticClass());
	CallCreateNode->AllocateDefaultPins();

	UK2Node_CallFunction* CallActiveNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallActiveNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UEasyPoolingObjectFuncLib, ActiveObject), UEasyPoolingObjectFuncLib::StaticClass());
	CallActiveNode->AllocateDefaultPins();
	
	// connect GenericCreateObject's self pin to the self node
	bool bSucceeded = ExpandDefaultToSelfPin(CompilerContext, SourceGraph, CallCreateNode);

	// store off the class to spawn before we mutate pin connections:
	UClass* ClassToSpawn = GetClassToSpawn();

	//set delay active
	{
		UEdGraphPin* DelayActivePin = CallCreateNode->FindPin(TEXT("bDelayActive"));
		DelayActivePin->DefaultValue = "true";
	}

	//connect exe
	{
		UEdGraphPin* SpawnExecPin = GetExecPin();
		UEdGraphPin* CallExecPin = CallCreateNode->GetExecPin();
		bSucceeded &= SpawnExecPin && CallExecPin && CompilerContext.MovePinLinksToIntermediate(*SpawnExecPin, *CallExecPin).CanSafeConnect();
	}

	//connect class
	{
		UEdGraphPin* SpawnClassPin = GetClassPin();
		UEdGraphPin* CallClassPin = CallCreateNode->FindPin(TEXT("Class"));
		bSucceeded &= SpawnClassPin && CallClassPin && CompilerContext.MovePinLinksToIntermediate(*SpawnClassPin, *CallClassPin).CanSafeConnect();
	}
		
	//connect world context
	{
		UEdGraphPin* SpawnWorldContextPin = GetWorldContextPin();
		if(SpawnWorldContextPin)
		{
			UEdGraphPin* CallWorldContextPin = CallCreateNode->FindPin(TEXT("WorldContext"));
			UEdGraphPin* ActiveWorldContextPin = CallActiveNode->FindPin(TEXT("WorldContext"));
			bSucceeded &= SpawnWorldContextPin && CallWorldContextPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnWorldContextPin, *CallWorldContextPin).CanSafeConnect();
			bSucceeded &= CompilerContext.CopyPinLinksToIntermediate(*SpawnWorldContextPin, *ActiveWorldContextPin).CanSafeConnect();
		}
	}

	UEdGraphPin* CallResultPin = nullptr;
	//connect result
	{
		UEdGraphPin* SpawnResultPin = GetResultPin();
		CallResultPin = CallCreateNode->GetReturnValuePin();
		UEdGraphPin* ActiveObjectPin = CallActiveNode->FindPin(TEXT("Object"));
		// cast HACK. It should be safe. The only problem is native code generation.
		if (SpawnResultPin && CallResultPin)
		{
			CallResultPin->PinType = SpawnResultPin->PinType;
		}
		bSucceeded &= SpawnResultPin && CallResultPin && CompilerContext.MovePinLinksToIntermediate(*SpawnResultPin, *CallResultPin).CanSafeConnect();
		// bSucceeded &= CompilerContext.MovePinLinksToIntermediate(*ActiveObjectPin, *CallResultPin).CanSafeConnect();
		CallResultPin->MakeLinkTo(ActiveObjectPin);
	}

	//assign exposed values and connect then
	{
		UEdGraphPin* LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallCreateNode, this, CallResultPin, ClassToSpawn);
		UEdGraphPin* ActiveExecPin = CallActiveNode->GetExecPin();
		Schema->TryCreateConnection(LastThen, ActiveExecPin);

		UEdGraphPin* ActiveThenPin = CallActiveNode->GetThenPin();
		UEdGraphPin* SpawnNodeThen = GetThenPin();
		bSucceeded &= SpawnNodeThen && LastThen && CompilerContext.MovePinLinksToIntermediate(*SpawnNodeThen, *ActiveThenPin).CanSafeConnect();
	}

	BreakAllNodeLinks();

	if (!bSucceeded)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("GenericCreateObject_Error", "ICE: GenericCreateObject error @@").ToString(), this);
	}

}

void UK2Node_GetPoolableObjectFromClass::EarlyValidation(FCompilerResultsLog& MessageLog) const
{
	Super::EarlyValidation(MessageLog);
	UEdGraphPin* ClassPin = GetClassPin(&Pins);
	const bool bAllowAbstract = ClassPin && ClassPin->LinkedTo.Num();
	UClass* ClassToSpawn = GetClassToSpawn();
	if (!FK2Node_GetPoolableObjectFromClass_Utils::CanSpawnObjectOfClass(ClassToSpawn, bAllowAbstract))
	{
		MessageLog.Error(*FText::Format(LOCTEXT("GenericCreateObject_WrongClassFmt", "Cannot construct objects of type '{0}' in @@"), FText::FromString(GetPathNameSafe(ClassToSpawn))).ToString(), this);
	}
}

bool UK2Node_GetPoolableObjectFromClass::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
	return Super::IsCompatibleWithGraph(TargetGraph) && (!Blueprint || (FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph && Blueprint->GeneratedClass->GetDefaultObject()->ImplementsGetWorld()));
}

FString UK2Node_GetPoolableObjectFromClass::GetPinMetaData(FName InPinName, FName InKey)
{
	if (InPinName == FName("Class") && InKey == FName("MustImplement"))
	{
		return "/Script/EasyPoolingObject.EasyPoolingInterface";
	}
	return Super::GetPinMetaData(InPinName, InKey);
}

bool UK2Node_GetPoolableObjectFromClass::ExpandDefaultToSelfPin(FKismetCompilerContext& CompilerContext,
                                                                UEdGraph* SourceGraph, UK2Node_CallFunction* CallCreateNode)
{
	// ensure valid input
	if (!SourceGraph || !CallCreateNode)
	{
		return false;
	}

	// Connect a self reference pin if there is a TScriptInterface default to self
	if (const UFunction* TargetFunc = CallCreateNode->GetTargetFunction())
	{
		const FString& MetaData = TargetFunc->GetMetaData(FBlueprintMetadata::MD_DefaultToSelf);
		if (!MetaData.IsEmpty())
		{
			// Get the Self Pin from this so we can default it correctly
			const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
			if (UEdGraphPin* DefaultToSelfPin = Schema->FindSelfPin(*this, EGPD_Input))
			{
				// If it has no links then spawn a new self node here
				if (DefaultToSelfPin->LinkedTo.IsEmpty())
				{
					UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
					SelfNode->AllocateDefaultPins();
					UEdGraphPin* SelfPin = SelfNode->FindPinChecked(UEdGraphSchema_K2::PSC_Self);
					// Make a connection from this intermediate self pin to here
					return Schema->TryCreateConnection(DefaultToSelfPin, SelfPin);
				}
			}
		}
	}
	return true;
}

FText UK2Node_GetPoolableObjectFromClass::GetBaseNodeTitle() const
{
	return NSLOCTEXT("K2Node", "ConstructObject_BaseTitle", "Get Poolable Object From Class");
}

FText UK2Node_GetPoolableObjectFromClass::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("K2Node", "ConstructObject_Title_NONE", "Get NONE");
}

FText UK2Node_GetPoolableObjectFromClass::GetNodeTitleFormat() const
{
	return NSLOCTEXT("K2Node", "Construct", "Get {ClassName}");
}

UClass* UK2Node_GetPoolableObjectFromClass::GetClassPinBaseClass() const
{
	return Super::GetClassPinBaseClass();
}

#undef LOCTEXT_NAMESPACE
