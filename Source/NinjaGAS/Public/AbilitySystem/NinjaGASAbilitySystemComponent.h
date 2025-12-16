// Copyright (c) Ninja Bear Studio Inc.
// 
// This file incorporates portions of code from:
//   Copyright (c) Dan Kestranek (https://github.com/tranek)
//   Copyright (c) Jared Taylor (https://github.com/Vaei/)
//
// The incorporated portions are licensed under the MIT License.
// The full MIT license text is included in THIRD_PARTY_NOTICES.md.
//
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Types/FNinjaAbilityDefaultHandles.h"
#include "Types/FNinjaAbilityDefaults.h"
#include "NinjaGASAbilitySystemComponent.generated.h"

struct FMontageBlendSettings;

class UNinjaGASDataAsset;
class UAnimMontage;
class USkeletalMeshComponent;

/**
 * Data about montages that were played locally (all montages in case of server. predictive montages in case of client). Never replicated directly.
 */
USTRUCT()
struct NINJAGAS_API FGameplayAbilityLocalAnimMontageForMesh
{
	GENERATED_BODY();

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh = nullptr)
		: Mesh(InMesh)
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh, const FGameplayAbilityLocalAnimMontage& InLocalMontageInfo)
		: Mesh(InMesh), LocalMontageInfo(InLocalMontageInfo)
	{
	}
};

USTRUCT()
struct NINJAGAS_API FPlayTagGameplayAbilityRepAnimMontage : public FGameplayAbilityRepAnimMontage
{
	GENERATED_BODY()

	UPROPERTY()
	bool bOverrideBlendIn;

	UPROPERTY()
	FMontageBlendSettings BlendInOverride;

	FPlayTagGameplayAbilityRepAnimMontage()
		: bOverrideBlendIn(false)
		, BlendInOverride({})
	{}
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

/**
 * Data about montages that is replicated to simulated clients.
 */
USTRUCT()
struct NINJAGAS_API FGameplayAbilityRepAnimMontageForMesh
{
	GENERATED_BODY();

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FPlayTagGameplayAbilityRepAnimMontage RepMontageInfo;

	FGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh = nullptr)
		: Mesh(InMesh)
	{
	}
};

template<>
struct TStructOpsTypeTraits<FPlayTagGameplayAbilityRepAnimMontage> : public TStructOpsTypeTraitsBase2<FPlayTagGameplayAbilityRepAnimMontage>
{
	enum
	{
		WithNetSerializer = true,
	};
};

/**
 * Specialized version of the Ability System Component.
 *
 * Includes many quality of life elements that are an aggregation of multiple common practices.
 *
 * - Data-driven configuration of default Gameplay Abilities.
 * - Support for runtime IK Retarget setup, where the animation is driven by a hidden mesh.
 * - Batch activation and local cues, as per Dan Tranek's GAS Compendium.
 * - Automatic reset of the ASC when the avatar changes, or on-demand.
 * - Lazy loading the ASC, as per Alvaro Jover-Alvarez (Vorixo)'s blog.
 *
 * Additional references:
 * 
 * - https://github.com/tranek/GASDocumentation
 * - https://vorixo.github.io/devtricks/lazy-loading-asc/
 * - https://github.com/Vaei/PlayMontagePro/
 */
UCLASS(ClassGroup=(NinjaGAS), meta=(BlueprintSpawnableComponent))
class NINJAGAS_API UNinjaGASAbilitySystemComponent : public UAbilitySystemComponent, public IAbilitySystemDefaultsInterface
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilitySystemAvatarChangedSignature, AActor*, NewAvatar);
	
	GENERATED_BODY()

