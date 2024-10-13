// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AI/Types/EAgentAbilityActivationMode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CancelGameplayAbility.generated.h"

class UGameplayAbility;

/**
 * Cancels a gameplay ability.
 */
UCLASS(DisplayName = "Cancel Gameplay Ability", Category = "Ninja GAS")
class NINJAGAS_API UBTTask_CancelGameplayAbility : public UBTTaskNode
{
    
    GENERATED_BODY()

public:

    /** Determines how we want to cancel the ability. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Cancellation")
    EAgentAbilityActivationMode CancellationMode;
    
    /** The blackboard location used to retrieve the ability class. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Cancellation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::BlackboardClass", EditConditionHides))
    FBlackboardKeySelector AbilityClassKey;

    /** Ability Class directly provided to this task, used for cancellation. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Cancellation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::AbilityClass", EditConditionHides))
    TSubclassOf<UGameplayAbility> AbilityClass;

    /** Gameplay Tags used to cancel the ability. All Tags must be present. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Cancellation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::GameplayTags", EditConditionHides))
    FGameplayTagContainer AbilityTargetTags;

    /** If cancelling via tags, this is a filter that prevents an ability to be cancelled. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Cancellation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::GameplayTags", EditConditionHides))
    FGameplayTagContainer CancelFilterTags;
    
    UBTTask_CancelGameplayAbility();

protected:

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;
    
};
