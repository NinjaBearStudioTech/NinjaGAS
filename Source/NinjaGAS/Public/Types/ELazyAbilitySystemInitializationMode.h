// Ninja Bear Studio Inc., all rights reserved.
#pragma once

/**
 * Determines how the Ability System Component will be initialized.
 */
UENUM(BlueprintType)
enum class ELazyAbilitySystemInitializationMode : uint8
{
	/** The Ability System Component will be initialized when requested for the first time. */
	Lazy,

	/** The Ability System Component will be eagerly initialize as soon as the actor is spawned. */
	Eager
};