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
	const bool bIsPoolableClass = Class->ImplementsInterface(UEasyPoolingInterface::StaticClass());
	if(!bIsPoolableClass)
	{
		ensureAlwaysMsgf(false,TEXT("Try Get Unpoolable Object, Class: %s"),*(Class->GetName()));
		return nullptr;
	}
	if(Class->IsChildOf(AActor::StaticClass()))
	{
		ensureAlwaysMsgf(false,TEXT("Try Get Pooling Actor: %s, Please Use UPoolingObjectSubsystem::TryGetActorOfClass Instead"),*(Class->GetName()));
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
	const bool bIsPoolableClass = Class->ImplementsInterface(UEasyPoolingInterface::StaticClass());
	if(!bIsPoolableClass)
	{
		ensureAlwaysMsgf(false,TEXT("Try Get Unpoolable Object, Class: %s"),*(Class->GetName()));
		return nullptr;
	}
	if(!(Class->IsChildOf(AActor::StaticClass())))
	{
		ensureAlwaysMsgf(false,TEXT("Try Get Pooling Actor, But Class %s Is Not An Actor Class"),*(Class->GetName()));
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
			Actor->SetActorTickEnabled(false);
		}
		Detail->PooingObjectsCanUse.Add(Object);
		
		EasyPoolingInterface->OnPoolingObjectDeactivate();
		EasyPoolingInterface->BP_OnPoolingObjectDeactivate();

		TryProcessRequest(Class);
	}
}

UObject* UPoolingObjectSubsystem::Internal_GetObjectOfClass(TSubclassOf<UObject> Class)
{
	if(!IsValid(Class))
	{
		return nullptr;
	}
	FPooingObjectDetail* Detail = PooingObjectDetailMap.Find(Class);
	if(Detail == nullptr)
	{
		return nullptr;
	}
	if(Detail->Limit > 0 && Detail->TotalPooingObject.Num() >= Detail->Limit && Detail->PooingObjectsCanUse.IsEmpty())
	{
		return nullptr;
	}
	UObject* Object = nullptr;
	if(!Detail->PooingObjectsCanUse.IsEmpty())
	{
		Object = Detail->PooingObjectsCanUse.Pop(false);
	}
	else
	{
		Object = NewObject<UObject>(this,Class);
		Detail->TotalPooingObject.Add(Object);
		IEasyPoolingInterface* EasyPoolingInterface = Cast<IEasyPoolingInterface>(Object);
		if(EasyPoolingInterface)
		{
			EasyPoolingInterface->PostObjectConstruct();
		}
	}
	IEasyPoolingInterface* EasyPoolingInterface = Cast<IEasyPoolingInterface>(Object);
	if(EasyPoolingInterface)
	{
		EasyPoolingInterface->PrepareForActive();
		EasyPoolingInterface->BP_PrepareForActive();
		EasyPoolingInterface->OnPoolingObjectActivate();
		EasyPoolingInterface->BP_OnPoolingObjectActivate();
	}
	return Object;
}

AActor* UPoolingObjectSubsystem::Internal_GetActorOfClass(TSubclassOf<AActor> Class, FTransform Transform)
{
	if(!IsValid(Class))
	{
		return nullptr;
	}
	FPooingObjectDetail* Detail = PooingObjectDetailMap.Find(Class);
	if(Detail == nullptr)
	{
		return nullptr;
	}
	if(Detail->Limit > 0 && Detail->TotalPooingObject.Num() >= Detail->Limit && Detail->PooingObjectsCanUse.IsEmpty())
	{
		return nullptr;
	}
	AActor* Actor = nullptr;
	if(!Detail->PooingObjectsCanUse.IsEmpty())
	{
		Actor = Cast<AActor>(Detail->PooingObjectsCanUse.Pop(false));
	}
	else
	{
		Actor = NewObject<AActor>(this,Class);
		Detail->TotalPooingObject.Add(Actor);
		IEasyPoolingInterface* EasyPoolingInterface = Cast<IEasyPoolingInterface>(Actor);
		if(EasyPoolingInterface)
		{
			EasyPoolingInterface->PostObjectConstruct();
		}
	}
	IEasyPoolingInterface* EasyPoolingInterface = Cast<IEasyPoolingInterface>(Actor);
	if(EasyPoolingInterface)
	{
		EasyPoolingInterface->PrepareForActive();
		EasyPoolingInterface->BP_PrepareForActive();

		Actor->SetActorEnableCollision(true);
		Actor->SetActorHiddenInGame(false);
		const bool bIsDefaultTickEnable = Class->GetDefaultObject<AActor>()->IsActorTickEnabled();
		Actor->SetActorTickEnabled(bIsDefaultTickEnable);
		Actor->SetActorTransform(Transform);
		
		EasyPoolingInterface->OnPoolingObjectActivate();
		EasyPoolingInterface->BP_OnPoolingObjectActivate();
	}
	return Actor;
}

