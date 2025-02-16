// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StateTreeTaskBase.h"
#include "StateTreeActivateGameplayAbilityTask.generated.h"

class AAIController;

USTRUCT()
struct FStateTreeActivateGameplayAbilityTaskInstanceData
{
	
	GENERATED_BODY()

	/** The AI Controller that is running the State Tree. */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> AIController = nullptr;

	/** Gameplay Tags used to activate the ability. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagContainer AbilityActivationTags = FGameplayTagContainer::EmptyContainer;

	/** Determines if a cancelled ability should be handled as success. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bTreatCancelledAbilityAsSuccess = true;

	/** If set to true, finishes the task/state once the ability ends. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bShouldFinishStateWhenAbilityCompletes = true;

	/** If set to true, cancels the ability when the state changes. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bShouldEndAbilityWhenStateFinishes = true;
	
	/** Spec for the Gameplay Ability that has Ended. */
	UPROPERTY(EditAnywhere, Category = Output)
	FGameplayAbilitySpecHandle AbilitySpec = FGameplayAbilitySpecHandle();

	/** Informs when the ability has ended. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bAbilityHasEnded = false;
	
	/** Informs if this ability was cancelled. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bAbilityWasCancelled = false;

	/** Delegate Handle provided by the ASC. */
	FDelegateHandle AbilityEndedHandle;
};

/**
 * Activates a Gameplay Ability and completes the state when the ability finishes.
 */
USTRUCT(meta = (DisplayName = "Activate Gameplay Ability", Category = "G.A.S."))
struct NINJAGAS_API FStateTreeActivateGameplayAbilityTask : public FStateTreeTaskCommonBase
{
	
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeActivateGameplayAbilityTaskInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:

	/**
	 * Activates the ability requested in the context.
	 *
	 * @param Context				Context providing activation info.
	 * @param AbilityComponent		Ability System Component that will activate the ability.
	 * @return						True if the activation is successful.
	 */
	virtual EStateTreeRunStatus ActivateAbility(const FStateTreeExecutionContext& Context, UAbilitySystemComponent* AbilityComponent) const;

	/**
	 * Checks the ability that has ended to match if it's the one we are trying to activate.
	 *
	 * @param InstanceData			Access to the instance data backing this task.
	 * @param AbilityEndedData		Information about the Gameplay Ability that has ended.
	 * @return						True if this is the same ability that this task activated.
	 */
	virtual bool CheckAbilityThatHasEnded(const FInstanceDataType* InstanceData, const FAbilityEndedData& AbilityEndedData) const;
	
};
