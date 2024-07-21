// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaAbilitiesPlayerState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NinjaAbilitySystemComponent.h"

FName ANinjaAbilitiesPlayerState::AbilityComponentName = TEXT("AbilitySystem");

ANinjaAbilitiesPlayerState::ANinjaAbilitiesPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	MinNetUpdateFrequency = 33.f;
	NetUpdateFrequency = 66.f;
	NetPriority = 3.f;

	AbilitySystemComponent = CreateOptionalDefaultSubobject<UNinjaAbilitySystemComponent>(AbilityComponentName);
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* ANinjaAbilitiesPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