public:

	/** Broadcasts a changed in the Avatar. */
	UPROPERTY(BlueprintAssignable)
	FAbilitySystemAvatarChangedSignature OnAbilitySystemAvatarChanged;
	
	UNinjaGASAbilitySystemComponent();

	// -- Begin Ability System Component implementation
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void ClearActorInfo() override;
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	virtual bool ShouldDoServerAbilityRPCBatch() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool GetShouldTick() const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// -- End Ability System Component implementation

	// -- Begin Ability System Defaults implementation
	virtual const UNinjaGASDataAsset* GetAbilityData() const override;
	// -- End Ability System Defaults implementation

	/**
	 * Obtains the Anim Instance from the Actor Info.
	 *
	 * Takes into consideration the pointer in the Actor Info, before calling the
	 * Getter function that will always attempt to retrieve it from the mesh.
	 */
	UFUNCTION(BlueprintPure, Category = "NBS|GAS|Ability System")
	virtual UAnimInstance* GetAnimInstanceFromActorInfo() const;
	
	/**
	 * Grants a new effect to the owner.
	 *
	 * @param EffectClass		Effect class being granted to the owner.
	 * @param Level				Initial level for the effect.
	 * @return					The handle that can be used for maintenance.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Ability System")
	FActiveGameplayEffectHandle ApplyGameplayEffectClassToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1);

	/**
	 * Grants a new ability to the owner.
	 * 
	 * @param AbilityClass		Ability class being granted to the owner.
	 * @param Level				Initial level for the ability.
	 * @param Input				An Input ID for the old input system.
	 * @return					The handle that can be used for activation.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Ability System")
	FGameplayAbilitySpecHandle GiveAbilityFromClass(const TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1, int32 Input = -1);

	/**
	 * Tries to activate the ability by the handle, aggregating all RPCs that happened in the same frame.
	 *
	 * @param AbilityHandle
	 *		Handle used to identify the ability.
	 *		
	 * @param bEndAbilityImmediately
	 *		Determines if the EndAbility is triggered right away or later, with its own RPC. This requires the Ability
	 *		to either implement IBatchGameplayAbilityInterface or be a subclass of NinjaGASGameplayAbility.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Ability System")
	virtual bool TryBatchActivateAbility(FGameplayAbilitySpecHandle AbilityHandle, bool bEndAbilityImmediately);

	/**
	 * Cancels Gameplay Abilities by their matching tags.
	 *
	 * @param AbilityTags		Gameplay Tags used to target abilities to cancel.
	 * @param CancelFilterTags	A filter that excludes an ability from being cancelled.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Ability System")
	virtual void CancelAbilitiesByTags(FGameplayTagContainer AbilityTags, FGameplayTagContainer CancelFilterTags);
	
	/**
	 * Locally executes a <b>Static<b> Gameplay Cue.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "NBS|GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Locally adds an <b>Actor<b> Gameplay Cue.
	 *
	 * When adding this Gameplay Cue locally, make sure to also remove it locally.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "NBS|GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void AddGameplayCueLocally(UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Locally removes an <b>Actor<b> Gameplay Cue.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */		
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "NBS|GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void RemoveGameplayCueLocally(UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Resets all Gameplay Abilities, Gameplay Effects, Attribute Sets and additional Gameplay Cues.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Ability System")
	virtual void ResetAbilitySystemComponent();
	
	/**
	 * Sets a base attribute value, after a deferred/lazy initialization.
	 *
	 * @param Attribute		Attribute being updated.
	 * @param NewValue		New float value to be set.
	 */
	void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, float NewValue);

	/**
	 * Sets a base attribute value, after a deferred/lazy initialization.
	 *
	 * @param Attribute		Attribute being updated.
	 * @param NewValue		New attribute data value to be applied.
	 */	
	void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue);
	
