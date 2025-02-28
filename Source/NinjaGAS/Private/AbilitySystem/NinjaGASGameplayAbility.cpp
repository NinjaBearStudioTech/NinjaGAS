// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AbilitySystem/NinjaGASGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "NinjaGASTags.h"
#include "Runtime/Launch/Resources/Version.h"

bool UNinjaGASGameplayAbility::IsPassiveAbility() const
{
	return HasAbilityTag(Tag_GAS_Ability_Passive);
}

bool UNinjaGASGameplayAbility::IsInitialCooldown() const
{
	return HasAbilityTag(Tag_GAS_Ability_InitialCooldown);	
}

void UNinjaGASGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (IsPassiveAbility())
	{
		static constexpr bool bAllowRemoteActivation = false;
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, bAllowRemoteActivation);
	}

	if (IsInitialCooldown() && IsValid(CooldownGameplayEffectClass))
	{
		const FGameplayAbilitySpecHandle& Handle = GetCurrentAbilitySpecHandle();
		const FGameplayAbilityActivationInfo& ActivationInfo = GetCurrentActivationInfo();
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}	
}

void UNinjaGASGameplayAbility::EndAbilityFromBatch_Implementation()
{
	K2_EndAbility();
}

bool UNinjaGASGameplayAbility::HasAbilityTag(FGameplayTag AbilityTag) const
{
#if ENGINE_MINOR_VERSION < 5
	return AbilityTags.HasTagExact(AbilityTag);
#else
	return GetAssetTags().HasTagExact(AbilityTag);
#endif	
}
