// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "Templates/SubclassOf.h"
#include "FNinjaAbilityDefaultHandles.generated.h"

class UNinjaGASDataAsset;
class UAttributeSet;

USTRUCT(BlueprintType)
struct NINJAGAS_API FAbilityDefaultHandles
{
	GENERATED_BODY()

	/** The setup that granted these handles. */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Handles")
	TObjectPtr<const UNinjaGASDataAsset> CurrentAbilitySetup;
	
	/**
	 * Attribute sets we initialized and will keep regardless of respawns.
	 * Attributes granted by the owner may be permanent.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Handles")
	TArray<TObjectPtr<UAttributeSet>> PermanentAttributes;

	/**
	 * Attribute sets we initialized and will reset when the avatar changes.
	 * Attributes granted by the avatar are always temporary.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Handles")	
	TArray<TObjectPtr<UAttributeSet>> TemporaryAttributes;
	
	/** All effects we initialized by default. */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Handles")
	TArray<FActiveGameplayEffectHandle> DefaultEffectHandles;
	
	/** All abilities we initialized by default. */
	UPROPERTY(BlueprintReadOnly, Category = "Ability Handles")
	TArray<FGameplayAbilitySpecHandle> DefaultAbilityHandles;	

	/** Informs if there are any valid handles or data assigned. */
	bool IsValid() const;
	
};