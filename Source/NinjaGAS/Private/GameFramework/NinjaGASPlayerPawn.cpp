// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASPlayerPawn.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Runtime/Launch/Resources/Version.h"

ANinjaGASPlayerPawn::ANinjaGASPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(AbilitySystemComponentName))
{
#if ENGINE_MINOR_VERSION < 5
	MinNetUpdateFrequency = 33.f;
	NetUpdateFrequency = 66.f;
#else
	SetMinNetUpdateFrequency(33.f);
	SetNetUpdateFrequency(66.f);
#endif
	
	bInitializeAbilityComponentOnBeginPlay = false;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Mixed;	
}

void ANinjaGASPlayerPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsPlayerControlled())
	{
		InitializeFromPlayerState();
	}
}

void ANinjaGASPlayerPawn::UnPossessed()
{
	ClearAbilitySystemComponent();
	Super::UnPossessed();
}

void ANinjaGASPlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeFromPlayerState();
}

UAbilitySystemComponent* ANinjaGASPlayerPawn::GetAbilitySystemComponent() const
{
	if (PawnAbilitiesPtr.IsValid() && PawnAbilitiesPtr->IsValidLowLevelFast())
	{
		UAbilitySystemComponent* AbilitySystemComponent = PawnAbilitiesPtr.Get(); 
		return AbilitySystemComponent;
	}
	
	return nullptr;	
}

void ANinjaGASPlayerPawn::SetupAbilitySystemComponent(AActor* AbilitySystemOwner)
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
	PawnAbilitiesPtr = CustomAbilityComponent;	
}

void ANinjaGASPlayerPawn::ClearAbilitySystemComponent()
{
	if (PawnAbilitiesPtr.IsValid() && PawnAbilitiesPtr->IsValidLowLevelFast())
	{
		PawnAbilitiesPtr->ClearActorInfo();
		PawnAbilitiesPtr.Reset();
	}
}
