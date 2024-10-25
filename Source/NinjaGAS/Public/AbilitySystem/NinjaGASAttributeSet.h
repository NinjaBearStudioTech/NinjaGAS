// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "NinjaGASAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * A base Attribute Set proving base functionality and exposing certain methods to Blueprints.
 */
UCLASS(Abstract, Hidden)
class NINJAGAS_API UNinjaGASAttributeSet : public UAttributeSet
{
	
	GENERATED_BODY()

public:

	UNinjaGASAttributeSet();
	
};