void UPoolingObjectSubsystem::TryProcessRequest(TSubclassOf<UObject> Class)
{
	FPooingObjectRequestQueue* Queue = PendingQueueMap.Find(Class);
	FPooingObjectDetail* Detail = PooingObjectDetailMap.Find(Class);
	if(Queue == nullptr || Queue->PendingQueue.IsEmpty())
	{
		return;
	}
	if(Detail->Limit > 0 && Detail->TotalPooingObject.Num() >= Detail->Limit && Detail->PooingObjectsCanUse.IsEmpty())
	{
		return;
	}
	int32 MinPriority = Queue->PendingQueue[0].Priority;
	int32 MinPriorityIndex = 0;
	for(int32 Index = 1; Index < Queue->PendingQueue.Num(); ++Index)
	{
		if(Queue->PendingQueue[Index].Priority < MinPriority)
		{
			MinPriority = Queue->PendingQueue[Index].Priority;
			MinPriorityIndex = Index;
		}
	}
	FPoolingObjectRequest Request = Queue->PendingQueue[MinPriorityIndex];
	Queue->PendingQueue.RemoveAtSwap(MinPriorityIndex);
	UObject* Object = nullptr;
	if(Request.RequestObjectClass->IsChildOf(AActor::StaticClass()))
	{
		Object = TryGetActorOfClass(Class.Get(),Request.ActorTransform);
	}
	else
	{
		Object = TryGetObjectOfClass(Class);
	}
	if(Object != nullptr)
	{
		Request.RequestSuccessDelegate.ExecuteIfBound(Object);
	}
	TryProcessRequest(Class);
}

FPoolingObjectRequestHandle UPoolingObjectSubsystem::RequestPoolingObject(FPoolingObjectRequest& Request)
{
	if(!IsValid(Request.RequestObjectClass))
	{
		return FPoolingObjectRequestHandle();
	}
	if(!Request.RequestSuccessDelegate.IsBound())
	{
		return FPoolingObjectRequestHandle();
	}
	if(!Request.RequestObjectClass->ImplementsInterface(UEasyPoolingInterface::StaticClass()))
	{
		ensureAlwaysMsgf(false,TEXT("Try Request Unpoolable Object, Class: %s"),*(Request.RequestObjectClass->GetName()));
		return FPoolingObjectRequestHandle();
	}
	FPooingObjectRequestQueue* Queue = PendingQueueMap.Find(Request.RequestObjectClass);
	if(Queue == nullptr)
	{
		InitObjectRequestQueue(Request.RequestObjectClass);
	}
	Queue = PendingQueueMap.Find(Request.RequestObjectClass);
	if(Queue->Limit > 0 && Queue->PendingQueue.Num() >= Queue->Limit)
	{
		UE_LOG(LogEasyPoolingObject,Warning,TEXT("Try Request Object, But Queue Is Full. Class: %s"),*(Request.RequestObjectClass->GetName()));
		return FPoolingObjectRequestHandle();
	}
	FPoolingObjectRequestHandle NewHandle = GenerateNewHandle();
	Request.Handle = NewHandle;
	Queue->PendingQueue.Add(Request);
	return NewHandle;
}

void UPoolingObjectSubsystem::CancelRequestAndInvalidateHandle(TSubclassOf<UObject> Class,
	FPoolingObjectRequestHandle& Handle)
{
	FPooingObjectRequestQueue* Queue = PendingQueueMap.Find(Class);
	if(Queue)
	{
		Queue->PendingQueue.RemoveAll([&Handle](const FPoolingObjectRequest& Item)
		{
			return Item.Handle == Handle;
		});
	}
	Handle.Invalidate();
}

FPoolingObjectRequestHandle UPoolingObjectSubsystem::GenerateNewHandle()
{
	++HandleIndex;
	return FPoolingObjectRequestHandle(HandleIndex);
}
