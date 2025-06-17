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

bool FStateTreeActivateGameplayAbilityTaskInstanceData::CheckAbilityThatHasEnded(const FAbilityEndedData& AbilityEndedData) const
{
	FGameplayTagContainer AbilityThatEndedTags = FGameplayTagContainer();
	if (!IsValid(AbilityEndedData.AbilityThatEnded) || !AbilityEndedData.AbilitySpecHandle.IsValid())
	{
		return false;
	}
	
#if ENGINE_MINOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
	AbilityThatEndedTags.AppendTags(Data.AbilityThatEnded->AbilityTags);
#else
	AbilityThatEndedTags.AppendTags(AbilityEndedData.AbilityThatEnded->GetAssetTags());
#endif

	// Check that the ability that ended has all activation tags that we are targeting.
	return AbilityThatEndedTags.HasAll(AbilityActivationTags);
}

void FStateTreeActivateGameplayAbilityTaskInstanceData::ResetBindings()
{
	if (AbilityComponent.IsValid() && AbilityEndedDelegateHandle.IsValid())
	{
		// Remove the binding from the ASC, before removing the pointers.
		AbilityComponent->OnAbilityEnded.Remove(AbilityEndedDelegateHandle);
	}
	
	AbilityComponent.Reset();
	AbilityEndedDelegateHandle.Reset();
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponent(Context);
	if (!AbilityComponent)
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask::EnterState failed since Pawn does not have an ASC."));
		return EStateTreeRunStatus::Failed;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bAbilityHasEnded = false;
	InstanceData.AbilitySpecHandle = FGameplayAbilitySpecHandle();
	InstanceData.AbilityComponent = AbilityComponent;

	return ActivateAbility(Context);
}

UAbilitySystemComponent* FStateTreeActivateGameplayAbilityTask::GetAbilitySystemComponent(const FStateTreeExecutionContext& Context)
{
	const AActor* OwnerActor = Cast<AActor>(Context.GetOwner());
	if (!IsValid(OwnerActor))
	{
		return nullptr;
	}

	if (OwnerActor->IsA<AAIController>())
	{
		const APawn* OwnerPawn = Cast<AAIController>(OwnerActor)->GetPawn();
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerPawn);
	}

	// Simply try to obtain the ASC directly from the actor (probably a pawn/character).
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
}

EStateTreeRunStatus FStateTreeActivateGameplayAbilityTask::ActivateAbility(const FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.AbilityComponent.IsValid())
	{
		UE_VLOG(Context.GetOwner(), LogStateTree, Error, TEXT("FStateTreeActivateGameplayAbilityTask::ActivateAbility failed, context has an invalid ASC."));
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* AbilityComponent = InstanceData.AbilityComponent.Get();
	const FDelegateHandle Handle = AbilityComponent->OnAbilityEnded.AddLambda([InstanceDataRef = Context.GetInstanceDataStructRef(*this)](const FAbilityEndedData& AbilityEndedData) mutable
	{
		if (InstanceDataRef.IsValid())
		{
			FInstanceDataType* InstanceDataPtr = InstanceDataRef.GetPtr();
			if (InstanceDataPtr && InstanceDataPtr->CheckAbilityThatHasEnded(AbilityEndedData))
			{
				InstanceDataPtr->bAbilityHasEnded = true;
				InstanceDataPtr->AbilityThatEnded = AbilityEndedData.AbilityThatEnded;
				InstanceDataPtr->AbilitySpecHandle = AbilityEndedData.AbilitySpecHandle;
				InstanceDataPtr->bAbilityWasCancelled = AbilityEndedData.bWasCancelled;
				InstanceDataPtr->ResetBindings();
			}
		}
	});

	bool bActivated = false; 
	if (Handle.IsValid())
	{
		InstanceData.AbilityEndedDelegateHandle = Handle;

		const FGameplayTagContainer AbilityTriggerTags = Context.GetInstanceData(*this).AbilityActivationTags;
		UE_VLOG(Context.GetOwner(), LogStateTree, Log, TEXT("FStateTreeActivateGameplayAbilityTask will activate ability using %s."), *AbilityTriggerTags.ToStringSimple());
		
		bActivated = AbilityComponent->TryActivateAbilitiesByTag(AbilityTriggerTags);
	}
	
	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
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
	if (InstanceData.AbilityComponent.IsValid() && InstanceData.AbilityComponent->IsValidLowLevelFast())
	{
		UAbilitySystemComponent* AbilityComponent = InstanceData.AbilityComponent.Get();

		// Delegate cleanup (only if actually bound).
		if (InstanceData.AbilityEndedDelegateHandle.IsValid())
		{
			AbilityComponent->OnAbilityEnded.Remove(InstanceData.AbilityEndedDelegateHandle);
			InstanceData.AbilityEndedDelegateHandle.Reset();
		}

		// Cancel ability if it's still running and we are configured to do so.
		if (!InstanceData.bAbilityHasEnded && bShouldCancelAbilityWhenStateFinishes)
		{
			UE_VLOG(Context.GetOwner(), LogStateTree, Log,
				TEXT("FStateTreeActivateGameplayAbilityTask forcing cancellation of ability activated by %s."),
				*InstanceData.AbilityActivationTags.ToStringSimple());

			if (InstanceData.AbilitySpecHandle.IsValid())
			{
				AbilityComponent->CancelAbilityHandle(InstanceData.AbilitySpecHandle);
			}
			else
			{
				AbilityComponent->CancelAbilities(&InstanceData.AbilityActivationTags);
			}
		}		
	}
	
	InstanceData.AbilityEndedDelegateHandle.Reset();
	InstanceData.AbilityThatEnded.Reset();
	InstanceData.AbilityComponent.Reset();
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
	
	const FText Format = (Formatting == EStateTreeNodeFormatting::RichText)
		? NSLOCTEXT("StateTree", "AbilityRich", "<b>Activate Ability</> {AbilityTags}")
		: NSLOCTEXT("StateTree", "Ability", "Activate Ability {AbilityTags}");

	return FText::FormatNamed(Format,
		TEXT("AbilityTags"), Value);	
}
#endif
