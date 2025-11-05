// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "NinjaGASLog.h"
#include "NinjaGASTags.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Data/NinjaGASDataAsset.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Interfaces/BatchGameplayAbilityInterface.h"
#include "Types/FNinjaAbilityDefaultHandles.h"

/**
 * CVAR to control the "Play Montage" flow.
 * Example: ninjagas.EnableDefaultPlayMontage true
 */
static bool GEnableDefaultPlayMontage = false;
static FAutoConsoleVariableRef CVarEnableDefaultPlayMontage(
	TEXT("ninjagas.EnableDefaultPlayMontage"),
	GEnableDefaultPlayMontage,
	TEXT("Enables or disables the PlayMontage default behavior."),
	ECVF_Default
);

UNinjaGASAbilitySystemComponent::UNinjaGASAbilitySystemComponent()
{
	static constexpr bool bIsReplicated = true;
	SetIsReplicatedByDefault(bIsReplicated);

	bEnableAbilityBatchRPC = true;
	bResetStateWhenAvatarChanges = true;
}

void UNinjaGASAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// Guard condition to ensure we should clear/init for a new Avatar Actor.
	const bool bAvatarHasChanged = AbilityActorInfo && AbilityActorInfo->AvatarActor != InAvatarActor && InAvatarActor != nullptr;
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	InitializeDefaultsFromOwner(InOwnerActor);

	if (bAvatarHasChanged)
	{
		if (bResetStateWhenAvatarChanges)
		{
			ResetAbilitySystemComponent();	
		}

		InitializeDefaultsFromAvatar(InAvatarActor);
		OnAbilitySystemAvatarChanged.Broadcast(InAvatarActor);
	}
}

void UNinjaGASAbilitySystemComponent::InitializeDefaultsFromOwner(const AActor* NewOwner)
{
	if (!IsValid(NewOwner) || OwnerHandles.IsValid())
	{
		return;
	}

	const IAbilitySystemDefaultsInterface* Defaults = Cast<IAbilitySystemDefaultsInterface>(NewOwner);
	if (!Defaults || !Defaults->HasAbilityData())
	{
		Defaults = Cast<IAbilitySystemDefaultsInterface>(this);
	}

	check(Defaults != nullptr);
	const UNinjaGASDataAsset* AbilityData = Defaults->GetAbilityData();
	if (IsValid(AbilityData))
	{
		InitializeFromData(AbilityData, OwnerHandles);
	}
}

void UNinjaGASAbilitySystemComponent::InitializeDefaultsFromAvatar(const AActor* NewAvatar)
{
	// Avatar defaults are added on top of the defaults added by the ASC owner.
	// The new avatar has to be valid, and we need to make sure it's not the owner actor.
	if (!IsValid(NewAvatar) || NewAvatar == GetOwnerActor())
	{
		return;
	}

	static constexpr bool bRemovePermanentAttributes = true; 
	ClearDefaults(AvatarHandles, bRemovePermanentAttributes);
	
	const IAbilitySystemDefaultsInterface* Defaults = Cast<IAbilitySystemDefaultsInterface>(NewAvatar);
	if (Defaults && Defaults->HasAbilityData())
	{
		const UNinjaGASDataAsset* AbilityData = Defaults->GetAbilityData();
		if (IsValid(AbilityData))
		{
			InitializeFromData(AbilityData, AvatarHandles);
		}
	}
}

