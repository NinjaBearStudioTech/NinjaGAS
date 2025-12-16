// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "StateTreeConsiderationBase.h"
#include "StateTreeAbilityCooldownConsideration.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FStateTreeAbilityCooldownConsiderationInstanceData
{
	
	GENERATED_BODY()

	/** Gameplay tags used to identify the desired cooldown. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagContainer AbilityCooldownTags = FGameplayTagContainer::EmptyContainer;

	/** Score applied when the ability is available. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float ScoreWhenAvailable = 1.f;
	
	/** Score applied when the ability is on cooldown. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float ScoreWhenOnCooldown = 0.f;
	
};

/**
 * Provides the utility value based on the ability cooldown.
 */
USTRUCT(DisplayName = "Ability Cooldown", Category = "GAS")
struct NINJAGAS_API FStateTreeAbilityCooldownConsideration : public FStateTreeConsiderationCommonBase
{
	
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeAbilityCooldownConsiderationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

protected:
	
	virtual float GetScore(FStateTreeExecutionContext& Context) const override;

	static UAbilitySystemComponent* GetAbilitySystemComponent(const UObject* Owner);
	static bool IsCooldownActive(const UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& AbilityCooldownTags); 
	
};
