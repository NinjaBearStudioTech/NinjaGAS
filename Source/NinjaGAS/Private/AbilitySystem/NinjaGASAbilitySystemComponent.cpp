// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "NinjaGASLog.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Data/NinjaGASDataAsset.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Interfaces/BatchGameplayAbilityInterface.h"

UNinjaGASAbilitySystemComponent::UNinjaGASAbilitySystemComponent()
{
	static constexpr bool bIsReplicated = true;
	SetIsReplicatedByDefault(bIsReplicated);

	bEnableAbilityBatchRPC = true;
	CurrentAbilitySetup = nullptr;
}

void UNinjaGASAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// Guard condition to ensure we should clear/init for this new Avatar Actor.
	const bool bAvatarHasChanged = AbilityActorInfo && AbilityActorInfo->AvatarActor != InAvatarActor && InAvatarActor != nullptr;
	
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
	if (Defaults == nullptr || !Defaults->HasAbilityData())
	{
		// Use the defaults provided by this own class.
		Defaults = Cast<IAbilitySystemDefaultsInterface>(this);
	}

	const UNinjaGASDataAsset* AbilityData = Defaults->GetAbilityData();
	if (IsValid(AbilityData) && AbilityData != CurrentAbilitySetup)
	{
		ClearDefaults();
		CurrentAbilitySetup = AbilityData;
		InitializeFromData(NewAvatarActor, AbilityData);
	}
}

void UNinjaGASAbilitySystemComponent::InitializeFromData(const AActor* NewAvatarActor, const UNinjaGASDataAsset* AbilityData)
{
	if (!IsValid(AbilityData) || !IsOwnerActorAuthoritative())
	{
		return;
	}

	const AActor* CurrentAvatar = GetAvatarActor();
	if (NewAvatarActor != CurrentAvatar)
	{
		return;
	}
	
	const TArray<FDefaultAttributeSet>& AttributeSets = AbilityData->DefaultAttributeSets;
	InitializeAttributeSets(AttributeSets);

	const TArray<FDefaultGameplayEffect>& GameplayEffects = AbilityData->DefaultGameplayEffects;
	InitializeGameplayEffects(GameplayEffects);

	const TArray<FDefaultGameplayAbility>& GameplayAbilities = AbilityData->DefaultGameplayAbilities;
	InitializeGameplayAbilities(GameplayAbilities);

	const FGameplayTagContainer& InitialGameplayTags = AbilityData->InitialGameplayTags; 
	if (InitialGameplayTags.IsValid())
	{
		AddReplicatedLooseGameplayTags(InitialGameplayTags);
	}
	
	UE_LOG(LogAbilitySystemComponent, Log, TEXT("Initialized ASC defaults from %s: [ Attribute Sets: %d, Effects: %d, Abilities: %d, Tags %d ]."),
		*GetNameSafe(AbilityData), AddedAttributes.Num(), DefaultEffectHandles.Num(), DefaultAbilityHandles.Num(), InitialGameplayTags.Num());	
}

