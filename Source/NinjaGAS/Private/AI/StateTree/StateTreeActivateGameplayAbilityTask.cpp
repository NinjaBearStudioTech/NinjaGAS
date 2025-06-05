// Ninja Bear Studio Inc., all rights reserved.
#include "AI/StateTree/StateTreeActivateGameplayAbilityTask.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Abilities/GameplayAbility.h"
#include "GameFramework/Pawn.h"
#include "Runtime/Launch/Resources/Version.h"
#include "VisualLogger/VisualLogger.h"

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
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

	InstanceData.bAbilityHasEnded = false;
	InstanceData.AbilitySpecHandle = FGameplayAbilitySpecHandle();

	return ActivateAbility(Context, AbilityComponent);
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::ActivateAbility(const FStateTreeExecutionContext& Context, UAbilitySystemComponent* AbilityComponent) const
{
	if (!IsValid(AbilityComponent))
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask failed since an invalid ASC was received."));
		return EStateTreeRunStatus::Failed;
	}

	const FDelegateHandle Handle = AbilityComponent->OnAbilityEnded.AddLambda([InstanceDataRef = Context.GetInstanceDataStructRef(*this)](const FAbilityEndedData& AbilityEndedData) mutable
	{
		FInstanceDataType* InstanceDataPtr = InstanceDataRef.GetPtr();
		if (InstanceDataPtr)
		{
			InstanceDataPtr->AbilityThatEnded = AbilityEndedData.AbilityThatEnded;
			InstanceDataPtr->AbilitySpecHandle = AbilityEndedData.AbilitySpecHandle;
			InstanceDataPtr->bAbilityWasCancelled = AbilityEndedData.bWasCancelled;
		}
	});

	bool bActivated = false; 
	if (Handle.IsValid())
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		InstanceData.AbilityEndedHandle = Handle;

		const FGameplayTagContainer AbilityTriggerTags = Context.GetInstanceData(*this).AbilityActivationTags;
		UE_VLOG(Context.GetOwner(), LogStateTree, Log, TEXT("FStateTreeActivateGameplayAbilityTask will activate ability using %s."), *AbilityTriggerTags.ToStringSimple());
		
		bActivated = AbilityComponent->TryActivateAbilitiesByTag(AbilityTriggerTags);
	}
	
	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.bAbilityHasEnded = CheckAbilityThatHasEnded(InstanceData);
	if (InstanceData.bAbilityHasEnded)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Log,
			TEXT("FStateTreeActivateGameplayAbilityTask has received ability status: %s: %s."),
			*InstanceData.AbilityActivationTags.ToStringSimple(),
			InstanceData.bAbilityWasCancelled ? TEXT("been cancelled") : TEXT("ended"));

		// Check what is the correct status, based on the parameters assigned to the instance.
		if (bShouldFinishStateWhenAbilityCompletes)
		{
			Status = InstanceData.bAbilityWasCancelled || !bTreatCancelledAbilityAsSuccess
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
		if (!InstanceData.bAbilityHasEnded && bShouldCancelAbilityWhenStateFinishes)
		{
			UE_VLOG(Context.GetOwner(), LogStateTree, Log,
				TEXT("FStateTreeActivateGameplayAbilityTask forcing cancellation of ability activated by %s."),
				*InstanceData.AbilityActivationTags.ToStringSimple());

			if (InstanceData.AbilitySpecHandle.IsValid())
			{
				// Unlikely to have a valid handle here. But if we do, then use it.
				AbilityComponent->CancelAbilityHandle(InstanceData.AbilitySpecHandle);	
			}
			else
			{
				const FGameplayTagContainer AbilityThatEndedTags = InstanceData.AbilityActivationTags;
				AbilityComponent->CancelAbilities(&AbilityThatEndedTags);
			}
		}
	}
	
	InstanceData.AbilityEndedHandle.Reset();
	InstanceData.AbilityThatEnded.Reset();
}

bool FStateTreeActivateGameplayAbilityTask::CheckAbilityThatHasEnded(const FInstanceDataType& InstanceData) const
{
	FGameplayTagContainer AbilityThatEndedTags = FGameplayTagContainer();
	if (!InstanceData.AbilityThatEnded.IsValid() || !InstanceData.AbilitySpecHandle.IsValid())
	{
		return false;
	}
	
#if ENGINE_MINOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
	AbilityThatEndedTags.AppendTags(Data.AbilityThatEnded->AbilityTags);
#else
	AbilityThatEndedTags.AppendTags(InstanceData.AbilityThatEnded->GetAssetTags());
#endif

	const FGameplayTagContainer AbilityActivationTags = InstanceData.AbilityActivationTags;
	return AbilityThatEndedTags.HasAll(AbilityActivationTags);
}

#if WITH_EDITOR
FText FStateTreeActivateGameplayAbilityTask::GetDescription(const FGuid& ID, const FStateTreeDataView InstanceDataView,
	const IStateTreeBindingLookup& BindingLookup, const EStateTreeNodeFormatting Formatting) const
{
	const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
	check(InstanceData);

	FText Value = FText::GetEmpty();

	if (InstanceData->AbilityActivationTags.IsValid())
	{
		Value = FText::Format(NSLOCTEXT("StateTree", "AbilityTags", "{0}"),
			FText::FromString(InstanceData->AbilityActivationTags.ToStringSimple()));
	}
	else
	{
		Value = NSLOCTEXT("StateTree", "EmptyTags", "Empty Tags");
	}
	
	const FText Format = (Formatting == RichText)
		? NSLOCTEXT("StateTree", "AbilityRich", "<b>Activate Ability</> {AbilityTags}")
		: NSLOCTEXT("StateTree", "Ability", "Activate Ability {AbilityTags}");

	return FText::FormatNamed(Format,
		TEXT("AbilityTags"), Value);	
}
#endif
