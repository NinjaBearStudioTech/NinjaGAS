// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "Async/NinjaGASAction_WaitForAbilitySystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "TimerManager.h"
#include "UnrealEngine.h"
#include "GameFramework/PlayerState.h"
#include "UObject/Package.h"

UNinjaGASAction_WaitForAbilitySystem* UNinjaGASAction_WaitForAbilitySystem::CreateAction(AActor* AbilityOwner, const float CheckInterval, const float MaxWait)
{
	UWorld* World = GEngine->GetWorldFromContextObject(AbilityOwner, EGetWorldErrorMode::ReturnNull);
	if (!IsValid(World))
	{
		return nullptr;
	}

	if (!IsValid(AbilityOwner))
	{
		return nullptr;
	}

	UNinjaGASAction_WaitForAbilitySystem* NewAction = NewObject<UNinjaGASAction_WaitForAbilitySystem>(GetTransientPackage(), StaticClass());
	NewAction->AbilityOwnerPtr = AbilityOwner;	
	NewAction->SetWorld(World);
	NewAction->SetTimerParameters(CheckInterval, MaxWait);
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;
}

UWorld* UNinjaGASAction_WaitForAbilitySystem::GetWorld() const
{
	if (WorldPtr.IsValid() && WorldPtr->IsValidLowLevelFast())
	{
		return WorldPtr.Get();
	}

	return nullptr;
}

void UNinjaGASAction_WaitForAbilitySystem::Activate()
{
	const AActor* AbilityOwner = GetAbilityOwner();
	if (!IsValid(AbilityOwner))
	{
		Cancel();
		return;
	}

	const UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner);
	if (IsValid(AbilityComponent) && !AbilityComponent->GetAvatarActor()->IsA<APlayerState>())
	{
		OnCompleted.Broadcast();
		Cancel();
	}
	else
	{
		AccumulateElapsedTime();
	}
}

void UNinjaGASAction_WaitForAbilitySystem::Cancel()
{
	Super::Cancel();

	OnCancelled.Broadcast();
	
	const UWorld* World = GetWorld();
	if(LookupHandle.IsValid() && IsValid(World))
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(LookupHandle);
	}
}

AActor* UNinjaGASAction_WaitForAbilitySystem::GetAbilityOwner() const
{
	return AbilityOwnerPtr.IsValid() ? AbilityOwnerPtr.Get() : nullptr;
}

void UNinjaGASAction_WaitForAbilitySystem::SetWorld(UWorld* World)
{
	WorldPtr = World;
}

void UNinjaGASAction_WaitForAbilitySystem::SetTimerParameters(const float InCheckInterval, const float InMaxWait)
{
	CheckInterval = InCheckInterval;
	MaxWait = InMaxWait;
}

void UNinjaGASAction_WaitForAbilitySystem::AccumulateElapsedTime()
{
	ElapsedTime += CheckInterval;
	if (ElapsedTime > MaxWait)
	{
		OnTimedOut.Broadcast();
		Cancel();
	}
}
