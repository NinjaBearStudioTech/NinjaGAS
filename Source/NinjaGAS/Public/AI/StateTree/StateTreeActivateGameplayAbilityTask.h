// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "StateTreeTaskBase.h"
#include "StateTreeActivateGameplayAbilityTask.generated.h"

USTRUCT()
struct FStateTreeActivateGameplayAbilityTaskInstanceData
{
	
	GENERATED_BODY()

	/** Gameplay Tags used to activate the ability. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FGameplayTagContainer AbilityActivationTags = FGameplayTagContainer::EmptyContainer;
	
	/** Spec for the Gameplay Ability that has Ended. */
	UPROPERTY(EditAnywhere, Category = Output)
	FGameplayAbilitySpecHandle AbilitySpecHandle = FGameplayAbilitySpecHandle();

	/** The last ability that has ended. */
	TWeakObjectPtr<UGameplayAbility> AbilityThatEnded;

	/** Owner Ability System Component. */
	TWeakObjectPtr<UAbilitySystemComponent> AbilityComponent;
	
	/** Informs when the ability has ended. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bAbilityHasEnded = false;
	
	/** Informs if this ability was cancelled. */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bAbilityWasCancelled = false;

	/** Delegate Handle provided by the ASC. */
	FDelegateHandle AbilityEndedDelegateHandle;

	/**
	 * Checks that a given "Ability Ended" data contains the ability tracked by this struct.
	 * The comparison happens using the Activation Tags configured in the task data.
	 */
	bool CheckAbilityThatHasEnded(const FAbilityEndedData& AbilityEndedData) const;

	/**
	 * Reset the bindings, keeping the outcome data (ability ended, cancellation, spec, etc.). 
	 * This will clear the handle and ability system component, which are not needed anymore
	 */
	void ResetBindings();
	
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

	/** If set to true, finishes the task/state once the ability ends. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bShouldFinishStateWhenAbilityCompletes = true;
	
	/** Determines if a cancelled ability should be handled as success. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bTreatCancelledAbilityAsSuccess = false;
	
	/** If set to true, cancels the ability when the state changes. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	bool bShouldCancelAbilityWhenStateFinishes = false;

	/**
	 * Retrieves the Ability System component from the context owner.
	 * It will attempt to find the ASC for an owner that is an AI Controller or Pawn.
	 */
	static UAbilitySystemComponent* GetAbilitySystemComponent(const FStateTreeExecutionContext& Context);
	
	/**
	 * Activates the ability requested in the context.
	 *
	 * @param Context				Context providing activation info.
	 * @return						True if the activation is successful.
	 */
	virtual EStateTreeRunStatus ActivateAbility(const FStateTreeExecutionContext& Context) const;

#if WITH_EDITOR
public:
	
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
	
#endif
};
