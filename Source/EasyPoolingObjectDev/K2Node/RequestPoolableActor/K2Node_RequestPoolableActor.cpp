// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_RequestPoolableActor.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "EditorCategoryUtils.h"
#include "FindInBlueprintManager.h"
#include "K2Node_CallArrayFunction.h"
#include "K2Node_CallFunction.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_EnumLiteral.h"
#include "KismetCompiler.h"
#include "EasyPoolingObject/Lib/EasyPoolingObjectFuncLib.h"
#include "EasyPoolingObjectDev/K2Node/EasyPoolingNodeDefine.h"
#include "EasyPoolingObject/Task/AsyncAction_RequestPoolableObject.h"
#include "EasyPoolingObjectDev/K2Node/BPInternalAsyncAction/K2Node_BPInternalAsyncAction.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "K2Node_RequestPoolableActorHelper"

UK2Node_RequestPoolableActor::UK2Node_RequestPoolableActor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void UK2Node_RequestPoolableActor::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

void UK2Node_RequestPoolableActor::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	ExpandSplitPins(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	UClass* ClassToSpawn = GetClassToSpawn();
	
	UK2Node_BPInternalAsyncAction* CallCreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_BPInternalAsyncAction>(this, SourceGraph);
	CallCreateNode->SetProxyFactoryFunctionName(ProxyFactoryFunctionName);
	CallCreateNode->SetProxyFactoryClass(ProxyFactoryClass);
	CallCreateNode->SetProxyClass(ProxyClass);
	CallCreateNode->AllocateDefaultPins();

	UK2Node_CallFunction* CallActiveNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallActiveNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UEasyPoolingObjectFuncLib, ActiveActor), UEasyPoolingObjectFuncLib::StaticClass());
	CallActiveNode->AllocateDefaultPins();

	UK2Node_DynamicCast* CastNode = CompilerContext.SpawnIntermediateNode<UK2Node_DynamicCast>(this, SourceGraph);
	CastNode->SetPurity(false);
	CastNode->TargetType = ClassToSpawn;
	CastNode->AllocateDefaultPins();

	bool bSucceeded = true;


	//connect exe
	{
		UEdGraphPin* SpawnExecPin = GetExecPin();
		UEdGraphPin* CallExecPin = CallCreateNode->GetExecPin();
		bSucceeded &= SpawnExecPin && CallExecPin && CompilerContext.MovePinLinksToIntermediate(*SpawnExecPin, *CallExecPin).CanSafeConnect();
	}

	//connect then
	{
		UEdGraphPin* SpawnThenPin = GetThenPin();
		UEdGraphPin* CallThenPin = CallCreateNode->GetThenPin();
		bSucceeded &= SpawnThenPin && CallThenPin && CompilerContext.MovePinLinksToIntermediate(*SpawnThenPin, *CallThenPin).CanSafeConnect();
	}

	//connect class
	{
		UEdGraphPin* SpawnClassPin = GetClassPin();
		UEdGraphPin* CallClassPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::ConstructClassPinName);
		bSucceeded &= SpawnClassPin && CallClassPin && CompilerContext.MovePinLinksToIntermediate(*SpawnClassPin, *CallClassPin).CanSafeConnect();
	}

	//connect priority
	{
		UEdGraphPin* SpawnPriorityPin = FindPin(FEasyPooingNodeHelper::PriorityPinName);
		UEdGraphPin* CallPriorityPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::PriorityPinName);
		bSucceeded &= SpawnPriorityPin && CallPriorityPin && CompilerContext.MovePinLinksToIntermediate(*SpawnPriorityPin, *CallPriorityPin).CanSafeConnect();
	}

	//connect transform
	{
		UEdGraphPin* SpawnTransformPin = FindPin(FEasyPooingNodeHelper::TransformPinName);
		UEdGraphPin* CallCreateTransformPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::TransformPinName);
		UEdGraphPin* CallActiveTransformPin = CallActiveNode->FindPin(FEasyPooingNodeHelper::TransformPinName);
		bSucceeded &= SpawnTransformPin && CallCreateTransformPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnTransformPin, *CallCreateTransformPin).CanSafeConnect();
		bSucceeded &= SpawnTransformPin && CallActiveTransformPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnTransformPin, *CallActiveTransformPin).CanSafeConnect();
	}

	//connect RequestFailed
	{
		UEdGraphPin* SpawnFailedPin = FindPin(FEasyPooingNodeHelper::RequestFailedDelegatePinName);
		UEdGraphPin* CastFailedPin = CastNode->GetInvalidCastPin();
		UEdGraphPin* CallFailedPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::RequestFailedDelegatePinName);
		bSucceeded &= SpawnFailedPin && CastFailedPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnFailedPin, *CastFailedPin).CanSafeConnect();
		bSucceeded &= SpawnFailedPin && CallFailedPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnFailedPin, *CallFailedPin).CanSafeConnect();
	}

	// connect cast input
	{
		UEdGraphPin* CallRequestResultPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::RequestResultPinName);
		UEdGraphPin* CastSourcePin = CastNode->GetCastSourcePin();
		CastSourcePin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		CallRequestResultPin->MakeLinkTo(CastSourcePin);
	}

	// connect cast
	{
		UEdGraphPin* CallRequestSuccessPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::RequestSuccessDelegatePinName);
		UEdGraphPin* CastExecPin = CastNode->GetExecPin();
		CallRequestSuccessPin->MakeLinkTo(CastExecPin);
	}

	UEdGraphPin* CastOutputPin = CastNode->GetCastResultPin();
	//connect active object
	{
		UEdGraphPin* ActiveObjectPin = CallActiveNode->FindPin(FEasyPooingNodeHelper::ActorPinName);
		CastOutputPin->MakeLinkTo(ActiveObjectPin);
	}

	//connect world context
	{
		UEdGraphPin* SpawnWorldContextPin = GetWorldContextPin();
		if(SpawnWorldContextPin)
		{
			UEdGraphPin* CallWorldContextPin = CallCreateNode->FindPin(FEasyPooingNodeHelper::WorldContextPinName);
			UEdGraphPin* ActiveWorldContextPin = CallActiveNode->FindPin(FEasyPooingNodeHelper::WorldContextPinName);
			bSucceeded &= SpawnWorldContextPin && CallWorldContextPin && CompilerContext.CopyPinLinksToIntermediate(*SpawnWorldContextPin, *CallWorldContextPin).CanSafeConnect();
			bSucceeded &= CompilerContext.CopyPinLinksToIntermediate(*SpawnWorldContextPin, *ActiveWorldContextPin).CanSafeConnect();
		}
	}

	UEdGraphPin* LastThenPin = CastNode->GetThenPin();
	//connect set var
	{
		bSucceeded &= ConnectSpawnProperties(ClassToSpawn,Schema,CompilerContext,SourceGraph,LastThenPin,CastOutputPin);
	}

	//connect active exec
	{
		UEdGraphPin* CallActiveExecPin = CallActiveNode->GetExecPin();
		LastThenPin->MakeLinkTo(CallActiveExecPin);
	}

	//connect spawn success
	{
		UEdGraphPin* CallActiveThenPin = CallActiveNode->GetThenPin();
		UEdGraphPin* SpawnSuccessPin = FindPin(FEasyPooingNodeHelper::RequestSuccessDelegatePinName);
		bSucceeded &= CompilerContext.MovePinLinksToIntermediate(*SpawnSuccessPin, *CallActiveThenPin).CanSafeConnect();
	}

	//connect spawn result
	{
		UEdGraphPin* SpawnResultPin = FindPin(FEasyPooingNodeHelper::RequestResultPinName);
		bSucceeded &= CompilerContext.MovePinLinksToIntermediate(*SpawnResultPin, *CastOutputPin).CanSafeConnect();
	}

	UEdGraphPin* ResultPin = GetRequestResultPin();
	ResultPin->PinType.PinSubCategoryObject = ClassToSpawn;
	
	
	BreakAllNodeLinks();

	if (!bSucceeded)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("GenericCreateObject_Error", "ICE: GenericCreateObject error @@").ToString(), this);
	}
}

