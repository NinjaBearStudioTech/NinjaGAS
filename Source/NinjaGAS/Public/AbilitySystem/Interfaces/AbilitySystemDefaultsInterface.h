// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Types/FNinjaAbilityDefaults.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "AbilitySystemDefaultsInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAbilitySystemDefaultsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Provides default attributes, effects and abilities to the Ability Component.
 */
class NINJAGAS_API IAbilitySystemDefaultsInterface
{
	
	GENERATED_BODY()

public:
	
	/** Attribute sets assigned by default. */
	virtual void GetDefaultAttributeSets(TArray<FDefaultAttributeSet>& OutAttributeSets) const = 0;

	/** Gameplay effects applied by default. */
	virtual void GetDefaultGameplayEffects(TArray<FDefaultGameplayEffect>& OutDefaultEffects) const = 0;
	
	/** Gameplay abilities granted by default. */
	virtual void GetDefaultGameplayAbilities(TArray<FDefaultGameplayAbility>& OutDefaultAbilities) const = 0;	
	
};
