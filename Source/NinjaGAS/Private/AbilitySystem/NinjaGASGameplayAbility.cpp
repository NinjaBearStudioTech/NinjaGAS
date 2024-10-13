// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AbilitySystem/NinjaGASGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "NinjaGASTags.h"

bool UNinjaGASGameplayAbility::IsPassiveAbility() const
{
	return AbilityTags.HasTagExact(Tag_GAS_Ability_Passive);
}

void UNinjaGASGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (IsPassiveAbility())
	{
		static constexpr bool bAllowRemoteActivation = false;
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, bAllowRemoteActivation);
	}
}

void UNinjaGASGameplayAbility::EndAbilityFromBatch_Implementation()
{
	K2_EndAbility();
}

