// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/States/AnimNotifyState_ApplyLooseGameplayTags.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayTagContainer.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ApplyLooseGameplayTags::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	const float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp) || GameplayTags.IsEmpty())
	{
		return;
	}

	const AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner);
	if (!IsValid(AbilitySystem))
	{
		return;
	}

	RemoveGameplayTags(MeshComp);

	AbilitySystem->AddLooseGameplayTags(GameplayTags);
	ActiveGameplayTags.Add(MeshComp, FAppliedLooseGameplayTagsInfo{ AbilitySystem, GameplayTags });
}

void UAnimNotifyState_ApplyLooseGameplayTags::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	RemoveGameplayTags(MeshComp);
}

void UAnimNotifyState_ApplyLooseGameplayTags::RemoveGameplayTags(const USkeletalMeshComponent* MeshComp)
{
	if (!IsValid(MeshComp))
	{
		return;
	}

	const FAppliedLooseGameplayTagsInfo* ActiveTags = ActiveGameplayTags.Find(MeshComp);
	if (!ActiveTags)
	{
		return;
	}

	if (ActiveTags->AbilitySystem.IsValid() && !ActiveTags->Tags.IsEmpty())
	{
		ActiveTags->AbilitySystem->RemoveLooseGameplayTags(ActiveTags->Tags);
	}

	ActiveGameplayTags.Remove(MeshComp);
}