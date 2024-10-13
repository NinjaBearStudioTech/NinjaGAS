// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "FNinjaAbilityDefaults.generated.h"

class UAttributeSet;
class UDataTable;
class UGameplayAbility;
class UGameplayEffect;

/**
 * Default Attribute Set, with initialization data.
 */
USTRUCT(BlueprintType)
struct NINJAGAS_API FDefaultAttributeSet
{

	GENERATED_BODY();
	
	/** Attribute set class to grant. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Set")
	TSubclassOf<UAttributeSet> AttributeSetClass;

	/** Data table with default attribute values. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Set", meta = (RequiredAssetDataTags = "RowStructure=/Script/GameplayAbilities.AttributeMetaData"))
	TObjectPtr<const UDataTable> AttributeTable;

	FDefaultAttributeSet()
	{
	}
	
	FDefaultAttributeSet(const TSubclassOf<UAttributeSet>& AttributeSet, const UDataTable* AttributeTable)
		: AttributeSetClass(AttributeSet), AttributeTable(AttributeTable)
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
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	/** Initial level. */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effect")
	float Level = 1;

	FDefaultGameplayEffect()
	{
	}
	
	FDefaultGameplayEffect(const TSubclassOf<UGameplayEffect>& GameplayEffect, const float Level = 1)
		: GameplayEffectClass(GameplayEffect), Level(Level)
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
	TSubclassOf<UGameplayAbility> GameplayAbilityClass;

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
		: GameplayAbilityClass(GameplayAbility), Level(Level), Input(Input)
	{
	}
};