// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Types/ELazyAbilitySystemInitializationMode.h"
#include "LazyAbilitySystemComponentOwnerInterface.generated.h"

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class ULazyAbilitySystemComponentOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * An interface for actors, pawns and characters that support lazy initialization for the Ability System Component.
 */
class NINJAGAS_API ILazyAbilitySystemComponentOwnerInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the initialization mode for the Ability System Component.
	 */
	virtual ELazyAbilitySystemInitializationMode GetAbilitySystemInitializationMode() const = 0;

	/**
	 * Logic invoked to initialize the Ability System Component, when necessary.
	 */
	virtual void InitializeAbilitySystemComponent() = 0;	
	
	/**
	 * Sets an Attribute Value, that is pending while the Attribute Set is being initialized/replicated.
	 *
	 * @param Attribute		The attribute being modified.
	 * @param NewValue		New value for the attribute.
	 */
	virtual void SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue) = 0;

	/**
	 * Handles pending attributes when the Ability System Component is initialized.
	 */
	virtual void ApplyPendingAttributesFromReplication() = 0;

};
