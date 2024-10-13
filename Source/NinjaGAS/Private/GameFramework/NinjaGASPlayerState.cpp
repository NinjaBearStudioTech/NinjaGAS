// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "GameFramework/NinjaGASPlayerState.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"

FName ANinjaGASPlayerState::AbilityComponentName = TEXT("AbilitySystem");

ANinjaGASPlayerState::ANinjaGASPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	MinNetUpdateFrequency = 33.f;
	NetUpdateFrequency = 66.f;
	NetPriority = 3.f;
	AbilityReplicationMode = EGameplayEffectReplicationMode::Mixed;

	// Subtypes of the Ability System can be created using the Object Initializer.
	// For different parents, they can also cancel this optional sub-object and create a different one.
	//
	// For Blueprints, replacing with a different child class is still viable, but cancelling the
	// creation and creating an ASC using a different hierarchy is not supported.
	//
	AbilitySystemComponent = CreateOptionalDefaultSubobject<UNinjaGASAbilitySystemComponent>(AbilityComponentName);
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(AbilityReplicationMode);	
	}
}

void ANinjaGASPlayerState::PostInitProperties()
{
	Super::PostInitProperties();

	if (IsValid(AbilitySystemComponent))
	{
		// Set the Replication Mode after properties are initialized but before components.
		// This way, once the Ability System Component initializes, it has the correct value.
		//
		AbilitySystemComponent->SetReplicationMode(AbilityReplicationMode);
	}
}

void ANinjaGASPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
}

void ANinjaGASPlayerState::BeginPlay()
{
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
}

void ANinjaGASPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
	Super::EndPlay(EndPlayReason);
}

void ANinjaGASPlayerState::CopyProperties(APlayerState* TargetPlayerState)
{
	Super::CopyProperties(TargetPlayerState);
	DispatchCopyToPlayerStateComponents(TargetPlayerState);
}

void ANinjaGASPlayerState::Reset()
{
	Super::Reset();
	DispatchResetPlayerStateComponents();
}

void ANinjaGASPlayerState::DispatchCopyToPlayerStateComponents(APlayerState* TargetPlayerState)
{
	check(IsValid(TargetPlayerState));
	
	ForEachPlayerStateComponent([this, TargetPlayerState](UPlayerStateComponent* Component)
	{
		UPlayerStateComponent* TargetComponent = GetPlayerStateComponent(TargetPlayerState, Component);
		if (IsValid(TargetComponent))
		{
			Component->CopyProperties(TargetComponent);
		}
	});
}

void ANinjaGASPlayerState::DispatchResetPlayerStateComponents()
{
	ForEachPlayerStateComponent([](UPlayerStateComponent* Component)
	{
		Component->Reset();
	});
}

UPlayerStateComponent* ANinjaGASPlayerState::GetPlayerStateComponent(APlayerState* TargetPlayerState, const UPlayerStateComponent* Reference) const
{
	check(IsValid(TargetPlayerState));

	UClass* SourceClass = Reference->GetClass();
	const FString SourceName = Reference->GetName();

	UObject* TargetObject = StaticFindObject(SourceClass, TargetPlayerState, *SourceName);
	if (IsValid(TargetObject) && TargetObject->IsA<UPlayerStateComponent>())
	{
		return Cast<UPlayerStateComponent>(TargetObject);
	}

	return nullptr;
}

void ANinjaGASPlayerState::ForEachPlayerStateComponent(const TFunctionRef<void(UPlayerStateComponent*)>& Function) const
{
	TInlineComponentArray<UPlayerStateComponent*> Components;
	GetComponents(Components);

	for (UPlayerStateComponent* Component : Components)
	{
		Function(Component);
	}
}

UAbilitySystemComponent* ANinjaGASPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