protected:

	/**
	 * Default configuration for the Ability System.
	 * 
	 * Can be overriden by avatars implementing the Ability System Defaults Interface.
	 * If avatars override the default data asset, this one is fully ignored.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<const UNinjaGASDataAsset> DefaultAbilitySetup;

	/**
	 * If set to true, fully resets the ASC State when the avatar changes.
	 *
	 * This means removing all gameplay abilities, gameplay effects and spawned attributes,
	 * before assigning new ones from scratch, using the default setup assigned to the new
	 * avatar, or to this component.
	 *
	 * If you don't want to fully reset the state for any avatar change, then you can set this
	 * to "false", and call "ResetAbilitySystemComponent" whenever the new avatar activates.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	bool bResetStateWhenAvatarChanges;
	
	/**
	 * Determines if the ASC can batch-activate abilities.
	 *
	 * This means that multiple abilities activating together can be bundled in the same RPC.
	 * Once enabled, abilities can be activated in a batch via the "TryBatchActivate" functions.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", DisplayName = "Enable Ability Batch RPCs")
	bool bEnableAbilityBatchRPC;
	
	/**
	 * Initializes default abilities, effects and attribute sets granted by the owner.
	 * The initialization for owner defaults only ever happens once, since the owner won't change.
	 */
	void InitializeDefaultsFromOwner(const AActor* NewOwner);

	/**
	 * Initializes default abilities, effects and attribute sets granted by the avatar.
	 * This will also reset previous defaults granted by a former avatar, if the avatar changes.
	 */
	void InitializeDefaultsFromAvatar(const AActor* NewAvatar);
	
	/**
	 * Initializes Abilities from the provided Data Asset.
	 */
	void InitializeFromData(const UNinjaGASDataAsset* AbilityData, FAbilityDefaultHandles& OutHandles);
	
	/**
	 * Initializes Attribute Sets provided by the interface.
	 */
	void InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets, FAbilityDefaultHandles& OutHandles);

	/**
	 * Initializes the Gameplay Effects provided by the interface.
	 */
	void InitializeGameplayEffects(const TArray<FDefaultGameplayEffect>& GameplayEffects, FAbilityDefaultHandles& OutHandles);

	/**
	 * Initializes the Gameplay Abilities provided by the interface.
	 */
	void InitializeGameplayAbilities(const TArray<FDefaultGameplayAbility>& GameplayAbilities, FAbilityDefaultHandles& OutHandles);

	/**
	 * Clears default abilities, effects and attribute sets.
	 */
	virtual void ClearDefaults(FAbilityDefaultHandles& Handles, bool bRemovePermanentAttributes = false);

private:

	/** Setup and handles granted by the owner. */
	FAbilityDefaultHandles OwnerHandles;

	/** Setup and handles granted by the avatar. */
	FAbilityDefaultHandles AvatarHandles;

#pragma region AnimationMontages
public:
	
	// ----------------------------------------------------------------------------------------------------------------
	//	AnimMontage Support for multiple USkeletalMeshComponents on the AvatarActor.
	//  Only one ability can be animating at a time though?
	// ----------------------------------------------------------------------------------------------------------------		
	
	/** 
	 * Plays a montage and handles replication and prediction based on passed in ability/activation info 
	 * 
	 * Marked as UFUNCTION so it can be seen by a reflection system, in case we want to dynamically swap 
	 * between this and the default Ability System Component, based on whichever is currently available. 
	 */
	UFUNCTION()
	virtual float PlayMontageForMesh(UGameplayAbility* AnimatingAbility, USkeletalMeshComponent* InMesh, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, bool bOverrideBlendIn, const FMontageBlendSettings& BlendInOverride, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.f, bool bReplicateMontage = true);

	// Plays a montage without updating replication/prediction structures. Used by simulated proxies when replication tells them to play a montage.
	virtual float PlayMontageSimulatedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* Montage, float InPlayRate, bool bOverrideBlendIn, const FMontageBlendSettings& BlendInOverride, float StartTimeSeconds = 0.f, FName StartSectionName = NAME_None);

	// Stops whatever montage is currently playing. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
	virtual void CurrentMontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	// Stops all montages currently playing
	virtual void StopAllCurrentMontages(float OverrideBlendOutTime = -1.0f);

	// Stops current montage if it's the one given as the Montage param
	virtual void StopMontageIfCurrentForMesh(USkeletalMeshComponent* InMesh, const UAnimMontage& Montage, float OverrideBlendOutTime = -1.0f);

	// Clear the animating ability that is passed in, if it's still currently animating
	virtual void ClearAnimatingAbilityForAllMeshes(UGameplayAbility* Ability);

	// Jumps current montage to given section. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
	virtual void CurrentMontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);

	// Sets current montages next section name. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
	virtual void CurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);

	// Sets current montage's play rate
	virtual void CurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, float InPlayRate);

	// Returns true if the passed in ability is the current animating ability
	bool IsAnimatingAbilityForAnyMesh(const UGameplayAbility* Ability) const;

	// Returns the current animating ability
	UGameplayAbility* GetAnimatingAbilityFromAnyMesh();

	// Returns the current animating ability
	UGameplayAbility* GetAnimatingAbilityFromMesh(USkeletalMeshComponent* InMesh);

	// Returns montages that are currently playing
	TArray<UAnimMontage*> GetCurrentMontages() const;

	// Returns the montage that is playing for the mesh
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	// Get SectionID of currently playing AnimMontage
	int32 GetCurrentMontageSectionIDForMesh(USkeletalMeshComponent* InMesh);

	// Get SectionName of currently playing AnimMontage
	FName GetCurrentMontageSectionNameForMesh(USkeletalMeshComponent* InMesh);

	// Get length in time of current section
	float GetCurrentMontageSectionLengthForMesh(USkeletalMeshComponent* InMesh);

	// Returns amount of time left in current section
	float GetCurrentMontageSectionTimeLeftForMesh(USkeletalMeshComponent* InMesh);		
	
