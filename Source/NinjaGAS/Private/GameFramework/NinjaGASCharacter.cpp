// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASCharacter.h"

#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Data/NinjaGASDataAsset.h"

FName ANinjaGASCharacter::AbilitySystemComponentName = TEXT("AbilitySystemComponent");

ANinjaGASCharacter::ANinjaGASCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	bOverridesAbilitySettings = false;
	NetPriority = 2.f;
	MinNetUpdateFrequency = 11.f;
	NetUpdateFrequency = 33.f;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Minimal;

	CharacterAbilities = CreateOptionalDefaultSubobject<UNinjaGASAbilitySystemComponent>(AbilitySystemComponentName);
	if (IsValid(CharacterAbilities))
	{
		CharacterAbilities->SetIsReplicated(bReplicates);
		CharacterAbilities->SetReplicationMode(AbilityReplicationMode);	
	}
}

void ANinjaGASCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(CharacterAbilities))
	{
		// The ASC is created by this class. Initialize it on Begin Play.
		SetupAbilitySystemComponent(this);	
	}
}

UAbilitySystemComponent* ANinjaGASCharacter::GetAbilitySystemComponent() const
{
	return CharacterAbilities;
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
	if (IsValid(CharacterAbilities))
	{
		CharacterAbilities->InitAbilityActorInfo(AbilitySystemOwner, this);
	}
}

void ANinjaGASCharacter::ClearAbilitySystemComponent()
{
	if (IsValid(CharacterAbilities))
	{
		CharacterAbilities->ClearActorInfo();
	}	
}
