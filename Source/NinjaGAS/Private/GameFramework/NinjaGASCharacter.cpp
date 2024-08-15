// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaGASCharacter.h"

#include "NinjaGASFunctionLibrary.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"

FName ANinjaGASCharacter::AbilitySystemComponentName = TEXT("AbilitySystemComponent");

ANinjaGASCharacter::ANinjaGASCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	bInitializeAbilityComponentOnBeginPlay = true;
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

void ANinjaGASCharacter::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsValid(CharacterAbilities))
	{
		// Set the Replication Mode after properties are initialized but before components.
		// This way, once the Ability System Component initializes, it has the correct value.
		//
		CharacterAbilities->SetReplicationMode(AbilityReplicationMode);
	}	
}

void ANinjaGASCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ANinjaGASCharacter::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();

	if (bInitializeAbilityComponentOnBeginPlay)
	{
		// This reinforces the ASC in this class, in case it was provided by a Game Feature.
		// If found, initializes the instance using the appropriate setup method.
		//
		CharacterAbilities = UNinjaGASFunctionLibrary::GetCustomAbilitySystemComponentFromActor(this);
		if (IsValid(CharacterAbilities))
		{
			SetupAbilitySystemComponent(this);
		}
	}
}

void ANinjaGASCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ANinjaGASCharacter::GetAbilitySystemComponent() const
{
	return CharacterAbilities;
}

UNinjaGASDataAsset* ANinjaGASCharacter::GetAbilityBundle() const
{
	return DefaultAbilitySetup;
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