void UK2Node_RequestPoolableActor::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);
	if (Pin && (Pin->PinName == FEasyPooingNodeHelper::ConstructClassPinName))
	{
		OnClassPinChanged();
		RefreshOutputPinType();
	}
}

FText UK2Node_RequestPoolableActor::GetKeywords() const
{
	return LOCTEXT("RequestPoolableActor", "Request");
}

bool UK2Node_RequestPoolableActor::HasExternalDependencies(TArray<UStruct*>* OptionalOutput) const
{
	UClass* SourceClass = GetClassToSpawn();
	const UBlueprint* SourceBlueprint = GetBlueprint();
	const bool bResult = (SourceClass && (SourceClass->ClassGeneratedBy.Get() != SourceBlueprint));
	if (bResult && OptionalOutput)
	{
		OptionalOutput->AddUnique(SourceClass);
	}
	const bool bSuperResult = Super::HasExternalDependencies(OptionalOutput);
	return bSuperResult || bResult;
}

// bool UK2Node_RequestPoolableActor::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
// {
// 	UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(TargetGraph);
// 	return Super::IsCompatibleWithGraph(TargetGraph) && (!Blueprint || (FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) != TargetGraph && Blueprint->GeneratedClass->GetDefaultObject()->ImplementsGetWorld()));
// }

