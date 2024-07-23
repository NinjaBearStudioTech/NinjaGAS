// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Interfaces/AbilitySystemDefaultsInterface.h"
#include "GameFramework/Character.h"
#include "NinjaGASCharacter.generated.h"

class UNinjaGASDataAsset;
class UNinjaGASAbilitySystemComponent;

/**
 * Base character class, with pre-configured Ability System Component features.
 */
UCLASS(Abstract)
class NINJAGAS_API ANinjaGASCharacter : public ACharacter, public IAbilitySystemInterface, public IAbilitySystemDefaultsInterface
{
	
	GENERATED_BODY()

public:

	ANinjaGASCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Ability System implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual bool HasDefaultAbilitySettings() const override;
	virtual void GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const override;
	virtual void GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const override;
	virtual void GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const override;
	// -- End Ability System implementation

protected:

	/** Informs if this character has an override for the settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", meta = (InlineEditConditionToggle))
	bool bOverridesAbilitySettings;
	
	/** Default configuration for the Ability System. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System", meta = (EditCondition = "bOverridesAbilitySettings"))
	TObjectPtr<UNinjaGASDataAsset> DefaultAbilitySetup;

	/**
	 * Initializes the ability system component using the source as an avatar.
	 *
	 * @param AbilitySystemOwner
	 *		Actor that owns the Ability System Component. It may be the same character
	 *		or an external source such as the Player State.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ninja GAS|Character")
	virtual void SetupAbilitySystemComponent(AActor* AbilitySystemOwner);
	
	/**
	 * Clears the ability system component, most likely due to the character being destroyed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ninja GAS|Character")
	virtual void ClearAbilitySystemComponent();
	
private:

	/** Weak reference to the Character's Ability Component. */
	TWeakObjectPtr<UNinjaGASAbilitySystemComponent> CharacterAbilitiesPtr;
	
};
