// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "Async/NinjaGASAction_WaitForAbilityEnd.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UNinjaGASAction_WaitForAbilityEnd* UNinjaGASAction_WaitForAbilityEnd::CreateAction(AActor* AbilityOwner, const FGameplayTagQuery AbilityCriteria)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(AbilityOwner, EGetWorldErrorMode::ReturnNull);
	if (!IsValid(World))
	{
		return nullptr;
	}

	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner);
	if (!IsValid(AbilityComponent))
	{
		return nullptr;
	}

	UNinjaGASAction_WaitForAbilityEnd* NewAction = NewObject<UNinjaGASAction_WaitForAbilityEnd>(AbilityOwner, StaticClass());
	NewAction->AbilitySystemPtr = AbilityComponent;
	NewAction->AbilityCriteria = AbilityCriteria;
	NewAction->RegisterWithGameInstance(World->GetGameInstance());
	return NewAction;	
}

void UNinjaGASAction_WaitForAbilityEnd::Activate()
{
	if (AbilitySystemPtr.IsValid())
	{
		AbilityEndedDelegateHandle = AbilitySystemPtr->OnAbilityEnded.AddUObject(this, &ThisClass::HandleAbilityEnded);
	}
}

void UNinjaGASAction_WaitForAbilityEnd::BeginDestroy()
{
	if (AbilityEndedDelegateHandle.IsValid() && AbilitySystemPtr.IsValid())
	{
		AbilitySystemPtr->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
		AbilityEndedDelegateHandle.Reset();
	}
	
	Super::BeginDestroy();
}

void UNinjaGASAction_WaitForAbilityEnd::HandleAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	if (AbilitySystemPtr.IsValid())
	{
		FGameplayTagContainer AbilityThatEndedTags;
		AbilityThatEndedTags.AppendTags(AbilityEndedData.AbilityThatEnded->GetAssetTags());

		if (AbilityThatEndedTags.MatchesQuery(AbilityCriteria))
		{
			const bool bWasCancelled = AbilityEndedData.bWasCancelled;
			OnAbilityEnded.Broadcast(bWasCancelled);	
		}
	}
}
