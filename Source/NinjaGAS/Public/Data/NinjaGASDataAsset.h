// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Types/FNinjaAbilityDefaults.h"
#include "Engine/DataAsset.h"
#include "NinjaGASDataAsset.generated.h"

/**
 * Configures abilities that can be assigned to an avatar.
 */
UCLASS()
class NINJAGAS_API UNinjaGASDataAsset : public UPrimaryDataAsset
{
	
	GENERATED_BODY()

public:
	
	/** Asset type that uniquely identifies this data asset. */
	static FPrimaryAssetType AssetType;	

	/** List of Attribute Sets assigned to an avatar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (AssetBundles = "Abilities", TitleProperty = "AttributeSetClass"))
	TArray<FDefaultAttributeSet> DefaultAttributeSets;

	/** List of Gameplay Effects assigned to an avatar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (AssetBundles = "Abilities", TitleProperty = "GameplayEffectClass"))
	TArray<FDefaultGameplayEffect> DefaultGameplayEffects;

	/** List of Gameplay Abilities assigned to an avatar. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities", meta = (AssetBundles = "Abilities", TitleProperty = "GameplayAbilityClass"))
	TArray<FDefaultGameplayAbility> DefaultGameplayAbilities;

	/** Gameplay tags that are added by default to the owner's ASC. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FGameplayTagContainer InitialGameplayTags;
	
	UNinjaGASDataAsset();

	// -- Begin Primary Data Asset implementation
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// -- End Primary Data Asset implementation

};
