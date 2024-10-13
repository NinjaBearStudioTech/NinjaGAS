// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "GameFramework/NinjaGASPlayerCharacter.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

ANinjaGASPlayerCharacter::ANinjaGASPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(AbilitySystemComponentName))
{
	bInitializeAbilityComponentOnBeginPlay = false;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void ANinjaGASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		InitializeFromPlayerState();
	}
}

void ANinjaGASPlayerCharacter::UnPossessed()
{
	ClearAbilitySystemComponent();
	Super::UnPossessed();
}

void ANinjaGASPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeFromPlayerState();
}

UAbilitySystemComponent* ANinjaGASPlayerCharacter::GetAbilitySystemComponent() const
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		UAbilitySystemComponent* AbilitySystemComponent = CharacterAbilitiesPtr.Get(); 
		return AbilitySystemComponent;
	}
	
	return nullptr;	
}

void ANinjaGASPlayerCharacter::SetupAbilitySystemComponent(AActor* AbilitySystemOwner)
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

void ANinjaGASPlayerCharacter::ClearAbilitySystemComponent()
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		CharacterAbilitiesPtr->ClearActorInfo();
		CharacterAbilitiesPtr.Reset();
	}
}
