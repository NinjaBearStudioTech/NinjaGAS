// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/ANinjaGASPlayerCharacter.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

AANinjaGASPlayerCharacter::AANinjaGASPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(AbilitySystemComponentName))
{
	AbilityReplicationMode = EGameplayEffectReplicationMode::Mixed;
}

void AANinjaGASPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		InitializeFromPlayerState();	
	}
}

void AANinjaGASPlayerCharacter::UnPossessed()
{
	ClearAbilitySystemComponent();
	Super::UnPossessed();
}

void AANinjaGASPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeFromPlayerState();
}

void AANinjaGASPlayerCharacter::InitializeFromPlayerState()
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (IsValid(PlayerController))
	{
		APlayerState* MyState = GetPlayerState<APlayerState>();
		if (IsValid(MyState))
		{
			NetPriority = MyState->NetPriority;
			MinNetUpdateFrequency = MyState->MinNetUpdateFrequency;
			NetUpdateFrequency = MyState->NetUpdateFrequency;
			
			SetupAbilitySystemComponent(MyState);
		}
		else
		{
			// We'll keep trying on next tick until the Player State replicates.
			GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::InitializeFromPlayerState);	
		}
	}
}

UAbilitySystemComponent* AANinjaGASPlayerCharacter::GetAbilitySystemComponent() const
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		UAbilitySystemComponent* AbilitySystemComponent = CharacterAbilitiesPtr.Get(); 
		return AbilitySystemComponent;
	}
	
	return nullptr;	
}

void AANinjaGASPlayerCharacter::SetupAbilitySystemComponent(AActor* AbilitySystemOwner)
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

void AANinjaGASPlayerCharacter::ClearAbilitySystemComponent()
{
	if (CharacterAbilitiesPtr.IsValid() && CharacterAbilitiesPtr->IsValidLowLevelFast())
	{
		CharacterAbilitiesPtr->ClearActorInfo();
		CharacterAbilitiesPtr.Reset();
	}
}
