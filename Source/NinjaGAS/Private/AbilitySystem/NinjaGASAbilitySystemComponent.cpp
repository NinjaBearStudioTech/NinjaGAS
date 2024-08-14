// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

#include "NinjaGASLog.h"
#include "AbilitySystem/Interfaces/AbilitySystemDefaultsInterface.h"
#include "Data/NinjaGASDataAsset.h"
#include "Interfaces/BatchGameplayAbilityInterface.h"

UNinjaGASAbilitySystemComponent::UNinjaGASAbilitySystemComponent()
{
	static constexpr bool bIsReplicated = true;
	SetIsReplicatedByDefault(bIsReplicated);

	bShouldDoServerAbilityRPCBatch = true;
}

void UNinjaGASAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
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

void UNinjaGASAbilitySystemComponent::InitializeDefaults(AActor* NewAvatarActor)
{
	if (!IsValid(NewAvatarActor) || !IsOwnerActorAuthoritative())
	{
		return;
	}

	const IAbilitySystemDefaultsInterface* Defaults = Cast<IAbilitySystemDefaultsInterface>(NewAvatarActor);
	if (Defaults == nullptr || !Defaults->HasDefaultAbilitySettings())
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

	UE_LOG(LogAbilitySystemComponent, Log, TEXT("Initialized ASC defaults for %s: [ Atribute Sets: %d, Gameplay Effects: %d, Gameplay Abilities: %d ]."),
		*GetNameSafe(NewAvatarActor), AddedAttributes.Num(), DefaultEffectHandles.Num(), DefaultAbilityHandles.Num());
}

void UNinjaGASAbilitySystemComponent::InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets)
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

void UNinjaGASAbilitySystemComponent::InitializeGameplayEffects(const TArray<FDefaultGameplayEffect>& GameplayEffects)
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

void UNinjaGASAbilitySystemComponent::InitializeGameplayAbilities(const TArray<FDefaultGameplayAbility>& GameplayAbilities)
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

FActiveGameplayEffectHandle UNinjaGASAbilitySystemComponent::ApplyGameplayEffectClassToSelf(const TSubclassOf<UGameplayEffect> EffectClass, const float Level)
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
			
			UE_LOG(LogAbilitySystemComponent, Verbose, TEXT("[%s] Effect '%s' granted at level %f."),
				*GetNameSafe(GetAvatarActor()), *GetNameSafe(EffectClass), Level);
		}
	}

	return Handle;	
}

FGameplayAbilitySpecHandle UNinjaGASAbilitySystemComponent::GiveAbilityFromClass(const TSubclassOf<UGameplayAbility> AbilityClass, const int32 Level, const int32 Input)
{
	FGameplayAbilitySpecHandle Handle;

	if (IsValid(AbilityClass))
	{
		const FGameplayAbilitySpec NewAbilitySpec(FGameplayAbilitySpec(AbilityClass, Level, Input, GetOwner()));
		Handle = GiveAbility(NewAbilitySpec);

		UE_LOG(LogAbilitySystemComponent, Log, TEXT("[%s] Ability '%s' %s at level %d."),
			*GetNameSafe(GetAvatarActor()), *GetNameSafe(AbilityClass),
			Handle.IsValid() ? TEXT("sucessfully granted") : TEXT("failed to be granted"), Level);
	}

	return Handle;
}

bool UNinjaGASAbilitySystemComponent::TryBatchActivateAbility(const FGameplayAbilitySpecHandle AbilityHandle, const bool bEndAbilityImmediately)
{
	bool bAbilityActivated = false;
	if (AbilityHandle.IsValid())
	{
		GAS_LOG(Warning, "Ability handle is invalid!");
		return bAbilityActivated;
	}
	
	FScopedServerAbilityRPCBatcher Batch(this, AbilityHandle);
	bAbilityActivated = TryActivateAbility(AbilityHandle, true);

	if (!bEndAbilityImmediately)
	{
		const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilityHandle);
		if (AbilitySpec != nullptr)
		{
			UGameplayAbility* Ability = AbilitySpec->GetPrimaryInstance();
			if (IsValid(Ability) && Ability->Implements<UBatchGameplayAbilityInterface>())
			{
				IBatchGameplayAbilityInterface::Execute_EndAbilityFromBatch(Ability);
			}
			else
			{
				const FString Message = IsValid(Ability) ? FString::Printf(TEXT("%s does not implement Batch Gameplay Ability Interface"), *GetNameSafe(Ability)) : "is invalid"; 
				GAS_LOG_ARGS(Error, "Ability %s!", *Message);
			}
		}
	}

	return bAbilityActivated;
}

void UNinjaGASAbilitySystemComponent::GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutAttributeSets = DefaultAbilitySetup->DefaultAttributeSets;
	}
}

void UNinjaGASAbilitySystemComponent::GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultEffects = DefaultAbilitySetup->DefaultGameplayEffects;
	}
}

void UNinjaGASAbilitySystemComponent::GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const
{
	if (IsValid(DefaultAbilitySetup))
	{
		OutDefaultAbilities = DefaultAbilitySetup->DefaultGameplayAbilities;
	}
}

void UNinjaGASAbilitySystemComponent::ClearActorInfo()
{
	ClearDefaults();
	Super::ClearActorInfo();
}

bool UNinjaGASAbilitySystemComponent::ShouldDoServerAbilityRPCBatch() const
{
	return bShouldDoServerAbilityRPCBatch;
}

void UNinjaGASAbilitySystemComponent::ClearDefaults()
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

	UE_LOG(LogAbilitySystemComponent, Log, TEXT("[%s] Cleared Gameplay Elements: [ Atribute Sets: %d, Gameplay Effects: %d, Gameplay Abilities: %d ]."),
		*GetNameSafe(GetAvatarActor()), AttributeSetCount, EffectHandleCount, AbilityHandleCount);
}
