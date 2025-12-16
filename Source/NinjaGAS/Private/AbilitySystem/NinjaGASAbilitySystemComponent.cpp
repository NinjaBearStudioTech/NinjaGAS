// Copyright (c) Ninja Bear Studio Inc.
// 
// This file incorporates portions of code from:
//   Copyright (c) Dan Kestranek (https://github.com/tranek)
//   Copyright (c) Jared Taylor (https://github.com/Vaei/)
//
// The incorporated portions are licensed under the MIT License.
// The full MIT license text is included in THIRD_PARTY_NOTICES.md.
//
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
#include "Net/UnrealNetwork.h"
#include "Types/FNinjaAbilityDefaultHandles.h"
#include "Runtime/Launch/Resources/Version.h"

bool FPlayTagGameplayAbilityRepAnimMontage::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityRepAnimMontage::NetSerialize(Ar, Map, bOutSuccess);

	Ar << bOverrideBlendIn;
	Ar << BlendInOverride.Blend.BlendTime;
	Ar << BlendInOverride.Blend.BlendOption;
	Ar << BlendInOverride.Blend.CustomCurve;
	Ar << BlendInOverride.BlendMode;
	Ar << BlendInOverride.BlendProfile;
	
	return true;
}

UNinjaGASAbilitySystemComponent::UNinjaGASAbilitySystemComponent()
{
	static constexpr bool bIsReplicated = true;
	SetIsReplicatedByDefault(bIsReplicated);

	bPendingMontageRepForMesh = false;
	bEnableAbilityBatchRPC = true;
	bResetStateWhenAvatarChanges = false;
}

void UNinjaGASAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	// Guard condition to ensure we should clear/init for a new Avatar Actor.
	const bool bAvatarHasChanged = AbilityActorInfo  && AbilityActorInfo->AvatarActor != InAvatarActor && InAvatarActor != nullptr;
	
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

void UNinjaGASAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, RepAnimMontageInfoForMeshes);
}

bool UNinjaGASAbilitySystemComponent::GetShouldTick() const
{
	for (const FGameplayAbilityRepAnimMontageForMesh& RepMontageInfo : RepAnimMontageInfoForMeshes)
	{
		const bool bHasReplicatedMontageInfoToUpdate = (IsOwnerActorAuthoritative() && RepMontageInfo.RepMontageInfo.IsStopped == false);

		if (bHasReplicatedMontageInfoToUpdate)
		{
			return true;
		}
	}
	return Super::GetShouldTick();
}

void UNinjaGASAbilitySystemComponent::TickComponent(float const DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsOwnerActorAuthoritative())
	{
		for (const FGameplayAbilityLocalAnimMontageForMesh& MontageInfo : LocalAnimMontageInfoForMeshes)
		{
			AnimMontage_UpdateReplicatedDataForMesh(MontageInfo.Mesh);
		}
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
