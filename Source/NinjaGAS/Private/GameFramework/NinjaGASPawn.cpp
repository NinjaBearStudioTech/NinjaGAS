// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "GameFramework/NinjaGASPawn.h"

#include "NinjaGASFunctionLibrary.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"

FName ANinjaGASPawn::AbilitySystemComponentName = TEXT("AbilitySystemComponent");

ANinjaGASPawn::ANinjaGASPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	MinNetUpdateFrequency = 11.f;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Minimal;

	PawnAbilities = CreateOptionalDefaultSubobject<UNinjaGASAbilitySystemComponent>(AbilitySystemComponentName);
	if (IsValid(PawnAbilities))
	{
		PawnAbilities->SetIsReplicated(bReplicates);
		PawnAbilities->SetReplicationMode(AbilityReplicationMode);	
	}
}

void ANinjaGASPawn::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsValid(PawnAbilities))
	{
		// Reinforce the Replication Mode in the ASC.
		PawnAbilities->SetReplicationMode(AbilityReplicationMode);
	}
}

void ANinjaGASPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ANinjaGASPawn::BeginPlay()
{
	Super::BeginPlay();

	// This reinforces the ASC in this class, in case it was provided by a Game Feature.
	//
	// Doing this is useful as it makes this class compatible with both a gameplay feature and the
	// ASC interface, avoiding the component lookup.
	//
	PawnAbilities = UNinjaGASFunctionLibrary::GetCustomAbilitySystemComponentFromActor(this);
	if (IsValid(PawnAbilities))
	{
		PawnAbilities->InitAbilityActorInfo(this, this);
	}	
}

void ANinjaGASPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ANinjaGASPawn::GetAbilitySystemComponent() const
{
	return PawnAbilities;
}
