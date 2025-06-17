// Ninja Bear Studio Inc., all rights reserved.
#include "AI/StateTree/StateTreeAbilityTrackerEvaluator.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "StateTreeExecutionContext.h"
#include "Runtime/Launch/Resources/Version.h"

void FStateTreeAbilityTrackerEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponent(Context);
	if (!IsValid(AbilityComponent))
	{
		return;
	}

	const FDelegateHandle Handle = AbilityComponent->OnAbilityEnded.AddLambda([this, InstanceDataRef = Context.GetInstanceDataStructRef(*this)](const FAbilityEndedData& AbilityEndedData) mutable
	{
		FInstanceDataType* InstanceDataPtr = InstanceDataRef.GetPtr();
		if (InstanceDataPtr && IsAbilityRelevant(InstanceDataPtr, AbilityEndedData))
		{
			if (IsSameAbility(InstanceDataPtr, AbilityEndedData))
			{
				if (AbilityEndedData.bWasCancelled)
				{
					InstanceDataPtr->ConsecutiveExecutions = 0;
					InstanceDataPtr->LastAbilityTags.Reset();
				}
				else
				{
					InstanceDataPtr->ConsecutiveExecutions++;	
				}
			}
			else
			{
				// Start tracking a new ability. First successful execution counts as 1.
				InstanceDataPtr->ConsecutiveExecutions = AbilityEndedData.bWasCancelled ? 0 : 1;
				InstanceDataPtr->LastAbilityTags = GetAbilityTags(AbilityEndedData);
			}
		}
	});

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.AbilityEndedHandle = Handle;	
}

bool FStateTreeAbilityTrackerEvaluator::IsAbilityRelevant(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData)
{
	if (!HasValidData(InstanceDataPtr, AbilityEndedData))
	{
		// Invalid data, fail fast.
		return false;
	}

	const FGameplayTagQuery& AbilityFilterQuery = InstanceDataPtr->AbilityFilterQuery;
	if (AbilityFilterQuery.IsEmpty())
	{
		// No filter, the ability is eligible.
		return true;
	}

	const FGameplayTagContainer AbilityTags = GetAbilityTags(AbilityEndedData);
	return AbilityFilterQuery.Matches(AbilityTags);
}

bool FStateTreeAbilityTrackerEvaluator::IsSameAbility(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData)
{
	if (!HasValidData(InstanceDataPtr, AbilityEndedData))
	{
		// Invalid data, fail fast.
		return false;
	}

	const FGameplayTagContainer AbilityTags = GetAbilityTags(AbilityEndedData);
	const FGameplayTagContainer LastAbilityTags = InstanceDataPtr->LastAbilityTags;
	
	return AbilityTags == LastAbilityTags;
}

FGameplayTagContainer FStateTreeAbilityTrackerEvaluator::GetAbilityTags(const FAbilityEndedData& AbilityEndedData)
{
	// Hopefully this has been checked before, but let's play safe in here too.
	if (!IsValid(AbilityEndedData.AbilityThatEnded))
	{
		return FGameplayTagContainer();
	}
	
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 5
	return AbilityEndedData.AbilityThatEnded->AbilityTags;
#elif ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 5
	return AbilityEndedData.AbilityThatEnded->GetAssetTags();
#endif
}

void FStateTreeAbilityTrackerEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	UAbilitySystemComponent* AbilityComponent = GetAbilitySystemComponent(Context);
	if (!IsValid(AbilityComponent))
	{
		return;
	}

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.AbilityEndedHandle.IsValid())
	{
		AbilityComponent->OnAbilityEnded.Remove(InstanceData.AbilityEndedHandle);
		InstanceData.AbilityEndedHandle.Reset();
	}
}

UAbilitySystemComponent* FStateTreeAbilityTrackerEvaluator::GetAbilitySystemComponent(const FStateTreeExecutionContext& Context)
{
	const AAIController* Owner = Cast<AAIController>(Context.GetOwner());
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner->GetPawn());
}

bool FStateTreeAbilityTrackerEvaluator::HasValidData(const FInstanceDataType* InstanceDataPtr, const FAbilityEndedData& AbilityEndedData)
{
	return InstanceDataPtr && IsValid(AbilityEndedData.AbilityThatEnded);
}