protected:
	
	UPROPERTY()
	bool bPendingMontageRepForMesh;
	
	/** 
	 * Data structure for montages that were instigated locally (everything if server, predictive if client. replicated if simulated proxy).
	 * Will be max one element per skeletal mesh on the AvatarActor.
	 */
	UPROPERTY()
	TArray<FGameplayAbilityLocalAnimMontageForMesh> LocalAnimMontageInfoForMeshes;
	
	/** 
	 * Data structure for replicating montage info to simulated clients
	 * Will be max one element per skeletal mesh on the AvatarActor
	 */
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh)
	TArray<FGameplayAbilityRepAnimMontageForMesh> RepAnimMontageInfoForMeshes;	

	// Finds the existing FGameplayAbilityLocalAnimMontageForMesh for the mesh or creates one if it doesn't exist
	FGameplayAbilityLocalAnimMontageForMesh& GetLocalAnimMontageInfoForMesh(USkeletalMeshComponent* InMesh);
	
	// Finds the existing FGameplayAbilityRepAnimMontageForMesh for the mesh or creates one if it doesn't exist
	FGameplayAbilityRepAnimMontageForMesh& GetGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh);

	// Called when a prediction key that played a montage is rejected
	virtual void OnPredictiveMontageRejectedForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* PredictiveMontage);

	// Copy LocalAnimMontageInfo into RepAnimMontageInfo
	void AnimMontage_UpdateReplicatedDataForMesh(USkeletalMeshComponent* InMesh);
	void AnimMontage_UpdateReplicatedDataForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);

	// Copy over playing flags for duplicate animation data
	void AnimMontage_UpdateForcedPlayFlagsForMesh(FGameplayAbilityRepAnimMontageForMesh& OutRepAnimMontageInfo);	

	UFUNCTION()
	virtual void OnRep_ReplicatedAnimMontageForMesh();

	// Returns true if we are ready to handle replicated montage information
	virtual bool IsReadyForReplicatedMontageForMesh();
	
	// RPC function called from CurrentMontageSetNextSectionName, replicates to other clients
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float ClientPosition, FName SectionName, FName NextSectionName);

	// RPC function called from CurrentMontageJumpToSection, replicates to other clients
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageJumpToSectionNameForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, FName SectionName);

	// RPC function called from CurrentMontageSetPlayRate, replicates to other clients
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCurrentMontageSetPlayRateForMesh(USkeletalMeshComponent* InMesh, UAnimMontage* ClientAnimMontage, float InPlayRate);
	
#pragma endregion
};
