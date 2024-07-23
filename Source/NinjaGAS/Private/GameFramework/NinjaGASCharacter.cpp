// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASCharacter.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Data/NinjaGASDataAsset.h"

ANinjaGASCharacter::ANinjaGASCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	bOverridesAbilitySettings = false;
	NetPriority = 2.f;
	MinNetUpdateFrequency = 11.f;
	NetUpdateFrequency = 33.f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

UAbilitySystemComponent* ANinjaGASCharacter::GetAbilitySystemComponent() const
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		UAbilitySystemComponent* AbilitySystemComponent = CharacterAbilitiesPtr.Get(); 
		return AbilitySystemComponent;
	}
	
	return nullptr;	
}

bool ANinjaGASCharacter::HasDefaultAbilitySettings() const
{
	return bOverridesAbilitySettings;
}

void ANinjaGASCharacter::GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutAttributeSets = DefaultAbilitySetup->DefaultAttributeSets;
	}
}

void ANinjaGASCharacter::GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultEffects = DefaultAbilitySetup->DefaultGameplayEffects;
	}
}

void ANinjaGASCharacter::GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultAbilities = DefaultAbilitySetup->DefaultGameplayAbilities;
	}
}

void ANinjaGASCharacter::SetupAbilitySystemComponent(AActor* AbilitySystemOwner)
{
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilitySystemOwner);
	if (!IsValid(AbilityComponent))
	{
		return;
	}

	UNinjaGASAbilitySystemComponent* CustomAbilityComponent = Cast<UNinjaGASAbilitySystemComponent>(AbilityComponent);
	if (!IsValid(CustomAbilityComponent))
	{
		return;
	}
	
	CustomAbilityComponent->InitAbilityActorInfo(AbilitySystemOwner, this);
	CharacterAbilitiesPtr = CustomAbilityComponent;	
}

void ANinjaGASCharacter::ClearAbilitySystemComponent()
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		CharacterAbilitiesPtr->ClearActorInfo();
		CharacterAbilitiesPtr.Reset();
	}	
}
