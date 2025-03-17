// Ninja Bear Studio Inc., all rights reserved.
#include "AI/StateTree/StateTreeCancelGameplayAbilityTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "VisualLogger/VisualLogger.h"

EStateTreeRunStatus FStateTreeCancelGameplayAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeCancelGameplayAbilityTask failed since AIController is missing."));
		return EStateTreeRunStatus::Failed;
	}

	const APawn* Pawn = InstanceData.AIController->GetPawn();
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!AbilityComponent)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeCancelGameplayAbilityTask failed since Pawn does not have an ASC."));
		return EStateTreeRunStatus::Failed;
	}

	return CancelAbilities(Context, AbilityComponent);
}

EStateTreeRunStatus FStateTreeCancelGameplayAbilityTask::CancelAbilities(const FStateTreeExecutionContext& Context, UAbilitySystemComponent* AbilityComponent) const
{
	if (!IsValid(AbilityComponent))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeCancelGameplayAbilityTask failed since an invalid ASC was received."));
		return EStateTreeRunStatus::Failed;
	}

	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	const FGameplayTagContainer CancelAbilitiesWithTags = InstanceData.CancelAbilityWithTags;
	const FGameplayTagContainer CancelAbilitiesWithoutTags = InstanceData.CancelAbilityWithoutTags;

	AbilityComponent->CancelAbilities(
		CancelAbilitiesWithTags.IsValid() ? &CancelAbilitiesWithTags : nullptr,
		CancelAbilitiesWithoutTags.IsValid() ? &CancelAbilitiesWithoutTags : nullptr
	);

	return EStateTreeRunStatus::Succeeded;
}
