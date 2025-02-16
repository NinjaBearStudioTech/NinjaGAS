// Ninja Bear Studio Inc., all rights reserved.
#include "AI/StateTree/StateTreeActivateGameplayAbilityTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Abilities/GameplayAbility.h"
#include "Runtime/Launch/Resources/Version.h"

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AIController)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask failed since AIController is missing."));
		return EStateTreeRunStatus::Failed;
	}

	const APawn* Pawn = InstanceData.AIController->GetPawn();
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);
	if (!AbilityComponent)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask failed since Pawn does not have an ASC."));
		return EStateTreeRunStatus::Failed;
	}

	return ActivateAbility(Context, AbilityComponent);
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::ActivateAbility(const FStateTreeExecutionContext& Context, UAbilitySystemComponent* AbilityComponent) const
{
	if (!IsValid(AbilityComponent))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask failed since an invalid ASC was received."));
		return EStateTreeRunStatus::Failed;
	}

	const FDelegateHandle Handle = AbilityComponent->OnAbilityEnded.AddLambda([this, InstanceDataRef = Context.GetInstanceDataStructRef(*this)](const FAbilityEndedData& AbilityEndedData) mutable
	{
		FInstanceDataType* InstanceDataPtr = InstanceDataRef.GetPtr();
		if (InstanceDataPtr && CheckAbilityThatHasEnded(InstanceDataPtr, AbilityEndedData))
		{
			InstanceDataPtr->bAbilityHasEnded = true;
			InstanceDataPtr->AbilitySpec = AbilityEndedData.AbilitySpecHandle;
			InstanceDataPtr->bAbilityWasCancelled = AbilityEndedData.bWasCancelled;
		}
	});

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.AbilityEndedHandle = Handle;
	
	const FGameplayTagContainer AbilityTriggerTags = Context.GetInstanceData(*this).AbilityActivationTags;
	UE_VLOG(Context.GetOwner(), LogStateTree, Log, TEXT("FStateTreeActivateGameplayAbilityTask will activate ability using %s."), *AbilityTriggerTags.ToStringSimple());
	
	AbilityComponent->TryActivateAbilitiesByTag(AbilityTriggerTags);
	return EStateTreeRunStatus::Running;
}

bool FStateTreeActivateGameplayAbilityTask::CheckAbilityThatHasEnded(const FInstanceDataType* InstanceData, const FAbilityEndedData& AbilityEndedData) const
{
	FGameplayTagContainer AbilityThatEndedTags = FGameplayTagContainer();

#if ENGINE_MINOR_VERSION < 5
	AbilityThatEndedTags.AppendTags(Data.AbilityThatEnded->AbilityTags);
#else
	AbilityThatEndedTags.AppendTags(AbilityEndedData.AbilityThatEnded->GetAssetTags());
#endif

	const FGameplayTagContainer AbilityActivationTags = InstanceData->AbilityActivationTags;
	return AbilityThatEndedTags.HasAll(AbilityActivationTags);
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (InstanceData.bAbilityHasEnded)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Log,
			TEXT("FStateTreeActivateGameplayAbilityTask has received ability status: %s: %s."),
			*InstanceData.AbilityActivationTags.ToStringSimple(),
			InstanceData.bAbilityWasCancelled ? TEXT("been cancelled") : TEXT("ended"));

		// Check what is the correct status, based on the parameters assigned to the instance.
		if (InstanceData.bShouldFinishStateWhenAbilityCompletes)
		{
			Status = InstanceData.bAbilityWasCancelled || !InstanceData.bTreatCancelledAbilityAsSuccess
				? EStateTreeRunStatus::Failed
				: EStateTreeRunStatus::Succeeded;
		}
	}
	
	return Status;
}

void FStateTreeActivateGameplayAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!IsValid(InstanceData.AIController))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Warning, TEXT("FStateTreeActivateGameplayAbilityTask AIController is invalid in ExitState."));
		return;
	}

	const APawn* Pawn = InstanceData.AIController->GetPawn();
	UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn);

	if (IsValid(AbilityComponent) && InstanceData.AbilityEndedHandle.IsValid())
	{
		// Remove the delegate first to avoid notifications when forcibly ending the ability.
		AbilityComponent->OnAbilityEnded.Remove(InstanceData.AbilityEndedHandle);
		InstanceData.AbilityEndedHandle.Reset();

		// Force ability cancellation if it hasn't already ended and should be stopped when the state finishes.
		if (!InstanceData.bAbilityHasEnded && InstanceData.bShouldCancelAbilityWhenStateFinishes)
		{
			UE_VLOG(Context.GetOwner(), LogStateTree, Log,
				TEXT("FStateTreeActivateGameplayAbilityTask forcing cancellation of ability activated by %s."),
				*InstanceData.AbilityActivationTags.ToStringSimple());

			if (InstanceData.AbilitySpec.IsValid())
			{
				// Unlikely to have a valid handle here. But if we do, then use it.
				AbilityComponent->CancelAbilityHandle(InstanceData.AbilitySpec);	
			}
			else
			{
				const FGameplayTagContainer AbilityThatEndedTags = InstanceData.AbilityActivationTags;
				AbilityComponent->CancelAbilities(&AbilityThatEndedTags);
			}
		}
	}
}