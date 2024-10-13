// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AI/BehaviorTree/BTTask_CancelGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Class.h"

UBTTask_CancelGameplayAbility::UBTTask_CancelGameplayAbility()
{
    NodeName = "Cancel Gameplay Ability";
    bNotifyTick = false;
    bNotifyTaskFinished = true;
    CancellationMode = EAgentAbilityActivationMode::BlackboardClass;
    AbilityClass = nullptr;
    AbilityTargetTags = FGameplayTagContainer::EmptyContainer;
    CancelFilterTags = FGameplayTagContainer::EmptyContainer;
	
    const TSubclassOf<UGameplayAbility> GameplayAbilityClass = UGameplayAbility::StaticClass();
    AbilityClassKey.AddClassFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CancelGameplayAbility, AbilityClassKey), GameplayAbilityClass);
}

void UBTTask_CancelGameplayAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (CancellationMode == EAgentAbilityActivationMode::BlackboardClass)
    {
        const UBlackboardData* BBAsset = GetBlackboardAsset();
        if (ensure(BBAsset))
        {
            AbilityClassKey.ResolveSelectedKey(*BBAsset);
        }
    }
}

EBTNodeResult::Type UBTTask_CancelGameplayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    const AAIController* BotController = OwnerComp.GetAIOwner();
    if (IsValid(BotController))
    {
        bool bCancelled = false;
        
        UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(BotController->GetPawn());
        if (IsValid(AbilityComponent))
        {
            switch(CancellationMode)
            {
            case EAgentAbilityActivationMode::BlackboardClass:
                {
                    // Load the class and allow the next case to cancel it from the Ability Spec (no break clause).
                    AbilityClass = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Class>(AbilityClassKey.GetSelectedKeyID());
                }
            case EAgentAbilityActivationMode::AbilityClass:
                {
                    const FGameplayAbilitySpec* Spec = AbilityComponent->FindAbilitySpecFromClass(AbilityClass);
                    if (Spec != nullptr && Spec->Handle.IsValid())
                    {
                        AbilityComponent->CancelAbilityHandle(Spec->Handle);
                        bCancelled = true;
                    }
                    break;
                }
            case EAgentAbilityActivationMode::GameplayTags:
                {
                    AbilityComponent->CancelAbilities(&AbilityTargetTags, &CancelFilterTags);
                    bCancelled = true;
                    break;
                }
            }
        }

        return bCancelled ? EBTNodeResult::Succeeded : EBTNodeResult::Failed; 
    }
	
	return EBTNodeResult::Failed;
}

FString UBTTask_CancelGameplayAbility::GetStaticDescription() const
{
    const FString KeyDesc = AbilityClassKey.IsSet() ? AbilityClassKey.SelectedKeyName.ToString() : TEXT("invalid");
    const UEnum* ActivationEnum = StaticEnum<EAgentAbilityActivationMode>();
    const int32 EnumValue = static_cast<int32>(CancellationMode);
	
    return FString::Printf(TEXT("%s:\n%s (by %s)")
        , *Super::GetStaticDescription()
        , *KeyDesc
        , *ActivationEnum->GetDisplayNameTextByIndex(EnumValue).ToString());
}
