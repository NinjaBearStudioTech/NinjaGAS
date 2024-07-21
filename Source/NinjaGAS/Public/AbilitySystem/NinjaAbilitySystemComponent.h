// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Types/ENinjaGameplayEffectReplicationMode.h"
#include "Types/FNinjaAbilityDefaults.h"
#include "NinjaAbilitySystemComponent.generated.h"

class UNinjaAbilitiesDataAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogNinjaFrameworkAbilitySystemComponent, Log, All);

/**
 * Specialized version of the Ability System Component, supporting defaults and callbacks.
 */
UCLASS(ClassGroup=(NinjaGAS), meta=(BlueprintSpawnableComponent))
class NINJAGAS_API UNinjaAbilitySystemComponent : public UAbilitySystemComponent, public IAbilitySystemDefaultsInterface
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilitySystemAvatarChangedSignature, AActor*, NewAvatar);
	
	GENERATED_BODY()

public:

	/** Broadcasts a changed in the Avatar. */
	UPROPERTY(BlueprintAssignable)
	FAbilitySystemAvatarChangedSignature OnAbilitySystemAvatarChanged;
	
	UNinjaAbilitySystemComponent();

	// -- Begin Ability System Component implementation
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	virtual void ClearActorInfo() override;
	// -- End Ability System Component implementation

	// -- Begin Ability System Defaults implementation
	virtual void GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const override;
	virtual void GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const override;
	virtual void GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const override;	
	// -- End Ability System Defaults implementation
	
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
	 * Sets a new Replication Mode for this Ability System Component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Ability System")
	void SetGameplayReplicationMode(ENinjaGameplayEffectReplicationMode NewReplicationMode);
	
protected:

	/**
	 * Default configuration for the Ability System.
	 * 
	 * Can be overriden by avatars implementing the Ability System Defaults Interface,
	 * in which case this data asset is ignored in favour of that one. 
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UNinjaAbilitiesDataAsset> DefaultAbilitySetup;

	/**
	 * The Replication mode applied to this Ability System.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	ENinjaGameplayEffectReplicationMode ReplicationMode;
	
	/**
	 * Initializes default abilities, effects and attribute sets.
	 *
	 * Should only be called when the owner is authoritative. However, calling from
	 * a client won't have any issues, but it won't do anything due to the internal check.
	 */
	void InitializeDefaults(AActor* NewAvatarActor);

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
	
};