void UK2Node_RequestPoolableActor::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	if (Pin && (Pin->PinName == FEasyPooingNodeHelper::ConstructClassPinName))
	{
		OnClassPinChanged();
		RefreshOutputPinType();
	}
}

void UK2Node_RequestPoolableActor::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	if (UClass* UseSpawnClass = GetClassToSpawn())
	{
		CreatePinsForClass(UseSpawnClass);
		RefreshOutputPinType();
	}
}

void UK2Node_RequestPoolableActor::AddSearchMetaDataInfo(TArray<FSearchTagDataPair>& OutTaggedMetaData) const
{
	Super::AddSearchMetaDataInfo(OutTaggedMetaData);
	OutTaggedMetaData.Add(FSearchTagDataPair(FFindInBlueprintSearchTags::FiB_NativeName, CachedNodeTitle.GetCachedText()));
}

void UK2Node_RequestPoolableActor::PostReconstructNode()
{
	Super::PostReconstructNode();
	RefreshOutputPinType();
}

FString UK2Node_RequestPoolableActor::GetPinMetaData(FName InPinName, FName InKey)
{
	if (InPinName == FName("Class") && InKey == FName("MustImplement"))
	{
		return "/Script/EasyPoolingObject.EasyPoolingInterface";
	}
	return Super::GetPinMetaData(InPinName, InKey);
}

void UK2Node_RequestPoolableActor::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	if(Pin == GetClassPin())
	{
		RefreshOutputPinType();
	}
}

void UK2Node_RequestPoolableActor::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	if (UClass* UseSpawnClass = GetClassToSpawn(&OldPins))
	{
		CreatePinsForClass(UseSpawnClass);
	}
	RestoreSplitPins(OldPins);
}

void UK2Node_RequestPoolableActor::GetNodeAttributes(TArray<TKeyValuePair<FString, FString>>& OutNodeAttributes) const
{
	UClass* ClassToSpawn = GetClassToSpawn();
	const FString ClassToSpawnStr = ClassToSpawn ? ClassToSpawn->GetName() : TEXT( "InvalidClass" );
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Type" ), TEXT( "RequestObjectFromClass" ) ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Class" ), GetClass()->GetName() ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "Name" ), GetName() ));
	OutNodeAttributes.Add( TKeyValuePair<FString, FString>( TEXT( "ObjectClass" ), ClassToSpawnStr ));
}

void UK2Node_RequestPoolableActor::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct GetMenuActions_Utils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_RequestPoolableActor* AsyncTaskNode = CastChecked<UK2Node_RequestPoolableActor>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());
						
				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass        = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass               = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UAsyncAction_RequestPoolableActor>(FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda([NodeClass](const UFunction* FactoryFunc)->UBlueprintNodeSpawner*
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
		check(NodeSpawner != nullptr);
		NodeSpawner->NodeClass = NodeClass;

		TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
		NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(GetMenuActions_Utils::SetNodeFunc, FunctionPtr);

		return NodeSpawner;
	}) );
}

FText UK2Node_RequestPoolableActor::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Gameplay);
}

