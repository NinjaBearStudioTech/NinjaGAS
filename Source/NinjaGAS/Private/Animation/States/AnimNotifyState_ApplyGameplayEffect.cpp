// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/States/AnimNotifyState_ApplyGameplayEffect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_ApplyGameplayEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp) || !IsValid(GameplayEffectClass))
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

	FGameplayEffectContextHandle EffectContext = AbilitySystem->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(GameplayEffectClass, EffectLevel, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	RemoveGameplayEffect(MeshComp);

	const FActiveGameplayEffectHandle ActiveHandle = AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	if (ActiveHandle.IsValid())
	{
		ActiveEffectHandles.Add(MeshComp, FAppliedGameplayEffectInfo{ AbilitySystem, ActiveHandle });
	}
}

void UAnimNotifyState_ApplyGameplayEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	RemoveGameplayEffect(MeshComp);
}

void UAnimNotifyState_ApplyGameplayEffect::RemoveGameplayEffect(const USkeletalMeshComponent* MeshComp)
{
	if (!IsValid(MeshComp))
	{
		return;
	}

	const FAppliedGameplayEffectInfo* ActiveEffect = ActiveEffectHandles.Find(MeshComp);
	if (!ActiveEffect)
	{
		return;
	}

	if (ActiveEffect->AbilitySystem.IsValid() && ActiveEffect->Handle.IsValid())
	{
		ActiveEffect->AbilitySystem->RemoveActiveGameplayEffect(ActiveEffect->Handle);
	}

	ActiveEffectHandles.Remove(MeshComp);
}