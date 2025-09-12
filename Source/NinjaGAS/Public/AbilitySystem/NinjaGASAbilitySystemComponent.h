// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Types/FNinjaAbilityDefaultHandles.h"
#include "Types/FNinjaAbilityDefaults.h"
#include "NinjaGASAbilitySystemComponent.generated.h"

class UNinjaGASDataAsset;
class UAnimMontage;

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
	virtual float PlayMontage(UGameplayAbility* AnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f) override;
	virtual void CurrentMontageStop(float OverrideBlendOutTime) override;
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
	UFUNCTION(BlueprintPure, Category = "Ninja GAS|Ability System")
	virtual UAnimInstance* GetAnimInstanceFromActorInfo() const;
	
	/**
	 * Grants a new effect to the owner.
	 *
	 * @param EffectClass		Effect class being granted to the owner.
	 * @param Level				Initial level for the effect.
	 * @return					The handle that can be used for maintenance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
	FActiveGameplayEffectHandle ApplyGameplayEffectClassToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1);

	/**
	 * Grants a new ability to the owner.
	 * 
	 * @param AbilityClass		Ability class being granted to the owner.
	 * @param Level				Initial level for the ability.
	 * @param Input				An Input ID for the old input system.
	 * @return					The handle that can be used for activation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
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
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
	virtual bool TryBatchActivateAbility(FGameplayAbilitySpecHandle AbilityHandle, bool bEndAbilityImmediately);

	/**
	 * Cancels Gameplay Abilities by their matching tags.
	 *
	 * @param AbilityTags		Gameplay Tags used to target abilities to cancel.
	 * @param CancelFilterTags	A filter that excludes an ability from being cancelled.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
	virtual void CancelAbilitiesByTags(FGameplayTagContainer AbilityTags, FGameplayTagContainer CancelFilterTags);
	
	/**
	 * Locally executes a <b>Static<b> Gameplay Cue.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Ninja GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	void ExecuteGameplayCueLocal(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Locally adds an <b>Actor<b> Gameplay Cue.
	 *
	 * When adding this Gameplay Cue locally, make sure to also remove it locally.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Ninja GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void AddGameplayCueLocally(UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Locally removes an <b>Actor<b> Gameplay Cue.
	 * 
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */		
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Ninja GAS|Ability System", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void RemoveGameplayCueLocally(UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Resets all Gameplay Abilities, Gameplay Effects, Attribute Sets and additional Gameplay Cues.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
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

	/**
	 * Conveniently separates the code that sets the animation to replicate, so it can be further modified.
	 */
	virtual void SetReplicatedMontageInfo(FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo, UAnimMontage* NewMontageToPlay, const FName& StartSectionName);
	
private:

	/** Setup and handles granted by the owner. */
	FAbilityDefaultHandles OwnerHandles;

	/** Setup and handles granted by the avatar. */
	FAbilityDefaultHandles AvatarHandles;
	
};
