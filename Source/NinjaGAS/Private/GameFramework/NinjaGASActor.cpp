// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "GameFramework/NinjaGASActor.h"

#include "NinjaGASFunctionLibrary.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"

FName ANinjaGASActor::AbilitySystemComponentName = TEXT("AbilitySystemComponent");

ANinjaGASActor::ANinjaGASActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	MinNetUpdateFrequency = 11.f;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Minimal;

	ActorAbilities = CreateOptionalDefaultSubobject<UNinjaGASAbilitySystemComponent>(AbilitySystemComponentName);
	if (IsValid(ActorAbilities))
	{
		ActorAbilities->SetIsReplicated(bReplicates);
		ActorAbilities->SetReplicationMode(AbilityReplicationMode);	
	}	
}

void ANinjaGASActor::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsValid(ActorAbilities))
	{
		// Reinforce the Replication Mode in the ASC.
		ActorAbilities->SetReplicationMode(AbilityReplicationMode);
	}		
}

void ANinjaGASActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ANinjaGASActor::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();

	// This reinforces the ASC in this class, in case it was provided by a Game Feature.
	//
	// Doing this is useful as it makes this class compatible with both a gameplay feature and the
	// ASC interface, avoiding the component lookup.
	//
	ActorAbilities = UNinjaGASFunctionLibrary::GetCustomAbilitySystemComponentFromActor(this);
	if (IsValid(ActorAbilities))
	{
		ActorAbilities->InitAbilityActorInfo(this, this);
	}
}

void ANinjaGASActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ANinjaGASActor::GetAbilitySystemComponent() const
{
	return ActorAbilities;
}
