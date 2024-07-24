// Ninja Bear Studio Inc., all rights reserved.
#include "NinjaGASFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

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
