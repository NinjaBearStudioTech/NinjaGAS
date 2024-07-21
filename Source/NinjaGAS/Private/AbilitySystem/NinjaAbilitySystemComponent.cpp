// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaAbilitySystemComponent.h"

#include "AbilitySystem/Interfaces/AbilitySystemDefaultsInterface.h"
#include "Data/NinjaAbilitiesDataAsset.h"

DEFINE_LOG_CATEGORY(LogNinjaFrameworkAbilitySystemComponent);

UNinjaAbilitySystemComponent::UNinjaAbilitySystemComponent()
{
	static constexpr bool bIsReplicated = true;
	SetIsReplicatedByDefault(bIsReplicated);

	ReplicationMode = ENinjaGameplayEffectReplicationMode::Mixed;
	SetGameplayReplicationMode(ReplicationMode);
}

void UNinjaAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// Guard condition to ensure we should clear/init for this new Avatar Actor.
	const bool bAvatarHasChanged = AbilityActorInfo && AbilityActorInfo->AvatarActor != InAvatarActor && InAvatarActor != nullptr;
	
	// Remove any defaults if we are changing an avatar from a previous valid one.
	if (bAvatarHasChanged)
	{
		ClearDefaults();
	}

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	// Apply the new defaults obtained from the owner's interface.
	if (bAvatarHasChanged)
	{
		InitializeDefaults(InAvatarActor);
		OnAbilitySystemAvatarChanged.Broadcast(InAvatarActor);
	}
}

void UNinjaAbilitySystemComponent::InitializeDefaults(AActor* NewAvatarActor)
{
	if (!IsValid(NewAvatarActor) || !IsOwnerActorAuthoritative())
	{
		return;
	}

	const IAbilitySystemDefaultsInterface* Defaults = Cast<IAbilitySystemDefaultsInterface>(NewAvatarActor);
	if (Defaults == nullptr)
	{
		Defaults = Cast<IAbilitySystemDefaultsInterface>(this);
	}
	
	TArray<FDefaultAttributeSet> AttributeSets;
	Defaults->GetDefaultAttributeSets(AttributeSets);
	InitializeAttributeSets(AttributeSets);

	TArray<FDefaultGameplayEffect> DefaultEffects;
	Defaults->GetDefaultGameplayEffects(DefaultEffects);
	InitializeGameplayEffects(DefaultEffects);

	TArray<FDefaultGameplayAbility> DefaultAbilities;
	Defaults->GetDefaultGameplayAbilities(DefaultAbilities);
	InitializeGameplayAbilities(DefaultAbilities);

	UE_LOG(LogNinjaFrameworkAbilitySystemComponent, Log, TEXT("Initialized ASC defaults for %s: [ Atribute Sets: %d, Gameplay Effects: %d, Gameplay Abilities: %d ]."),
		*GetNameSafe(NewAvatarActor), AddedAttributes.Num(), DefaultEffectHandles.Num(), DefaultAbilityHandles.Num());
}

void UNinjaAbilitySystemComponent::InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets)
{
	for (const auto& Entry : AttributeSets)
	{
		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), Entry.AttributeSet);
		if (GetSpawnedAttributes().Contains(NewAttributeSet) == false)
		{
			if (IsValid(Entry.AttributeTable))
			{
				NewAttributeSet->InitFromMetaDataTable(Entry.AttributeTable);	
			}

			AddAttributeSetSubobject(NewAttributeSet);
			AddedAttributes.Add(NewAttributeSet);
		}
	}		
}

void UNinjaAbilitySystemComponent::InitializeGameplayEffects(const TArray<FDefaultGameplayEffect>& GameplayEffects)
{
	const int32 GameplayEffectCount = GameplayEffects.Num(); 
	if (GameplayEffectCount > 0)
	{
		const int32 NewSize = DefaultEffectHandles.Num() + GameplayEffectCount;  
		DefaultEffectHandles.Reserve(NewSize);
		
		for (const auto& Entry : GameplayEffects)
		{
			FActiveGameplayEffectHandle Handle = ApplyGameplayEffectClassToSelf(Entry.GameplayEffect, Entry.Level);
			DefaultEffectHandles.Add(Handle);
		}
	}
}

