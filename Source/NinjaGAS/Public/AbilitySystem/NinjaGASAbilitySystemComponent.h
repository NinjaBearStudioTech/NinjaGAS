// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Engine/StreamableManager.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Types/FNinjaAbilityDefaults.h"
#include "NinjaGASAbilitySystemComponent.generated.h"

class UNinjaGASDataAsset;
class UAnimMontage;

/**
 * Specialized version of the Ability System Component, supporting defaults and callbacks.
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
	virtual UNinjaGASDataAsset* GetAbilityBundle() const override;
	// -- End Ability System Defaults implementation

	UAnimInstance* GetAnimInstanceFromActorInfo() const;
	
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
	
protected:

	/**
	 * Default configuration for the Ability System.
	 * 
	 * Can be overriden by avatars implementing the Ability System Defaults Interface.
	 * If avatars override the default data asset, this one is fully ignored.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UNinjaGASDataAsset> DefaultAbilitySetup;

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
	 * Initializes the bundle that has been loaded (or was already loaded).
	 */
	void InitializeFromBundle(const AActor* NewAvatarActor, const UNinjaGASDataAsset* AbilityBundle);
	
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

	/** Attribute sets we initialized and are keeping track. */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> AddedAttributes;
	
	/** All effects we initialized by default. */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> DefaultEffectHandles;
	
	/** All abilities we initialized by default. */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> DefaultAbilityHandles;

	/** Handle for the current default configuration. */
	TSharedPtr<FStreamableHandle> AbilityBundleHandle;
};
