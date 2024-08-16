// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPlayerCharacter.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

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

void ANinjaGASPlayerCharacter::InitializeFromPlayerState()
{
	APlayerState* MyState = GetPlayerState<APlayerState>();
	if (!IsValid(MyState))
	{
		// We'll keep trying on next tick until the Player State replicates.
		// Return fast so nothing else will be done for now (including subclasses).
		//
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::InitializeFromPlayerState);
		return;
	}
	
	NetPriority = MyState->NetPriority;
	MinNetUpdateFrequency = MyState->MinNetUpdateFrequency;
	NetUpdateFrequency = MyState->NetUpdateFrequency;
	SetupAbilitySystemComponent(MyState);
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
