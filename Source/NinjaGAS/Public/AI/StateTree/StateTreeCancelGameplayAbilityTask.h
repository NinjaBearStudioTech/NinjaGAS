// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "UObject/Object.h"
#include "StateTreeCancelGameplayAbilityTask.generated.h"

class AAIController;
class UAbilitySystemComponent;

USTRUCT()
struct FStateTreeCancelGameplayAbilityTaskInstanceData
{
	GENERATED_BODY()

	/** The AI Controller that is running the State Tree. */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> AIController = nullptr;

	/** If present, abilities with these tags will be cancelled. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagContainer CancelAbilityWithTags = FGameplayTagContainer::EmptyContainer;

	/** If absent, abilities without these tags will be cancelled. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagContainer CancelAbilityWithoutTags = FGameplayTagContainer::EmptyContainer;
	
};

/**
 * Cancels a Gameplay Ability and completes the state when done.
 */
USTRUCT(meta = (DisplayName = "Cancel Gameplay Ability", Category = "GAS"))
struct NINJAGAS_API FStateTreeCancelGameplayAbilityTask : public FStateTreeTaskCommonBase
{
	
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCancelGameplayAbilityTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:

	/**
	 * Cancels the ability requested in the context.
	 *
	 * @param Context				Context providing activation info.
	 * @param AbilityComponent		Ability System Component that will cancel the ability.
	 * @return						True if the cancellation was successful.
	 */
	virtual EStateTreeRunStatus CancelAbilities(const FStateTreeExecutionContext& Context, UAbilitySystemComponent* AbilityComponent) const;
	
};