void UK2Node_RequestPoolableActor::RefreshOutputPinType()
{
	UEdGraphPin* ResultPin = GetRequestResultPin();
	UClass* OldClass = (UClass*)(ResultPin->PinType.PinSubCategoryObject.Get());
	UClass* NewClass = GetClassToSpawn();
	if (OldClass != NewClass)
	{

		if (ResultPin->SubPins.Num() > 0)
		{
			GetSchema()->RecombinePin(ResultPin);
		}

		// NOTE: purposefully not disconnecting the ResultPin (even though it changed type)... we want the user to see the old
		//       connections, and incompatible connections will produce an error (plus, some super-struct connections may still be valid)
		ResultPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		ResultPin->PinType.PinSubCategoryObject = (NewClass == nullptr) ? UObject::StaticClass() : NewClass;
		CachedNodeTitle.Clear();
	}
	SetPinToolTip(*ResultPin);
}

bool UK2Node_RequestPoolableActor::UseWorldContext() const
{
	UBlueprint* BP = GetBlueprint();
	const UClass* ParentClass = BP ? BP->ParentClass : nullptr;
	return ParentClass ? ParentClass->HasMetaDataHierarchical(FBlueprintMetadata::MD_ShowWorldContextPin) != nullptr : false;
}

void UK2Node_RequestPoolableActor::CreatePinsForClass(UClass* InClass, TArray<UEdGraphPin*>* OutClassPins)
{
	check(InClass);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	const UObject* const ClassDefaultObject = InClass->GetDefaultObject(false);

	for (TFieldIterator<FProperty> PropertyIt(InClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;
		UClass* PropertyClass = CastChecked<UClass>(Property->GetOwner<UObject>());
		const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
		const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
		const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

		if(	bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags(CPF_Parm) && 
			bIsSettableExternally &&
			Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
			!bIsDelegate &&
			(nullptr == FindPin(Property->GetFName()) ) &&
			FBlueprintEditorUtils::PropertyStillExists(Property))
		{
			if (UEdGraphPin* Pin = CreatePin(EGPD_Input, NAME_None, Property->GetFName()))
			{
				K2Schema->ConvertPropertyToPinType(Property, /*out*/ Pin->PinType);
				if (OutClassPins)
				{
					OutClassPins->Add(Pin);
				}

				if (ClassDefaultObject && K2Schema->PinDefaultValueIsEditable(*Pin))
				{
					FString DefaultValueAsString;
					const bool bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(Property, reinterpret_cast<const uint8*>(ClassDefaultObject), DefaultValueAsString, this);
					check(bDefaultValueSet);
					K2Schema->SetPinAutogeneratedDefaultValue(Pin, DefaultValueAsString);
				}

				// Copy tooltip from the property.
				K2Schema->ConstructBasicPinTooltip(*Pin, Property->GetToolTipText(), Pin->PinToolTip);
			}
		}
	}

	// Change class of output pin
	UEdGraphPin* ResultPin = GetRequestResultPin();
	ResultPin->PinType.PinSubCategoryObject = InClass->GetAuthoritativeClass();
	SetPinToolTip(*ResultPin);
}

bool UK2Node_RequestPoolableActor::IsSpawnVarPin(UEdGraphPin* Pin) const
{
	if (Pin->ParentPin)
	{
		return IsSpawnVarPin(Pin->ParentPin);
	}

	if(Pin->Direction == EGPD_Output)
	{
		return false;
	}

	return(	Pin->PinName != UEdGraphSchema_K2::PN_Execute &&
			Pin->PinName != UEdGraphSchema_K2::PN_Then &&
			Pin->PinName != UEdGraphSchema_K2::PN_ReturnValue &&
			Pin->PinName != FEasyPooingNodeHelper::ConstructClassPinName &&
			Pin->PinName != FEasyPooingNodeHelper::WorldContextPinName &&
			Pin->PinName != FEasyPooingNodeHelper::PriorityPinName &&
			Pin->PinName != FEasyPooingNodeHelper::TransformPinName);
}

UEdGraphPin* UK2Node_RequestPoolableActor::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch /*= NULL*/) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == FEasyPooingNodeHelper::ConstructClassPinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_RequestPoolableActor::GetWorldContextPin() const
{
	UEdGraphPin* Pin = FindPin(FEasyPooingNodeHelper::WorldContextPinName);
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UEdGraphPin* UK2Node_RequestPoolableActor::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_RequestPoolableActor::GetRequestResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(FEasyPooingNodeHelper::RequestResultPinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_RequestPoolableActor::GetRequestSuccessPin() const
{
	UEdGraphPin* Pin = FindPinChecked(FEasyPooingNodeHelper::RequestSuccessDelegatePinName);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

void UK2Node_RequestPoolableActor::SetPinToolTip(UEdGraphPin& MutatablePin) const
{
	MutatablePin.PinToolTip = FString();
	UEdGraphSchema_K2 const* const K2Schema = Cast<const UEdGraphSchema_K2>(GetSchema());
	if (K2Schema != nullptr)
	{
		MutatablePin.PinToolTip += K2Schema->GetPinDisplayName(&MutatablePin).ToString();
		MutatablePin.PinToolTip += FString(TEXT("\n"));

	}
	MutatablePin.PinToolTip += UEdGraphSchema_K2::TypeToText(MutatablePin.PinType).ToString();
}

UClass* UK2Node_RequestPoolableActor::GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	UClass* UseSpawnClass = nullptr;
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* ClassPin = GetClassPin(PinsToSearch);
	if (ClassPin && ClassPin->DefaultObject && ClassPin->LinkedTo.Num() == 0)
	{
		UseSpawnClass = CastChecked<UClass>(ClassPin->DefaultObject);
	}
	else if (ClassPin && ClassPin->LinkedTo.Num())
	{
		UEdGraphPin* ClassSource = ClassPin->LinkedTo[0];
		UseSpawnClass = ClassSource ? Cast<UClass>(ClassSource->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return UseSpawnClass;
}

void UK2Node_RequestPoolableActor::OnClassPinChanged()
{
	// Remove all pins related to archetype variables
	TArray<UEdGraphPin*> OldPins = Pins;
	TArray<UEdGraphPin*> OldClassPins;

	for (UEdGraphPin* OldPin : OldPins)
	{
		if (IsSpawnVarPin(OldPin))
		{
			Pins.Remove(OldPin);
			OldClassPins.Add(OldPin);
		}
	}

	CachedNodeTitle.MarkDirty();

	TArray<UEdGraphPin*> NewClassPins;
	if (UClass* UseSpawnClass = GetClassToSpawn())
	{
		CreatePinsForClass(UseSpawnClass, &NewClassPins);
	}

	RestoreSplitPins(OldPins);

	UEdGraphPin* ResultPin = GetRequestResultPin();
	// Cache all the pin connections to the ResultPin, we will attempt to recreate them
	TArray<UEdGraphPin*> ResultPinConnectionList = ResultPin->LinkedTo;
	SetPinToolTip(*ResultPin);
	// Because the archetype has changed, we break the output link as the output pin type will change
	ResultPin->BreakAllPinLinks(true);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	// Recreate any pin links to the Result pin that are still valid
	for (UEdGraphPin* Connections : ResultPinConnectionList)
	{
		K2Schema->TryCreateConnection(ResultPin, Connections);
	}

	// Rewire the old pins to the new pins so connections are maintained if possible
	RewireOldPinsToNewPins(OldClassPins, Pins, nullptr);

	// Refresh the UI for the graph so the pin changes show up
	GetGraph()->NotifyGraphChanged();

	// Mark dirty
	FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
}


bool UK2Node_RequestPoolableActor::ConnectSpawnProperties(UClass* ClassToSpawn, const UEdGraphSchema_K2* Schema, class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, UEdGraphPin*& LastThenPin, UEdGraphPin* SpawnedActorReturnPin)
{
	bool bIsErrorFree = true;
	for (const auto& SpawnVarPin : Pins)
	{
		if(!IsSpawnVarPin(SpawnVarPin))
		{
			continue;
		}

		const bool bHasDefaultValue = !SpawnVarPin->DefaultValue.IsEmpty() || !SpawnVarPin->DefaultTextValue.IsEmpty() || SpawnVarPin->DefaultObject;
		if (SpawnVarPin->LinkedTo.Num() > 0 || bHasDefaultValue)
		{
			if (SpawnVarPin->LinkedTo.Num() == 0)
			{
				FProperty* Property = FindFProperty<FProperty>(ClassToSpawn, SpawnVarPin->PinName);
				// NULL property indicates that this pin was part of the original node, not the 
				// class we're assigning to:
				if (!Property)
				{
					continue;
				}

				// This is sloppy, we should be comparing to defaults much later in the compile process:
				if (ClassToSpawn->ClassDefaultObject != nullptr)
				{
					// We don't want to generate an assignment node unless the default value 
					// differs from the value in the CDO:
					FString DefaultValueAsString;
					FBlueprintEditorUtils::PropertyValueToString(Property, (uint8*)ClassToSpawn->ClassDefaultObject.Get(), DefaultValueAsString, this);
					if (DefaultValueAsString == SpawnVarPin->DefaultValue)
					{
						continue;
					}
				}
			}


			UFunction* SetByNameFunction = Schema->FindSetVariableByNameFunction(SpawnVarPin->PinType);
			if (SetByNameFunction)
			{
				UK2Node_CallFunction* SetVarNode = nullptr;
				if (SpawnVarPin->PinType.IsArray())
				{
					SetVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallArrayFunction>(this, SourceGraph);
				}
				else
				{
					SetVarNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
				}
				SetVarNode->SetFromFunction(SetByNameFunction);
				SetVarNode->AllocateDefaultPins();

				// Connect this node into the exec chain
				bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, SetVarNode->GetExecPin());
				LastThenPin = SetVarNode->GetThenPin();

				static const FName ObjectParamName(TEXT("Object"));
				static const FName ValueParamName(TEXT("Value"));
				static const FName PropertyNameParamName(TEXT("PropertyName"));

				// Connect the new actor to the 'object' pin
				UEdGraphPin* ObjectPin = SetVarNode->FindPinChecked(ObjectParamName);
				SpawnedActorReturnPin->MakeLinkTo(ObjectPin);

				// Fill in literal for 'property name' pin - name of pin is property name
				UEdGraphPin* PropertyNamePin = SetVarNode->FindPinChecked(PropertyNameParamName);
				PropertyNamePin->DefaultValue = SpawnVarPin->PinName.ToString();

				UEdGraphPin* ValuePin = SetVarNode->FindPinChecked(ValueParamName);
				if (SpawnVarPin->LinkedTo.Num() == 0 &&
					SpawnVarPin->DefaultValue != FString() &&
					SpawnVarPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Byte &&
					SpawnVarPin->PinType.PinSubCategoryObject.IsValid() &&
					SpawnVarPin->PinType.PinSubCategoryObject->IsA<UEnum>())
				{
					// Pin is an enum, we need to alias the enum value to an int:
					UK2Node_EnumLiteral* EnumLiteralNode = CompilerContext.SpawnIntermediateNode<UK2Node_EnumLiteral>(this, SourceGraph);
					EnumLiteralNode->Enum = CastChecked<UEnum>(SpawnVarPin->PinType.PinSubCategoryObject.Get());
					EnumLiteralNode->AllocateDefaultPins();
					EnumLiteralNode->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue)->MakeLinkTo(ValuePin);

					UEdGraphPin* InPin = EnumLiteralNode->FindPinChecked(UK2Node_EnumLiteral::GetEnumInputPinName());
					InPin->DefaultValue = SpawnVarPin->DefaultValue;
				}
				else
				{
					// For non-array struct pins that are not linked, transfer the pin type so that the node will expand an auto-ref that will assign the value by-ref.
					if (SpawnVarPin->PinType.IsArray() == false && SpawnVarPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && SpawnVarPin->LinkedTo.Num() == 0)
					{
						ValuePin->PinType.PinCategory = SpawnVarPin->PinType.PinCategory;
						ValuePin->PinType.PinSubCategory = SpawnVarPin->PinType.PinSubCategory;
						ValuePin->PinType.PinSubCategoryObject = SpawnVarPin->PinType.PinSubCategoryObject;
						CompilerContext.MovePinLinksToIntermediate(*SpawnVarPin, *ValuePin);
					}
					else
					{
						// Move connection from the variable pin on the spawn node to the 'value' pin
						CompilerContext.MovePinLinksToIntermediate(*SpawnVarPin, *ValuePin);
						SetVarNode->PinConnectionListChanged(ValuePin);
					}
				}
			}
		}
	}
	return bIsErrorFree;
}

#undef LOCTEXT_NAMESPACE