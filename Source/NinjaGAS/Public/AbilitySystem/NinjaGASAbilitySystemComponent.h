// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Types/FNinjaAbilityDefaults.h"
#include "NinjaGASAbilitySystemComponent.generated.h"

class UNinjaGASDataAsset;
class UAnimMontage;

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

/**
 * Specialized version of the Ability System Component.
 *
 * Includes many quality of life elements that are an aggregation of multiple common practices.
 *
 * - Data-driven configuration of default Gameplay Abilities.
 * - Support for runtime IK Retarget setup, where the animation is driven by a hidden mesh.
 * - Batch activation and local cues, as per Dan Tranek's GAS Compendium.
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
	virtual bool ShouldDoServerAbilityRPCBatch() const override;
	virtual float PlayMontage(UGameplayAbility* AnimatingAbility, FGameplayAbilityActivationInfo ActivationInfo, UAnimMontage* Montage, float InPlayRate, FName StartSectionName = NAME_None, float StartTimeSeconds = 0.0f) override;
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
	 * Determines if the ASC can batch-activate abilities.
	 *
	 * This means that multiple abilities activating together can be bundled in the same RPC.
	 * Once enabled, abilities can be activated in a batch via the "TryBatchActivate" functions.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", DisplayName = "Enable Ability Batch RPCs")
	bool bEnableAbilityBatchRPC;
	
	/**
	 * Initializes default abilities, effects and attribute sets.
	 *
	 * Should only be called when the owner is authoritative. However, calling from
	 * a client won't have any issues, but it won't do anything due to the internal check.
	 */
	void InitializeDefaults(const AActor* NewAvatarActor);

	/**
	 * Initializes Abilities from the provided Data Asset.
	 */
	void InitializeFromData(const AActor* NewAvatarActor, const UNinjaGASDataAsset* AbilityData);
	
	/**
	 * Initializes Attribute Sets provided by the interface.
	 */
	void InitializeAttributeSets(const TArray<FDefaultAttributeSet>& AttributeSets);

	/**
	 * Initializes the Gameplay Effects provided by the interface.
	 */
	void InitializeGameplayEffects(const TArray<FDefaultGameplayEffect>& GameplayEffects);

	/**
	 * Initializes the Gameplay Abilities provided by the interface.
	 */
	void InitializeGameplayAbilities(const TArray<FDefaultGameplayAbility>& GameplayAbilities);

	/**
	 * Clears default abilities, effects and attribute sets.
	 */
	virtual void ClearDefaults();

	/**
	 * Conveniently separates the code that sets the animation to replicate, so it can be further modified.
	 */
	virtual void SetReplicatedMontageInfo(FGameplayAbilityRepAnimMontage& MutableRepAnimMontageInfo, UAnimMontage* NewMontageToPlay, const FName& StartSectionName);
	
private:

	/** Actual ability setup used to initialize this ASC. */
	UPROPERTY()
	TObjectPtr<const UNinjaGASDataAsset> CurrentAbilitySetup;
	
	/** Attribute sets we initialized and are keeping track. */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> AddedAttributes;
	
	/** All effects we initialized by default. */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> DefaultEffectHandles;
	
	/** All abilities we initialized by default. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> DefaultAbilityHandles;

};
