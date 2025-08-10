// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "FNinjaAbilityDefaults.generated.h"

class UAttributeSet;
class UDataTable;
class UGameplayAbility;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EAttributeSetInstantiationScope : uint8
{
	/** This attribute set will only be applied in the local client. */
	Local,

	/** This attribute set will only be applied in the server. */
	Server,

	/** This attribute set will be applied in both the server and local client. */
	Both
};

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

	/** Scope for this Attribute Set. */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Set")
	EAttributeSetInstantiationScope InstantiationScope = EAttributeSetInstantiationScope::Both;
	
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

	bool AppliesOnServer() const
	{
		return InstantiationScope == EAttributeSetInstantiationScope::Server
			|| InstantiationScope == EAttributeSetInstantiationScope::Both;
	}

	bool AppliesOnClient() const
	{
		return InstantiationScope == EAttributeSetInstantiationScope::Local
			|| InstantiationScope == EAttributeSetInstantiationScope::Both;
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