void UNinjaGASAbilitySystemComponent::InitializeFromData(const UNinjaGASDataAsset* AbilityData, FAbilityDefaultHandles& OutHandles)
{
	if (!IsValid(AbilityData))
	{
		return;
	}

	const TArray<FDefaultAttributeSet>& AttributeSets = AbilityData->DefaultAttributeSets;
	InitializeAttributeSets(AttributeSets, OutHandles);

	int32 TagCount = 0;
	const bool bIsAuth = IsOwnerActorAuthoritative(); 

	if (bIsAuth)
	{
		const TArray<FDefaultGameplayEffect>& GameplayEffects = AbilityData->DefaultGameplayEffects;
		InitializeGameplayEffects(GameplayEffects, OutHandles);

		const TArray<FDefaultGameplayAbility>& GameplayAbilities = AbilityData->DefaultGameplayAbilities;
		InitializeGameplayAbilities(GameplayAbilities, OutHandles);

		const FGameplayTagContainer& InitialGameplayTags = AbilityData->InitialGameplayTags; 
		if (InitialGameplayTags.IsValid())
		{
			TagCount = InitialGameplayTags.Num();
			#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
			AddLooseGameplayTags(InitialGameplayTags);
			#else
			AddReplicatedLooseGameplayTags(InitialGameplayTags);
			#endif
		}
	}

	OutHandles.CurrentAbilitySetup = AbilityData;
	UE_LOG(LogAbilitySystemComponent, Log, TEXT("Initialized ASC defaults on %s from %s: [ Permanent Attribute Sets: %d, Temporary Attribute Sets: %d, Effects: %d, Abilities: %d, Tags %d ]."),
		bIsAuth ? TEXT("auth") : TEXT("client"), *GetNameSafe(OutHandles.CurrentAbilitySetup), OutHandles.PermanentAttributes.Num(), OutHandles.TemporaryAttributes.Num(), OutHandles.DefaultEffectHandles.Num(), OutHandles.DefaultAbilityHandles.Num(), TagCount);		
}

void UNinjaGASAbilitySystemComponent::InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets, FAbilityDefaultHandles& OutHandles)
{
	const bool bIsAuth = IsOwnerActorAuthoritative();
	
	for (const FDefaultAttributeSet& Entry : AttributeSets)
	{
		if ((bIsAuth && Entry.AppliesOnServer()) || (!bIsAuth && Entry.AppliesOnClient()))
		{
			const TSubclassOf<UAttributeSet> AttributeSetClass = Entry.AttributeSetClass;
			if (!IsValid(AttributeSetClass))
			{
				UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Attribute Set Entry is missing a valid Attribute Set class!"));
				continue;	
			}

			if (GetSpawnedAttributes().ContainsByPredicate([AttributeSetClass](const UAttributeSet* AttributeSet){ return AttributeSet->GetClass() == AttributeSetClass; }))
			{
				UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Discarding Attribute Set %s since it was already spawned!"), *GetNameSafe(AttributeSetClass));
				continue;
			}
		
			UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(GetOwner(), AttributeSetClass);
			check(IsValid(NewAttributeSet));
		
			const UDataTable* AttributeTable = Entry.AttributeTable;
			if (IsValid(AttributeTable))
			{
				NewAttributeSet->InitFromMetaDataTable(AttributeTable);
			}

			AddAttributeSetSubobject(NewAttributeSet);

			if (Entry.IsPermanent())
			{
				OutHandles.PermanentAttributes.Add(NewAttributeSet);
				UE_LOG(LogAbilitySystemComponent, Verbose, TEXT("Initialized permanent Attribute Set %s with %s."), *GetNameSafe(NewAttributeSet), *GetNameSafe(AttributeTable));
			}
			else
			{
				OutHandles.TemporaryAttributes.Add(NewAttributeSet);
				UE_LOG(LogAbilitySystemComponent, Verbose, TEXT("Initialized temporary Attribute Set %s with %s."), *GetNameSafe(NewAttributeSet), *GetNameSafe(AttributeTable));
			}
		}
	}	
}

void UNinjaGASAbilitySystemComponent::InitializeGameplayEffects(const TArray<FDefaultGameplayEffect>& GameplayEffects, FAbilityDefaultHandles& OutHandles)
{
	const int32 GameplayEffectCount = GameplayEffects.Num(); 
	if (GameplayEffectCount > 0)
	{
		const int32 NewSize = OutHandles.DefaultEffectHandles.Num() + GameplayEffectCount;  
		OutHandles.DefaultEffectHandles.Reserve(NewSize);
		
		for (const FDefaultGameplayEffect& Entry : GameplayEffects)
		{
			const TSubclassOf<UGameplayEffect> GameplayEffectClass = Entry.GameplayEffectClass;
			FActiveGameplayEffectHandle Handle = ApplyGameplayEffectClassToSelf(GameplayEffectClass, Entry.Level);
			if (Handle.IsValid() && Handle.WasSuccessfullyApplied())
			{
				OutHandles.DefaultEffectHandles.Add(Handle);	
			}
		}
	}
}