void UNinjaAbilitySystemComponent::InitializeGameplayAbilities(const TArray<FDefaultGameplayAbility>& GameplayAbilities)
{
	const int32 GameplayAbilityCount = GameplayAbilities.Num(); 
	if (GameplayAbilityCount > 0)
	{
		const int32 NewSize = DefaultAbilityHandles.Num() + GameplayAbilityCount; 
		DefaultAbilityHandles.Reserve(NewSize);
		
		for (const auto& Entry : GameplayAbilities)
		{
			FGameplayAbilitySpecHandle Handle = GiveAbilityFromClass(Entry.GameplayAbility, Entry.Level, Entry.Input);
			DefaultAbilityHandles.Add(Handle);
		}
	}
}

FActiveGameplayEffectHandle UNinjaAbilitySystemComponent::ApplyGameplayEffectClassToSelf(const TSubclassOf<UGameplayEffect> EffectClass, const float Level)
{
	FActiveGameplayEffectHandle Handle;
    
	if (IsValid(EffectClass))
	{
		FGameplayEffectContextHandle ContextHandle = MakeEffectContext();
		ContextHandle.AddSourceObject(GetOwner());

		const FGameplayEffectSpecHandle SpecHandle = MakeOutgoingSpec(EffectClass, Level, ContextHandle);
		if (SpecHandle.IsValid())
		{
			Handle = ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
			UE_LOG(LogNinjaFrameworkAbilitySystemComponent, Verbose, TEXT("[%s] Effect '%s' granted at level %f."),
				*GetNameSafe(GetAvatarActor()), *GetNameSafe(EffectClass), Level);
		}
	}

	return Handle;	
}

FGameplayAbilitySpecHandle UNinjaAbilitySystemComponent::GiveAbilityFromClass(const TSubclassOf<UGameplayAbility> AbilityClass, int32 Level, int32 Input)
{
	FGameplayAbilitySpecHandle Handle;

	if (IsValid(AbilityClass))
	{
		const FGameplayAbilitySpec NewAbilitySpec(FGameplayAbilitySpec(AbilityClass, Level, Input, GetOwner()));
		Handle = GiveAbility(NewAbilitySpec);

		UE_LOG(LogNinjaFrameworkAbilitySystemComponent, Log, TEXT("[%s] Ability '%s' %s at level %d."),
			*GetNameSafe(GetAvatarActor()), *GetNameSafe(AbilityClass),
			Handle.IsValid() ? TEXT("sucessfully granted") : TEXT("failed to be granted"), Level);
	}

	return Handle;
}

void UNinjaAbilitySystemComponent::SetGameplayReplicationMode(const ENinjaGameplayEffectReplicationMode NewReplicationMode)
{
	switch(NewReplicationMode)
	{
		case ENinjaGameplayEffectReplicationMode::Minimal:
			SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
			break;
		case ENinjaGameplayEffectReplicationMode::Mixed:
			SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
			break;
		case ENinjaGameplayEffectReplicationMode::Full:
			SetReplicationMode(EGameplayEffectReplicationMode::Full);
			break;
	}
}

void UNinjaAbilitySystemComponent::GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutAttributeSets = DefaultAbilitySetup->DefaultAttributeSets;
	}
}

void UNinjaAbilitySystemComponent::GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultEffects = DefaultAbilitySetup->DefaultGameplayEffects;
	}
}

void UNinjaAbilitySystemComponent::GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultAbilities = DefaultAbilitySetup->DefaultGameplayAbilities;
	}
}

void UNinjaAbilitySystemComponent::ClearActorInfo()
{
	ClearDefaults();
	Super::ClearActorInfo();
}

void UNinjaAbilitySystemComponent::ClearDefaults()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}
	
	int32 AbilityHandleCount = 0;
	for (auto It(DefaultAbilityHandles.CreateIterator()); It; ++It)
	{
		SetRemoveAbilityOnEnd(*It);
		It.RemoveCurrent();
		++AbilityHandleCount;
	}

	int32 EffectHandleCount = 0;
	for (auto It(DefaultEffectHandles.CreateIterator()); It; ++It)
	{
		RemoveActiveGameplayEffect(*It);
		It.RemoveCurrent();
		++EffectHandleCount;
	}

	int32 AttributeSetCount = 0;
	for (auto It(AddedAttributes.CreateIterator()); It; ++It)
	{
		RemoveSpawnedAttribute(*It);
		It.RemoveCurrent();
		++AttributeSetCount;
	}

	UE_LOG(LogNinjaFrameworkAbilitySystemComponent, Log, TEXT("[%s] Cleared Gameplay Elements: [ Atribute Sets: %d, Gameplay Effects: %d, Gameplay Abilities: %d ]."),
		*GetNameSafe(GetAvatarActor()), AttributeSetCount, EffectHandleCount, AbilityHandleCount);
}
