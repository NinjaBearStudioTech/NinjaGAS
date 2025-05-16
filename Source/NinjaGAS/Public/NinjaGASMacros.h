// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"

/**
 * Clamps an attribute in the given range expressed by min and max values.
 * Most likely used in the "PostGameplayEffectExecute" function in the Attribute Set.
 * 
 * @param AttrName		Name of the Attribute that will be clamped.
 * @param Min			Minimum value allowed for the attribute.
 * @param Max			Maximum value allowed for the attribute.
 */
#define CLAMP_GAS_ATTRIBUTE_RANGE(AttrName, Min, Max) \
if (Data.EvaluatedData.Attribute == Get##AttrName##Attribute()) \
{ \
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent(); \
	if (ensure(AbilityComp)) \
	{ \
	float Value = AbilityComp->GetNumericAttribute(Get##AttrName##Attribute()); \
	Value = FMath::Clamp(Value, Min, Max); \
	AbilityComp->SetNumericAttributeBase(Get##AttrName##Attribute(), Value); \
	} \
}

/**
 * Clamps an attribute in the given range expressed by zero and max values.
 * Most likely used in the "PostGameplayEffectExecute" function in the Attribute Set.
 *
 * @param AttrName		Name of the Attribute that will be clamped.
 * @param Max			Maximum value allowed for the attribute.
 */
#define CLAMP_GAS_ATTRIBUTE_ZERO_MAX(AttrName, Max) CLAMP_GAS_ATTRIBUTE_RANGE(AttrName, 0.f, Max)

/**
 * Clamps an attribute in the given range expressed by zero and one values.
 * Most likely used in the "PostGameplayEffectExecute" function in the Attribute Set.
 *
 * @param AttrName		Name of the Attribute that will be clamped.
 */
#define CLAMP_GAS_ATTRIBUTE_ZERO_ONE(AttrName) CLAMP_GAS_ATTRIBUTE_RANGE(AttrName, 0.f, 1.f)