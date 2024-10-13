// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "EAgentAbilityActivationMode.generated.h"

/**
 * Determines how an AI Agent will try to activate an Ability.
 */
UENUM(BlueprintType)
enum class EAgentAbilityActivationMode : uint8
{
	
    /** Attempt to activate the ability by its class. */
    AbilityClass,
    
    /** Attempt to activate the ability by its class, retrieved by the blackboard. */
    BlackboardClass,

    /** Activates the ability by gameplay tags. */
    GameplayTags
	
};
