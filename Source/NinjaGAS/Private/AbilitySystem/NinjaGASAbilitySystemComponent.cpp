// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

#include "NinjaGASLog.h"
#include "Data/NinjaGASDataAsset.h"
#include "Engine/AssetManager.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
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

void UNinjaGASAbilitySystemComponent::InitializeDefaults(const AActor* NewAvatarActor)
{
	if (!IsValid(NewAvatarActor) || !IsOwnerActorAuthoritative())
	{
		return;
	}

	const IAbilitySystemDefaultsInterface* Defaults = Cast<IAbilitySystemDefaultsInterface>(NewAvatarActor);
	if (Defaults == nullptr || !Defaults->HasAbilityBundle())
	{
		// Use the defaults provided by this class.
		Defaults = Cast<IAbilitySystemDefaultsInterface>(this);
	}

	const UNinjaGASDataAsset* AbilityBundle = Defaults->GetAbilityBundle();
	if (IsValid(AbilityBundle))
	{
		UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		check(AssetManager);

		const TArray<FName> Bundles = { "Abilities" };
		const FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ThisClass::InitializeFromBundle, NewAvatarActor, AbilityBundle);
		AbilityBundleHandle = AssetManager->LoadPrimaryAsset(AbilityBundle->GetPrimaryAssetId(), Bundles, Delegate, FStreamableManager::AsyncLoadHighPriority);
	}
}

void UNinjaGASAbilitySystemComponent::InitializeFromBundle(const AActor* NewAvatarActor, const UNinjaGASDataAsset* AbilityBundle)
{
	if (!IsValid(AbilityBundle) || !IsOwnerActorAuthoritative())
	{
		return;
	}

	const AActor* CurrentAvatar = GetAvatarActor();
	if (NewAvatarActor != CurrentAvatar)
	{
		return;
	}
	
	const TArray<FDefaultAttributeSet>& AttributeSets = AbilityBundle->DefaultAttributeSets;
	InitializeAttributeSets(AttributeSets);

	const TArray<FDefaultGameplayEffect>& GameplayEffects = AbilityBundle->DefaultGameplayEffects;
	InitializeGameplayEffects(GameplayEffects);

	const TArray<FDefaultGameplayAbility>& GameplayAbilities = AbilityBundle->DefaultGameplayAbilities;
	InitializeGameplayAbilities(GameplayAbilities);

	UE_LOG(LogAbilitySystemComponent, Log, TEXT("Initialized ASC defaults from %s: [ Atribute Sets: %d, Gameplay Effects: %d, Gameplay Abilities: %d ]."),
		*GetNameSafe(AbilityBundle), AddedAttributes.Num(), DefaultEffectHandles.Num(), DefaultAbilityHandles.Num());	
}

