// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASGameplayAbility.h"

#include "AbilitySystemComponent.h"

void UNinjaGASGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (bIsPassiveAbility)
	{
		static constexpr bool bAllowRemoteActivation = false;
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, bAllowRemoteActivation);
	}
}

void UNinjaGASGameplayAbility::EndAbilityFromBatch_Implementation()
{
	K2_EndAbility();
}
