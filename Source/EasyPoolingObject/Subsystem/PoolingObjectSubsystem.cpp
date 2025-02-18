// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolingObjectSubsystem.h"

#include "EasyPoolingObject/EasyPoolingObject.h"
#include "EasyPoolingObject/Interface/EasyPoolingInterface.h"
#include "EasyPoolingObject/Settings/EasyPoolingObjectSettings.h"

void UPoolingObjectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPoolingObjectSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPoolingObjectSubsystem::InitObjectDetail(TSubclassOf<UObject> Class)
{
	if(PooingObjectDetailMap.Contains(Class))
	{
		return;
	}
	const TMap<TSubclassOf<UObject>,int32>& MaxObjectPoolingNumMap = GET_EASY_POOLING_OBJECT_SETTING_PROPERTY(MaxObjectPoolingNum);
	if(MaxObjectPoolingNumMap.Contains(Class))
	{
		PooingObjectDetailMap.Emplace(Class,MaxObjectPoolingNumMap.FindRef(Class));
		return;
	}
	PooingObjectDetailMap.Emplace(Class,-1);
}

void UPoolingObjectSubsystem::InitObjectRequestQueue(TSubclassOf<UObject> Class)
{
	if(PendingQueueMap.Contains(Class))
	{
		return;
	}
	const TMap<TSubclassOf<UObject>,int32>& ObjectRequestLimitMap = GET_EASY_POOLING_OBJECT_SETTING_PROPERTY(ObjectRequestQueueLengthLimit);
	if(ObjectRequestLimitMap.Contains(Class))
	{
		PendingQueueMap.Emplace(Class,ObjectRequestLimitMap.FindRef(Class));
		return;
	}
	PendingQueueMap.Emplace(Class,-1);
}

UObject* UPoolingObjectSubsystem::TryGetObjectOfClass(TSubclassOf<UObject> Class)
{
	if(!::IsValid(Class))
	{
		return nullptr;
	}
	if(Class->IsChildOf(AActor::StaticClass()))
	{
		ensureAlwaysMsgf(false,TEXT("UPoolingObjectSubsystem: Try Get Pooling Actor: %s, Please Use UPoolingObjectSubsystem::TryGetActorOfClass Instead"),*(Class->GetName()));
		return nullptr;
	}
	if(!PooingObjectDetailMap.Contains(Class))
	{
		InitObjectDetail(Class);
	}
	return Internal_GetObjectOfClass(Class);
}

AActor* UPoolingObjectSubsystem::TryGetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform)
{
	if(!::IsValid(Class))
	{
		return nullptr;
	}
	if(!(Class->IsChildOf(AActor::StaticClass())))
	{
		ensureAlwaysMsgf(false,TEXT("UPoolingObjectSubsystem: Try Get Pooling Actor, But Class %s Is Not An Actor Class"),*(Class->GetName()));
		return nullptr;
	}
	if(!PooingObjectDetailMap.Contains(Class))
	{
		InitObjectDetail(Class);
	}
	return Internal_GetActorOfClass(Class,Transform);
}

void UPoolingObjectSubsystem::ReleaseObjectToPool(UObject* Object)
{
	if(!IsValid(Object))
	{
		return;
	}
	UClass* Class = Object->GetClass();
	FPooingObjectDetail* Detail = PooingObjectDetailMap.Find(Class);
	if(Detail == nullptr || !Detail->TotalPooingObject.Contains(Object))
	{
		UE_LOG(LogEasyPoolingObject,Warning,TEXT("Attemp To Release An UnPoolingable Object"));
	}
	const bool bIsPoolableObject = Object->Implements<UEasyPoolingInterface>();
	if(bIsPoolableObject)
	{
		IEasyPoolingInterface* EasyPoolingInterface = Cast<IEasyPoolingInterface>(Object);
		EasyPoolingInterface->PrepareForPooling();
		EasyPoolingInterface->BP_PrepareForPooling();

		if(AActor* Actor = Cast<AActor>(Object))
		{
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
		}
		Detail->PooingObjectsCanUse.Add(Object);
		
		EasyPoolingInterface->OnPoolingObjectDeactivate();
		EasyPoolingInterface->BP_OnPoolingObjectDeactivate();

		TryProcessRequest(Class);
	}
}

UObject* UPoolingObjectSubsystem::Internal_GetObjectOfClass(TSubclassOf<UObject> Class)
{
	return nullptr;
}

AActor* UPoolingObjectSubsystem::Internal_GetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform)
{
	return nullptr;
}

void UPoolingObjectSubsystem::TryProcessRequest(TSubclassOf<AActor> Class)
{
}

FPoolingObjectRequestHandle UPoolingObjectSubsystem::RequestPoolingObject(FPoolingObjectRequest& Request)
{
	Request.Handle = GenerateNewHandle();
	return Request.Handle;
}

void UPoolingObjectSubsystem::CancelRequestAndInvalidateHandle(TSubclassOf<UObject> Class,
	FPoolingObjectRequestHandle& Handle)
{
}

FPoolingObjectRequestHandle UPoolingObjectSubsystem::GenerateNewHandle()
{
	++HandleIndex;
	return FPoolingObjectRequestHandle(HandleIndex);
}