void UNinjaGASAbilitySystemComponent::InitializeGameplayAbilities(const TArray<FDefaultGameplayAbility>& GameplayAbilities, FAbilityDefaultHandles& OutHandles)
{
	const int32 GameplayAbilityCount = GameplayAbilities.Num(); 
	if (GameplayAbilityCount > 0)
	{
		const int32 NewSize = OutHandles.DefaultAbilityHandles.Num() + GameplayAbilityCount; 
		OutHandles.DefaultAbilityHandles.Reserve(NewSize);
		
		for (const FDefaultGameplayAbility& Entry : GameplayAbilities)
		{
			const TSubclassOf<UGameplayAbility> GameplayAbilityClass = Entry.GameplayAbilityClass;
			FGameplayAbilitySpecHandle Handle = GiveAbilityFromClass(GameplayAbilityClass, Entry.Level, Entry.Input);
			if (Handle.IsValid())
			{
				OutHandles.DefaultAbilityHandles.Add(Handle);	
			}
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

void UNinjaGASAbilitySystemComponent::ResetAbilitySystemComponent()
{
	DestroyActiveState();
	RemoveAllGameplayCues();

	GameplayTagCountContainer.Reset();

	static constexpr bool bRemovePermanentAttributes = true;
	ClearDefaults(OwnerHandles, bRemovePermanentAttributes);
	ClearDefaults(AvatarHandles, bRemovePermanentAttributes);
}

void UNinjaGASAbilitySystemComponent::ClearActorInfo()
{
	ClearDefaults(AvatarHandles);
	Super::ClearActorInfo();
}

void UNinjaGASAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// As done by Lyra, use a replicated event instead of replicating the input directly.
	if (Spec.IsActive())
	{
		TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();

		const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfoRef();
		const FPredictionKey OriginalPredictionKey = ActivationInfo.GetActivationPredictionKey();

		// Invoke the replication event, providing a valid Prediction Key.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UNinjaGASAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// As done by Lyra, use a replicated event instead of replicating the input directly.
	if (Spec.IsActive())
	{
		TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
		
		const FGameplayAbilityActivationInfo& ActivationInfo = Instances.Last()->GetCurrentActivationInfoRef();
		const FPredictionKey OriginalPredictionKey = ActivationInfo.GetActivationPredictionKey();

		// Invoke the replication event, providing a valid Prediction Key.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
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

void UNinjaGASAbilitySystemComponent::CurrentMontageStop(const float OverrideBlendOutTime)
{
	if (GEnableDefaultPlayMontage)
	{
		// Always useful to still allow the default flow, if there are some meaningful changes in the core system
		// that were not yet reflect in this custom implementation. Can be enabled with CVar "GEnableDefaultPlayMontage".
		//
		Super::CurrentMontageStop(OverrideBlendOutTime);
	}

	UAnimInstance* AnimInstance = GetAnimInstanceFromActorInfo();

	const UAnimMontage* MontageToStop = LocalAnimMontageInfo.AnimMontage;
	const bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		const float BlendOutTime = (OverrideBlendOutTime >= 0.0f ? OverrideBlendOutTime : MontageToStop->BlendOut.GetBlendTime());
		AnimInstance->Montage_Stop(BlendOutTime, MontageToStop);

		if (IsOwnerActorAuthoritative())
		{
			AnimMontage_UpdateReplicatedData();
		}
	}
}

void UNinjaGASAbilitySystemComponent::SetReplicatedMontageInfo(FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo, UAnimMontage* NewMontageToPlay, const FName& StartSectionName)
{
	const uint8 PlayInstanceId = MutableRepAnimMontageInfo.PlayInstanceId < UINT8_MAX ? MutableRepAnimMontageInfo.PlayInstanceId + 1 : 0;
	const uint8 SectionIdToPlay = NewMontageToPlay->GetSectionIndex(StartSectionName) + 1;
	
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 3)
	
	MutableRepAnimMontageInfo.AnimMontage = NewMontageToPlay;
	MutableRepAnimMontageInfo.PlayInstanceId = PlayInstanceId;

	MutableRepAnimMontageInfo.SectionIdToPlay = 0;
	if (MutableRepAnimMontageInfo.AnimMontage && StartSectionName != NAME_None)
	{
		MutableRepAnimMontageInfo.SectionIdToPlay = SectionIdToPlay;
	}
	
#elif (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 3)

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

void UNinjaGASAbilitySystemComponent::DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, const float NewValue)
{
	const float OldValue = ActiveGameplayEffects.GetAttributeBaseValue(Attribute);
	ActiveGameplayEffects.SetAttributeBaseValue(Attribute, NewValue);
	SetBaseAttributeValueFromReplication(Attribute, NewValue, OldValue);
}

void UNinjaGASAbilitySystemComponent::DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue)
{
	const float OldValue = ActiveGameplayEffects.GetAttributeBaseValue(Attribute);
	ActiveGameplayEffects.SetAttributeBaseValue(Attribute, NewValue.GetBaseValue());
	SetBaseAttributeValueFromReplication(Attribute, NewValue.GetBaseValue(), OldValue);
}

void UNinjaGASAbilitySystemComponent::ClearDefaults(FAbilityDefaultHandles& Handles, const bool bRemovePermanentAttributes)
{
	int32 TagCount = 0;
	int32 AbilityHandleCount = 0;
	int32 EffectHandleCount = 0;
	int32 PermanentAttributeSetCount = 0;
	int32 TemporaryAttributeSetCount = 0;

	const bool bIsAuth = IsOwnerActorAuthoritative(); 
	if (bIsAuth)
	{
		FGameplayTagContainer InitialGameplayTags = FGameplayTagContainer::EmptyContainer;  
		if (IsValid(Handles.CurrentAbilitySetup))
		{
			InitialGameplayTags = Handles.CurrentAbilitySetup->InitialGameplayTags;
			if (InitialGameplayTags.IsValid())
			{
				TagCount = InitialGameplayTags.Num();

				#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7)
				RemoveLooseGameplayTags(InitialGameplayTags);
				#else
				RemoveReplicatedLooseGameplayTags(InitialGameplayTags);
				#endif
			}	
		}
	
		for (auto It(Handles.DefaultAbilityHandles.CreateIterator()); It; ++It)
		{
			const FGameplayAbilitySpecHandle& Handle = *It;
			const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
		
			if (Spec && Spec->Ability->GetAssetTags().HasTagExact(Tag_GAS_Ability_Passive))
			{
				// A passive ability will not end, so we need to deliberately cancel it first.
				CancelAbilityHandle(Handle);
			}
		
			SetRemoveAbilityOnEnd(Handle);	
			It.RemoveCurrent();
			++AbilityHandleCount;
		}
		
		for (auto It(Handles.DefaultEffectHandles.CreateIterator()); It; ++It)
		{
			RemoveActiveGameplayEffect(*It);
			It.RemoveCurrent();
			++EffectHandleCount;
		}
	}

	for (auto It(Handles.TemporaryAttributes.CreateIterator()); It; ++It)
	{
		RemoveSpawnedAttribute(*It);
		It.RemoveCurrent();
		++TemporaryAttributeSetCount;
	}

	if (bRemovePermanentAttributes)
	{
		for (auto It(Handles.PermanentAttributes.CreateIterator()); It; ++It)
		{
			RemoveSpawnedAttribute(*It);
			It.RemoveCurrent();
			++PermanentAttributeSetCount;
		}
	}

	Handles.CurrentAbilitySetup = nullptr;

	UE_LOG(LogAbilitySystemComponent, Log, TEXT("Cleared Gameplay Elements on %s for %s: [ Permanent Attribute Sets: %d, Temporary Attribute Sets: %d, Effects: %d, Abilities: %d, Tags: %d ]."),
		bIsAuth ? TEXT("auth") : TEXT("client"), *GetNameSafe(GetAvatarActor()), PermanentAttributeSetCount, TemporaryAttributeSetCount, EffectHandleCount, AbilityHandleCount, TagCount);
}
