// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "StateTreeAbilityTrackerEvaluator.generated.h"

USTRUCT()
struct FStateTreeAbilityTrackerEvaluatorInstanceData
{
	
	GENERATED_BODY()

	/**
	 * Filter used to determine which abilities to include or exclude from the tracking.
	 * If this query is left empty, then this filter will have no effect.
	 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagQuery AbilityFilterQuery = FGameplayTagQuery::EmptyQuery;

	/** Ability Tags for the last ability that has ended. */
	UPROPERTY(EditAnywhere, Category = Output)
	FGameplayTagContainer LastAbilityTags = FGameplayTagContainer::EmptyContainer;

	/**
	 * The number of consecutive executions for the last ability.
	 *
	 * This counter starts when an ability first successfully ends, and is reset when
	 * an ability is cancelled. New tracked abilities will also restart the streak.
	 */
	UPROPERTY(EditAnywhere, Category = Output)
	int32 ConsecutiveExecutions = 0;
	
	/** Delegate Handle provided by the ASC. */
	FDelegateHandle AbilityEndedHandle;
	
};

/**
 * Tracks how many times a gameplay ability has been completed in a row.
 * 
 * This is useful to track consecutive ability executions and potentially use that as a way
 * to determine if the ability should be executed again or another state should be picked,
 * maybe as a condition or as a utility consideration.
 *
 * Abilities are considered to be the same, based on the Gameplay Tags used to identify them,
 * which is done in their "Ability"/"Asset" tags container.
 */
USTRUCT(DisplayName = "Ability Tracker", Category = "G.A.S.")
struct NINJAGAS_API FStateTreeAbilityTrackerEvaluator : public FStateTreeEvaluatorCommonBase
{
	
	GENERATED_BODY()

	FStateTreeAbilityTrackerEvaluator() = default;

	using FInstanceDataType = FStateTreeAbilityTrackerEvaluatorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	// -- Begin State Tree Evaluator implementation
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	// -- End State Tree Evaluator implementation

protected:

	/**
	 * Retrieves the Ability System Component from the AI Controller in the context. 
	 */
	static UAbilitySystemComponent* GetAbilitySystemComponent(const FStateTreeExecutionContext& Context);
	
	/**
	 * Check for incoming data, to make sure it can be processed.
	 */
	static bool HasValidData(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData);
	
	/**
	 * Checks if the ability that has ended is relevant, based on this evaluator setup.
	 */
	static bool IsAbilityRelevant(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData);

	/**
	 * Checks if the ability that has ended is the same ability currently being tracked.
	 */
	static bool IsSameAbility(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData);
	
	/**
	 * Retrieves the ability tags, considering the appropriate Unreal Engine version.
	 */
	static FGameplayTagContainer GetAbilityTags(const FAbilityEndedData& AbilityEndedData);
	
};
