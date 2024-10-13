// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AI/BehaviorTree/BTTask_ActivateGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Class.h"

UBTTask_ActivateGameplayAbility::UBTTask_ActivateGameplayAbility()
{
    NodeName = "Activate Gameplay Ability";
	bNotifyTick = false;
	bNotifyTaskFinished = true;
	bWaitForAbilityToEnd = true;
    bTreatCancelledAbilityAsSuccess = false;
	ActivationMode = EAgentAbilityActivationMode::GameplayTags;
    AbilityClass = nullptr;
    AbilityTriggerTags = FGameplayTagContainer::EmptyContainer;
	
	const TSubclassOf<UGameplayAbility> GameplayAbilityClass = UGameplayAbility::StaticClass();
	AbilityClassKey.AddClassFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, AbilityClassKey), GameplayAbilityClass);
}

void UBTTask_ActivateGameplayAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
    Super::InitializeFromAsset(Asset);

    if (ActivationMode == EAgentAbilityActivationMode::BlackboardClass)
    {
        const UBlackboardData* BBAsset = GetBlackboardAsset();
        if (ensure(BBAsset))
        {
            AbilityClassKey.ResolveSelectedKey(*BBAsset);
        }
    }
}

void UBTTask_ActivateGameplayAbility::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
    EBTMemoryInit::Type InitType) const
{
    FExecuteGameplayAbilityMemory* MyMemory = CastInstanceNodeMemory<FExecuteGameplayAbilityMemory>(NodeMemory);
    check(MyMemory);
	
    MyMemory->LastTimeFinished = 0.f;
    MyMemory->AbilityCallbackDelegateHandle.Reset();
}

EBTNodeResult::Type UBTTask_ActivateGameplayAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FExecuteGameplayAbilityMemory* MyMemory = CastInstanceNodeMemory<FExecuteGameplayAbilityMemory>(NodeMemory);
	check(MyMemory);

    const AAIController* BotController = OwnerComp.GetAIOwner();
    if (IsValid(BotController))
    {
        bool bActivated = false;
        
        UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(BotController->GetPawn());
        if (IsValid(AbilityComponent))
        {
            switch(ActivationMode)
            {
            case EAgentAbilityActivationMode::BlackboardClass:
                {
                    AbilityClass = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Class>(AbilityClassKey.GetSelectedKeyID());
                }
            case EAgentAbilityActivationMode::AbilityClass:
                {
                    if (bWaitForAbilityToEnd)
                    {
                        MyMemory->AbilityCallbackDelegateHandle = AbilityComponent->OnAbilityEnded.AddUObject(
                        	this, &ThisClass::OnAbilityEnded_Class, &OwnerComp);
                    }
                    
                    bActivated = AbilityComponent->TryActivateAbilityByClass(AbilityClass);
                    break;
                }
            case EAgentAbilityActivationMode::GameplayTags:
                {
                    if (bWaitForAbilityToEnd)
                    {
                        MyMemory->AbilityCallbackDelegateHandle = AbilityComponent->OnAbilityEnded.AddUObject(
                        	this, &ThisClass::OnAbilityEnded_Tags, &OwnerComp);
                    }
                    
                    bActivated = AbilityComponent->TryActivateAbilitiesByTag(AbilityTriggerTags);
                    break;
                }
            }
        }

        const EBTNodeResult::Type SuccessResultValue = bWaitForAbilityToEnd ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
        return bActivated ? SuccessResultValue : EBTNodeResult::Failed; 
    }

	return EBTNodeResult::Failed;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UBTTask_ActivateGameplayAbility::OnAbilityEnded_Class(const FAbilityEndedData& Data, UBehaviorTreeComponent* OwnerComp)
{
    if (Data.AbilityThatEnded->IsA(AbilityClass))
    {
        HandleFinishedAbility(Data, OwnerComp);
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UBTTask_ActivateGameplayAbility::OnAbilityEnded_Tags(const FAbilityEndedData& Data, UBehaviorTreeComponent* OwnerComp)
{
    if (Data.AbilityThatEnded->AbilityTags.HasAll(AbilityTriggerTags))
    {
        HandleFinishedAbility(Data, OwnerComp);
    }
}

void UBTTask_ActivateGameplayAbility::HandleFinishedAbility(const FAbilityEndedData& Data,
    UBehaviorTreeComponent* OwnerComp) const
{
    const AAIController* BotController = OwnerComp->GetAIOwner();
    const AActor* BehaviorOwner = IsValid(BotController) ? BotController->GetPawn() : nullptr;
    const AActor* AbilityOwner = Data.AbilityThatEnded->GetAvatarActorFromActorInfo();

    if (AbilityOwner == BehaviorOwner)
    {
        const EBTNodeResult::Type CancelResult = bTreatCancelledAbilityAsSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
        const EBTNodeResult::Type TaskResult = Data.bWasCancelled ? CancelResult : EBTNodeResult::Succeeded;    
        FinishLatentTask(*OwnerComp, TaskResult);
    }
}

void UBTTask_ActivateGameplayAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
    const EBTNodeResult::Type TaskResult)
{
    FExecuteGameplayAbilityMemory* MyMemory = CastInstanceNodeMemory<FExecuteGameplayAbilityMemory>(NodeMemory);
    check(MyMemory);

    if (TaskResult != EBTNodeResult::InProgress && MyMemory->AbilityCallbackDelegateHandle.IsValid())
    {
        const AAIController* BotController = OwnerComp.GetAIOwner();
        if (IsValid(BotController))
        {
            UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(BotController->GetPawn());
            if (IsValid(AbilityComponent))
            {
                AbilityComponent->OnAbilityEnded.Remove(MyMemory->AbilityCallbackDelegateHandle);
                MyMemory->AbilityCallbackDelegateHandle.Reset();
            }
        }
    }
}

uint16 UBTTask_ActivateGameplayAbility::GetInstanceMemorySize() const
{
    return sizeof(FExecuteGameplayAbilityMemory);
}

FString UBTTask_ActivateGameplayAbility::GetStaticDescription() const
{
    const FString KeyDesc = AbilityClassKey.IsSet() ? AbilityClassKey.SelectedKeyName.ToString() : TEXT("invalid");
    const UEnum* ActivationEnum = StaticEnum<EAgentAbilityActivationMode>();
    const int32 EnumValue = static_cast<int32>(ActivationMode);

	FString Note = TEXT("");
	if (ActivationMode == EAgentAbilityActivationMode::BlackboardClass)
	{
		Note = AbilityClassKey.IsSet() ? AbilityClassKey.SelectedKeyName.ToString() : TEXT("invalid");
	}
	else if (ActivationMode == EAgentAbilityActivationMode::AbilityClass)
	{
		Note = *GetNameSafe(AbilityClass);
	}
	else if (ActivationMode == EAgentAbilityActivationMode::AbilityClass)
	{
		Note = AbilityTriggerTags.ToStringSimple();
	}
	
    return FString::Printf(TEXT("%s:\n%s : %s")
        , *Super::GetStaticDescription()
        , *ActivationEnum->GetDisplayNameTextByIndex(EnumValue).ToString()
        , *Note);
}