void UNinjaGASAbilitySystemComponent::InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets)
{
	for (const FDefaultAttributeSet& Entry : AttributeSets)
	{
		const TSubclassOf<UAttributeSet> AttributeSetClass = Entry.AttributeSetClass.Get();
		const UDataTable* AttributeTable = Entry.AttributeTable.Get();
		
		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
		if (GetSpawnedAttributes().Contains(NewAttributeSet) == false)
		{
			if (IsValid(AttributeTable))
			{
				NewAttributeSet->InitFromMetaDataTable(AttributeTable);	
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
		
		for (const FDefaultGameplayEffect& Entry : GameplayEffects)
		{
			const TSubclassOf<UGameplayEffect> GameplayEffectClass = Entry.GameplayEffectClass.Get();
			FActiveGameplayEffectHandle Handle = ApplyGameplayEffectClassToSelf(GameplayEffectClass, Entry.Level);
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
		
		for (const FDefaultGameplayAbility& Entry : GameplayAbilities)
		{
			const TSubclassOf<UGameplayAbility> GameplayAbilityClass = Entry.GameplayAbilityClass.Get();
			FGameplayAbilitySpecHandle Handle = GiveAbilityFromClass(GameplayAbilityClass, Entry.Level, Entry.Input);
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


void UNinjaGASAbilitySystemComponent::ClearActorInfo()
{
	ClearDefaults();
	Super::ClearActorInfo();
}

bool UNinjaGASAbilitySystemComponent::ShouldDoServerAbilityRPCBatch() const
{
	return bShouldDoServerAbilityRPCBatch;
}

UAnimInstance* UNinjaGASAbilitySystemComponent::GetAnimInstanceFromActorInfo() const
{
	if (!AbilityActorInfo.IsValid())
	{
		return nullptr;
	}
	
	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (ActorInfo->AnimInstance.IsValid() && ActorInfo->AnimInstance->IsValidLowLevelFast())
	{
		// Return the one that was deliberately set in the Actor Info.
		return ActorInfo->AnimInstance.Get();
	}

	// Otherwise, let the getter method try to figure out the animation instance.
	return ActorInfo->GetAnimInstance();
}

float UNinjaGASAbilitySystemComponent::PlayMontage(UGameplayAbility* AnimatingAbility,
	const FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, const float InPlayRate, const FName StartSectionName,
	const float StartTimeSeconds)
{
	float Duration = -1.f;

	UAnimInstance* AnimInstance = GetAnimInstanceFromActorInfo();
	if (AnimInstance && Montage)
	{
		Duration = AnimInstance->Montage_Play(Montage, InPlayRate, EMontagePlayReturnType::MontageLength, StartTimeSeconds);
		if (Duration > 0.f)
		{
			if (Montage->HasRootMotion() && AnimInstance->GetOwningActor())
			{
				UE_LOG(LogRootMotion, Log, TEXT("UAbilitySystemComponent::PlayMontage %s, Role: %s")
					, *GetNameSafe(Montage)
					, *UEnum::GetValueAsString(TEXT("Engine.ENetRole"), AnimInstance->GetOwningActor()->GetLocalRole())
					);
			}

			LocalAnimMontageInfo.AnimMontage = Montage;
			LocalAnimMontageInfo.AnimatingAbility = AnimatingAbility;
			LocalAnimMontageInfo.PlayInstanceId = (LocalAnimMontageInfo.PlayInstanceId < UINT8_MAX ? LocalAnimMontageInfo.PlayInstanceId + 1 : 0);
			
			if (AnimatingAbility)
			{
				AnimatingAbility->SetCurrentMontage(Montage);
			}
			
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, Montage);
			}

			// Replicate for non-owners and for replay recordings
			// The data we set from GetRepAnimMontageInfo_Mutable() is used both by the server to replicate to clients and by clients to record replays.
			// We need to set this data for recording clients because there exists network configurations where an abilities montage data will not replicate to some clients (for example: if the client is an autonomous proxy.)
			if (ShouldRecordMontageReplication())
			{
				FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo = GetRepAnimMontageInfo_Mutable();

				// Those are static parameters, they are only set when the montage is played. They are not changed after that.
				MutableRepAnimMontageInfo.AnimMontage = Montage;
				MutableRepAnimMontageInfo.PlayInstanceId = (MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0);

				MutableRepAnimMontageInfo.SectionIdToPlay = 0;
				if (MutableRepAnimMontageInfo.AnimMontage && StartSectionName != NAME_None)
				{
					// we add one so INDEX_NONE can be used in the on rep
					MutableRepAnimMontageInfo.SectionIdToPlay = MutableRepAnimMontageInfo.AnimMontage->GetSectionIndex(StartSectionName) + 1;
				}

				// Update parameters that change during Montage lifetime.
				AnimMontage_UpdateReplicatedData();
			}

			// Replicate to non-owners
			if (IsOwnerActorAuthoritative())
			{
				// Force net update on our avatar actor.
				if (AbilityActorInfo->AvatarActor != nullptr)
				{
					AbilityActorInfo->AvatarActor->ForceNetUpdate();
				}
			}
			else
			{
				// If this prediction key is rejected, we need to end the preview
				FPredictionKey PredictionKey = GetPredictionKeyForNewAction();
				if (PredictionKey.IsValidKey())
				{
					PredictionKey.NewRejectedDelegate().BindUObject(this, &ThisClass::OnPredictiveMontageRejected, Montage);
				}
			}
		}
	}

	return Duration;
}

UNinjaGASDataAsset* UNinjaGASAbilitySystemComponent::GetAbilityBundle() const
{
	return DefaultAbilitySetup;
}

void UNinjaGASAbilitySystemComponent::ClearDefaults()
{
	AbilityBundleHandle.Reset();
	
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
