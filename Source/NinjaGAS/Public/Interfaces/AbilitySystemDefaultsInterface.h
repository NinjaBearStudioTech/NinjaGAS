// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "AbilitySystemDefaultsInterface.generated.h"

class UNinjaGASDataAsset;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAbilitySystemDefaultsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides the bundle with default attributes, effects and abilities to the Ability Component.
 */
class NINJAGAS_API IAbilitySystemDefaultsInterface
{
	
	GENERATED_BODY()

public:

	/** Provides the default bundle for the avatar. */
	virtual const UNinjaGASDataAsset* GetAbilityData() const = 0;
	
	/** Informs if an Ability Bundle is available. */
	virtual bool HasAbilityData() const { return GetAbilityData() != nullptr; }
};
