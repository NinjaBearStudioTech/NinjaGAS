// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "GameFramework/NinjaGASActor.h"

#include "NinjaGASFunctionLibrary.h"
#include "NinjaGASLog.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Launch/Resources/Version.h"

FName ANinjaGASActor::AbilitySystemComponentName = TEXT("AbilitySystemComponent");

ANinjaGASActor::ANinjaGASActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;

#if ENGINE_MINOR_VERSION < 5
	MinNetUpdateFrequency = 11.f;
	NetUpdateFrequency = 33.f;
#else
	SetMinNetUpdateFrequency(11.f);
	SetNetUpdateFrequency(33.f);
#endif

	AbilitySystemInitializationMode = ELazyAbilitySystemInitializationMode::Lazy;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Minimal;
	AbilitySystemComponentClass = UNinjaGASAbilitySystemComponent::StaticClass();
}

void ANinjaGASActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);
	if (!ReplicatedActorAbilities && ActorAbilities)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedActorAbilities, this);
		ReplicatedActorAbilities = ActorAbilities;
		GAS_LOG(Log, "Synchronized Ability System for replication.");
	}	
}

void ANinjaGASActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedActorAbilities, Params);
}

void ANinjaGASActor::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsValid(ActorAbilities))
	{
		// Reinforce the Replication Mode in the ASC.
		ActorAbilities->SetReplicationMode(AbilityReplicationMode);

		const UEnum* AttitudeEnum = StaticEnum<EGameplayEffectReplicationMode>();
		GAS_LOG_ARGS(Verbose, "Reinforced ASC replication mode to %s.", *AttitudeEnum->GetValueAsName(AbilityReplicationMode).ToString());
	}		
}

void ANinjaGASActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);

	if (AbilitySystemInitializationMode == ELazyAbilitySystemInitializationMode::Eager && GetNetMode() != NM_Client)
	{
		check(!ActorAbilities);
		GAS_LOG_ARGS(Verbose, "Eagerly initializing the ASC for '%s'.", *GetNameSafe(this));
		InitializeAbilitySystemComponent();
		ForceNetUpdate();
	}
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
		GAS_LOG_ARGS(Verbose, "Actor '%s' received a valid ASC (probably from a Game Feature?).", *GetNameSafe(this));
		ActorAbilities->InitAbilityActorInfo(this, this);
		ForceNetUpdate();
	}
}

void ANinjaGASActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* ANinjaGASActor::GetAbilitySystemComponent() const
{
	if (!ActorAbilities && HasAuthority() && AbilitySystemInitializationMode == ELazyAbilitySystemInitializationMode::Lazy && GetWorld() && !IsUnreachable())
	{
		ANinjaGASActor* MutableActor = const_cast<ANinjaGASActor*>(this);
		MutableActor->InitializeAbilitySystemComponent();
		MutableActor->ForceNetUpdate();
	}
	
	return ActorAbilities;
}

UNinjaGASDataAsset* ANinjaGASActor::GetAbilityData() const
{
	return DefaultAbilitySetup;
}

ELazyAbilitySystemInitializationMode ANinjaGASActor::GetAbilitySystemInitializationMode() const
{
	return AbilitySystemInitializationMode;
}

void ANinjaGASActor::SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue)
{
	PendingAttributeReplications.Emplace(FPendingAttributeReplication(Attribute, NewValue));
	GAS_LOG_ARGS(Verbose, "Added pending attribute '%s' with base/current value: %f/%f.", *Attribute.GetName(), NewValue.GetBaseValue(), NewValue.GetCurrentValue());
}

void ANinjaGASActor::InitializeAbilitySystemComponent()
{
	ActorAbilities = NewObject<UNinjaGASAbilitySystemComponent>(this, AbilitySystemComponentClass);
	ActorAbilities->SetIsReplicated(true);
	ActorAbilities->SetReplicationMode(AbilityReplicationMode);
	ActorAbilities->RegisterComponent();
	ActorAbilities->InitAbilityActorInfo(this, this);
	GAS_LOG_ARGS(Log, "Initialized Ability System Component for actor '%s'.", *GetNameSafe(this));
}

void ANinjaGASActor::ApplyPendingAttributesFromReplication()
{
	checkf(ActorAbilities, TEXT("Attempted to apply pending attributes without an ASC!"));
	if (PendingAttributeReplications.Num() > 0)
	{
		for (const FPendingAttributeReplication& Pending : PendingAttributeReplications) 
		{
			ActorAbilities->DeferredSetBaseAttributeValueFromReplication(Pending.Attribute, Pending.NewValue);
		}
		
		PendingAttributeReplications.Empty();
	}
}

void ANinjaGASActor::OnRep_ReplicatedActorAbilities()
{
	ActorAbilities = ReplicatedActorAbilities;
	if (ActorAbilities)
	{
		InitializeAbilitySystemComponent();
		ApplyPendingAttributesFromReplication();
	}
}
