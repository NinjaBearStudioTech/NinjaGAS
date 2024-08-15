// Ninja Bear Studio Inc., all rights reserved.
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

void ANinjaGASPlayerState::DispatchCopyToPlayerStateComponents(APlayerState* TargetPlayerState)
{
	check(IsValid(TargetPlayerState));
	
	TInlineComponentArray<UPlayerStateComponent*> SourceStateComponents;
	GetComponents(SourceStateComponents);

	for (UPlayerStateComponent* SourceComponent : SourceStateComponents)
	{
		UClass* SourceClass = SourceComponent->GetClass();
		FString SourceName = SourceComponent->GetName();
		UObject* TargetObject = StaticFindObject(SourceClass, TargetPlayerState, *SourceName);

		if (IsValid(TargetObject) && TargetObject->IsA<UPlayerStateComponent>())
		{
			UPlayerStateComponent* TargetComponent = Cast<UPlayerStateComponent>(TargetObject);
			SourceComponent->CopyProperties(TargetComponent);
		}
	}
}

void ANinjaGASPlayerState::Reset()
{
	Super::Reset();
	DispatchResetPlayerStateComponents();
}

void ANinjaGASPlayerState::DispatchResetPlayerStateComponents()
{
	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);

	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
}

UAbilitySystemComponent* ANinjaGASPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}