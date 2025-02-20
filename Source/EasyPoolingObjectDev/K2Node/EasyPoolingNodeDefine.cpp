#include "EasyPoolingNodeDefine.h"

#include "EasyPoolingObject/Interface/EasyPoolingInterface.h"

bool FEasyPooingNodeHelper::CanSpawnActorOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract)
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

		// Exclude all types in the initial set by default.
		bCanSpawnObject = false;
		const UClass* CurrentClass = ObjectClass;

		// Climb up the class hierarchy and look for "BlueprintType." If "NotBlueprintType" is seen first, or if the class is not allowed, then stop searching.
		while (!bCanSpawnObject && CurrentClass != nullptr && !CurrentClass->GetBoolMetaData(NotBlueprintTypeName))
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
			CurrentClass = CurrentClass->GetSuperClass();
		}
	}

	if(!ObjectClass->ImplementsInterface(UEasyPoolingInterface::StaticClass()))
	{
		return false;
	}

	return bCanSpawnObject;
}

bool FEasyPooingNodeHelper::CanSpawnObjectOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract)
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
