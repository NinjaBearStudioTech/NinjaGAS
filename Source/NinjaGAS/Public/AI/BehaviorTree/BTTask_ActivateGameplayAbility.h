// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AI/Types/EAgentAbilityActivationMode.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ActivateGameplayAbility.generated.h"

class UAbilitySystemComponent;
class UGameplayAbility;

struct FExecuteGameplayAbilityMemory
{
    /** The last time we finished our ability. */
    float LastTimeFinished;
	
    /** Delegate handle for our ability execution. */
    FDelegateHandle AbilityCallbackDelegateHandle;
};

/**
 * Activates a gameplay ability using the request activation mode.
 */
UCLASS(DisplayName = "Activate Gameplay Ability", Category = "Ninja GAS")
class NINJAGAS_API UBTTask_ActivateGameplayAbility : public UBTTaskNode
{
    
    GENERATED_BODY()

public:

    /** Determines how we want to activate the ability. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation")
    EAgentAbilityActivationMode ActivationMode;
    
    /** The blackboard location used to retrieve the ability class. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::BlackboardClass", EditConditionHides))
    FBlackboardKeySelector AbilityClassKey;

    /** Ability Class directly provided to this task, used for activation. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::AbilityClass", EditConditionHides))
    TSubclassOf<UGameplayAbility> AbilityClass;

    /** Gameplay Tags used to activate the ability. All Tags must be present. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation", meta = (EditCondition = "ActivationMode == EAgentAbilityActivationMode::GameplayTags", EditConditionHides))
    FGameplayTagContainer AbilityTriggerTags;
    
    /** Should we wait for the ability to end? */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation")
    bool bWaitForAbilityToEnd;

    /** Determines if a cancelled ability should be handled as success. */
    UPROPERTY(EditAnywhere, Category = "Gameplay Ability Activation")
    bool bTreatCancelledAbilityAsSuccess;
    
    UBTTask_ActivateGameplayAbility();
    
protected:

    virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
    virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
    virtual uint16 GetInstanceMemorySize() const override;
    virtual FString GetStaticDescription() const override;
    
    /**
     * Effectively handles an ability that has finished.
     *
     * @param Data			    Information the ability that finished executing.
     * @param OwnerComp		    Behavior Tree component that owns this task.
     */
    UFUNCTION()
    void HandleFinishedAbility(const FAbilityEndedData& Data, UBehaviorTreeComponent* OwnerComp) const;
    
    /**
     * Callback triggered when the ability has finished executing.
     *
     * It uses an Ability Class as a reference to check if the ability that ended is the one expected.
     *
     * @param Data			    Information the ability that finished executing.
     * @param OwnerComp		    Behavior Tree component that owns this task.
     */
    void OnAbilityEnded_Class(const FAbilityEndedData& Data, UBehaviorTreeComponent* OwnerComp);

    /**
     * Callback triggered when the ability has finished executing.
     *
     * It uses a container as a reference to check if the ability that ended is the one expected.
     *
     * @param Data			    Information the ability that finished executing.
     * @param OwnerComp		    Behavior Tree component that owns this task.
     */
    UFUNCTION()
    void OnAbilityEnded_Tags(const FAbilityEndedData& Data, UBehaviorTreeComponent* OwnerComp);
    
};
