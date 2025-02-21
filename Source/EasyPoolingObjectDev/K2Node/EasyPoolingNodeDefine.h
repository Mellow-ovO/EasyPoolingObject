
#pragma once
#include "CoreMinimal.h"

struct FEasyPooingNodeHelper
{
	static inline FName ConstructClassPinName = "Class";
	static inline FName WorldContextPinName = "WorldContext";
	static inline FName TransformPinName = "Transform";
	static inline FName ObjectPinName = "Object";
	static inline FName ActorPinName = "Actor";
	static inline FName PriorityPinName = "InPriority";
	static inline FName DelayActivePinName = "bDelayActive";
	static inline FName RequestResultPinName = "RequestedResult";
	static inline FName RequestSuccessDelegatePinName = "OnRequestSuccess";
	static inline FName RequestFailedDelegatePinName = "OnRequestFailed";
	static inline FString PoolingInterfaceRef = "/Script/EasyPoolingObject.EasyPoolingInterface";

	static bool CanSpawnActorOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract);
	static bool CanSpawnObjectOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract);
};