void UNinjaGASAbilitySystemComponent::InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets)
{
	for (const FDefaultAttributeSet& Entry : AttributeSets)
	{
		const TSubclassOf<UAttributeSet> AttributeSetClass = Entry.AttributeSetClass;
		const UDataTable* AttributeTable = Entry.AttributeTable;
		
		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
		if (GetSpawnedAttributes().Contains(NewAttributeSet))
		{
			UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Discarding Attribute Set %s since it was already spawned."), *GetNameSafe(NewAttributeSet));
		}
		else
		{
			if (IsValid(AttributeTable))
			{
				NewAttributeSet->InitFromMetaDataTable(AttributeTable);
				UE_LOG(LogAbilitySystemComponent, Verbose, TEXT("Initialized Attribute Set %s with %s."), *GetNameSafe(NewAttributeSet), *GetNameSafe(AttributeTable));
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
			const TSubclassOf<UGameplayEffect> GameplayEffectClass = Entry.GameplayEffectClass;
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
			const TSubclassOf<UGameplayAbility> GameplayAbilityClass = Entry.GameplayAbilityClass;
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
			Handle.IsValid() ? TEXT("successfully granted") : TEXT("failed to be granted"), Level);
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

void UNinjaGASAbilitySystemComponent::CancelAbilitiesByTags(const FGameplayTagContainer AbilityTags, const FGameplayTagContainer CancelFilterTags)
{
	CancelAbilities(&AbilityTags, &CancelFilterTags);
}

void UNinjaGASAbilitySystemComponent::ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UNinjaGASAbilitySystemComponent::AddGameplayCueLocally(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	CueManager->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UNinjaGASAbilitySystemComponent::RemoveGameplayCueLocally(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(GetOwner(), GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

void UNinjaGASAbilitySystemComponent::ClearActorInfo()
{
	ClearDefaults();
	Super::ClearActorInfo();
}

bool UNinjaGASAbilitySystemComponent::ShouldDoServerAbilityRPCBatch() const
{
	return bEnableAbilityBatchRPC;
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
	if (GEnableDefaultPlayMontage)
	{
		// Always useful to still allow the default flow, if there are some meaningful changes in the core system
		// that were not yet reflect in this custom implementation. Can be enabled with CVar "GEnableDefaultPlayMontage".
		//
		return Super::PlayMontage(AnimatingAbility, ActivationInfo, Montage, InPlayRate, StartSectionName, StartTimeSeconds);
	}
	
	float Duration = -1.f;

	// This method was re-written just to ensure that the Animation Instance is retrieved from the Actor Info
	// by default, but also, other scenarios can be supported. Biggest example being an IK Runtime Retarget.
	//
	// This virtual "GetAnimInstanceFromActorInfo" provides some flexibility on how the Anim Instance is
	// retrieved. It can be extended in projects that should support IK Runtime Retargets and also traditional
	// Anim Instances set in the Actor Info. 
	//
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
				SetReplicatedMontageInfo(MutableRepAnimMontageInfo, Montage, StartSectionName);

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

void UNinjaGASAbilitySystemComponent::SetReplicatedMontageInfo(FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo, UAnimMontage* NewMontageToPlay, const FName& StartSectionName)
{
	const uint8 PlayInstanceId = MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0;
	const uint8 SectionIdToPlay = NewMontageToPlay->GetSectionIndex(StartSectionName) + 1;
	
#if ENGINE_MINOR_VERSION == 3
	
	MutableRepAnimMontageInfo.AnimMontage = NewMontageToPlay;
	MutableRepAnimMontageInfo.PlayInstanceId = PlayInstanceId;

	MutableRepAnimMontageInfo.SectionIdToPlay = 0;
	if (MutableRepAnimMontageInfo.AnimMontage && StartSectionName != NAME_None)
	{
		MutableRepAnimMontageInfo.SectionIdToPlay = SectionIdToPlay;
	}
	
#elif ENGINE_MINOR_VERSION > 3

	UAnimSequenceBase* Animation = NewMontageToPlay;
	if (NewMontageToPlay->IsDynamicMontage())
	{
		Animation = NewMontageToPlay->GetFirstAnimReference();

		check(!NewMontageToPlay->SlotAnimTracks.IsEmpty());
		MutableRepAnimMontageInfo.SlotName = NewMontageToPlay->SlotAnimTracks[0].SlotName;
		MutableRepAnimMontageInfo.BlendOutTime = NewMontageToPlay->GetDefaultBlendInTime();		
	}
	
	MutableRepAnimMontageInfo.Animation = Animation;
	MutableRepAnimMontageInfo.PlayInstanceId = PlayInstanceId;

	MutableRepAnimMontageInfo.SectionIdToPlay = 0;
	if (MutableRepAnimMontageInfo.Animation && StartSectionName != NAME_None)
	{
		MutableRepAnimMontageInfo.SectionIdToPlay = SectionIdToPlay;
	}
	
#endif
}

const UNinjaGASDataAsset* UNinjaGASAbilitySystemComponent::GetAbilityData() const
{
	return DefaultAbilitySetup;
}

void UNinjaGASAbilitySystemComponent::ClearDefaults()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	FGameplayTagContainer InitialGameplayTags = FGameplayTagContainer::EmptyContainer;  
	if (IsValid(CurrentAbilitySetup))
	{
		InitialGameplayTags = CurrentAbilitySetup->InitialGameplayTags;
		if (InitialGameplayTags.IsValid())
		{
			RemoveReplicatedLooseGameplayTags(InitialGameplayTags);	
		}	
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

	CurrentAbilitySetup = nullptr;
	
	UE_LOG(LogAbilitySystemComponent, Log, TEXT("[%s] Cleared Gameplay Elements: [ Attribute Sets: %d, Effects: %d, Abilities: %d, Tags: %d ]."),
		*GetNameSafe(GetAvatarActor()), AttributeSetCount, EffectHandleCount, AbilityHandleCount, InitialGameplayTags.Num());
}
