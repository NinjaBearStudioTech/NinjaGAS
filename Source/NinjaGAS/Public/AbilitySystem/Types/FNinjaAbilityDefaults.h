// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "FNinjaAbilityDefaults.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAttributeSet;

/**
 * Default Attribute Set, with initialization data.
 */
USTRUCT(BlueprintType)
struct NINJAGAS_API FDefaultAttributeSet
{

	GENERATED_BODY();
	
	/** Attribute set class to grant. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Set")
	TSubclassOf<UAttributeSet> AttributeSet;

	/** Data table with default attribute values. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Set")
	TObjectPtr<UDataTable> AttributeTable;

	FDefaultAttributeSet()
	{
	}
	
	FDefaultAttributeSet(const TSubclassOf<UAttributeSet>& AttributeSet, UDataTable* AttributeTable)
		: AttributeSet(AttributeSet), AttributeTable(AttributeTable)
	{
	}
};

/**
 * Default Gameplay Effect.
 */
USTRUCT(BlueprintType)
struct NINJAGAS_API FDefaultGameplayEffect
{

	GENERATED_BODY();
	
	/** Gameplay Effect class to grant. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	/** Initial level. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	float Level = 1;

	FDefaultGameplayEffect()
	{
	}
	
	FDefaultGameplayEffect(const TSubclassOf<UGameplayEffect>& GameplayEffect, const float Level = 1)
		: GameplayEffect(GameplayEffect), Level(Level)
	{
	}
};

/**
 * Default Gameplay Ability.
 */
USTRUCT(BlueprintType)
struct NINJAGAS_API FDefaultGameplayAbility
{

	GENERATED_BODY();
	
	/** Gameplay Ability class to grant. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	/** Initial level. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	int32 Level = 1;

	/** Input assigned to this ability. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	int32 Input = INDEX_NONE;
	
	FDefaultGameplayAbility()
	{
	}
	
	FDefaultGameplayAbility(const TSubclassOf<UGameplayAbility>& GameplayAbility, const int32 Level = 1, const float Input = INDEX_NONE)
		: GameplayAbility(GameplayAbility), Level(Level), Input(Input)
	{
	}
};