// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "NinjaGASFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

UNinjaGASAbilitySystemComponent* UNinjaGASFunctionLibrary::GetCustomAbilitySystemComponentFromActor(AActor* Owner)
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	return Cast<UNinjaGASAbilitySystemComponent>(ASC);	
}

int32 UNinjaGASFunctionLibrary::SendGameplayEventToActor(const AActor* AbilityOwner, const FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner);
	return SendGameplayEventToComponent(AbilityComponent, EventTag, EventData);
}

int32 UNinjaGASFunctionLibrary::SendGameplayEventToComponent(UAbilitySystemComponent* AbilityComponent, const FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	if (!EventTag.IsValid() || !IsValid(AbilityComponent))
	{
		return 0;
	}
	
	return AbilityComponent->HandleGameplayEvent(EventTag, &EventData);	
}

void UNinjaGASFunctionLibrary::AddGameplayCueLocally(AActor* Target, const FGameplayTag GameplayCueTag,
	const FGameplayCueParameters& GameplayCueParameters) const
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}
	
	const UNinjaGASAbilitySystemComponent* AbilityComponent = GetCustomAbilitySystemComponentFromActor(Target);
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AddGameplayCueLocally(GameplayCueTag, GameplayCueParameters);
	}
}

void UNinjaGASFunctionLibrary::RemoveGameplayCueLocally(AActor* Target, const FGameplayTag GameplayCueTag,
	const FGameplayCueParameters& GameplayCueParameters) const
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	const UNinjaGASAbilitySystemComponent* AbilityComponent = GetCustomAbilitySystemComponentFromActor(Target);
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->RemoveGameplayCueLocally(GameplayCueTag, GameplayCueParameters);
	}	
}
