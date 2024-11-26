// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AbilitySystem/NinjaGASGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "NinjaGASTags.h"
#include "Runtime/Launch/Resources/Version.h"

bool UNinjaGASGameplayAbility::IsPassiveAbility() const
{
#if ENGINE_MINOR_VERSION < 5
	return AbilityTags.HasTagExact(Tag_GAS_Ability_Passive);
#else
	return GetAssetTags().HasTagExact(Tag_GAS_Ability_Passive);
#endif
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

