// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Templates/SubclassOf.h"
#include "AnimNotifyState_ApplyGameplayEffect.generated.h"

class UGameplayEffect;

/**
 * Applies and removes a gameplay effect.
 */
UCLASS(meta = (DisplayName = "Apply Gameplay Effect"))
class NINJAGAS_API UAnimNotifyState_ApplyGameplayEffect : public UAnimNotifyState
{
	
	GENERATED_BODY()
	
public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Effect")
	float EffectLevel = 1.f;

	void RemoveGameplayEffect(const USkeletalMeshComponent* MeshComp);
	
private:

	struct FAppliedGameplayEffectInfo
	{
		TWeakObjectPtr<UAbilitySystemComponent> AbilitySystem;
		FActiveGameplayEffectHandle Handle;
	};

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FAppliedGameplayEffectInfo> ActiveEffectHandles;
};
