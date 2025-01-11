// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

// ------
// These includes are not used in the base class, but helpful in subclasses.
// ReSharper disable CppUnusedIncludeDirective
#include "AbilitySystemComponent.h"
#include "Interfaces/LazyAbilitySystemComponentOwnerInterface.h"
// ReSharper restore CppUnusedIncludeDirective
// ------

#include "NinjaGASAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * A base Attribute Set proving base functionality and exposing certain methods to Blueprints.
 */
UCLASS(Abstract)
class NINJAGAS_API UNinjaGASAttributeSet : public UAttributeSet
{
	
	GENERATED_BODY()

public:

	UNinjaGASAttributeSet();
	
};

/**
 * This macro has the same behavior as the original GAMEPLAYATTRIBUTE_REPNOTIFY, but it's aware of the potential lazy
 * initialization of the Ability System Component. In which case, it forwards the attribute to the owner, via the
 * appropriate interface.
 *	
 * void UMyHealthSet::OnRep_Health(const FGameplayAttributeData& OldValue)
 * {
 *     LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(UMyHealthSet, Health, OldValue);
 * }
 */
#define LAZY_GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, PropertyName, OldValue) \
{ \
	static FProperty* ThisProperty = FindFieldChecked<FProperty>(ClassName::StaticClass(), GET_MEMBER_NAME_CHECKED(ClassName, PropertyName)); \
	if (!GetOwningAbilitySystemComponent()) \
	{ \
		if (ILazyAbilitySystemComponentOwnerInterface* LazyComponentOwner = Cast<ILazyAbilitySystemComponentOwnerInterface>(GetOwningActor())) \
		{ \
			LazyComponentOwner->SetPendingAttributeFromReplication(FGameplayAttribute(ThisProperty), PropertyName); \
		} \
		return; \
	} \
	else \
	{ \
		GetOwningAbilitySystemComponentChecked()->SetBaseAttributeValueFromReplication(FGameplayAttribute(ThisProperty), PropertyName, OldValue); \
	} \
};