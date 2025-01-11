// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "Abilities/GameplayAbilityTypes.h"

/**
 * Represents attributes that were not replicated yet.
 */
struct FPendingAttributeReplication
{
	/** The attribute that must be replicated. */
	FGameplayAttribute Attribute;

	/** The value that is pending replication. */
	FGameplayAttributeData NewValue;

	FPendingAttributeReplication() { }

	FPendingAttributeReplication(const FGameplayAttribute& InAttribute, const FGameplayAttributeData& InNewValue)
	{
		Attribute = InAttribute;
		NewValue = InNewValue;
	}
};