// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AI/BehaviorTree/BTService_SelectGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SelectGameplayAbility::UBTService_SelectGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Select Gameplay Ability";
	bNotifyTick = false;
	bNotifyBecomeRelevant = true;

	const TSubclassOf<UGameplayAbility> GameplayAbilityClass = UGameplayAbility::StaticClass();
	AbilityClassKey.AddClassFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, AbilityClassKey), GameplayAbilityClass);
}

void UBTService_SelectGameplayAbility::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* BBAsset = GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		AbilityClassKey.ResolveSelectedKey(*BBAsset);
	}	
}

void UBTService_SelectGameplayAbility::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	UBlackboardComponent* MyBlackboard = OwnerComp.GetBlackboardComponent();
	
	if (Abilities.IsEmpty())
	{
		return;
	}

	TSubclassOf<UGameplayAbility> AbilityClass;
	if (Abilities.Num() == 1)
	{
		AbilityClass = Abilities[0]; 
	}
	else
	{
		static constexpr int32 MaxAttempts = 5;
		for (int32 Idx = 0; Idx < MaxAttempts; ++Idx)
		{
			const int32 RandomIdx = FMath::RandRange(0, Abilities.Num() - 1);
			AbilityClass = Abilities[RandomIdx];

			if (CanBeActivated(OwnerComp, AbilityClass))
			{
				break;
			}
		}
	}

	MyBlackboard->SetValueAsClass(AbilityClassKey.SelectedKeyName, AbilityClass);	
}

bool UBTService_SelectGameplayAbility::CanBeActivated(const UBehaviorTreeComponent& OwnerComp, const TSubclassOf<UGameplayAbility>& AbilityClass)
{
	const UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerComp.GetOwner());
	if (!IsValid(AbilityComponent))
	{
		return false;
	}

	const FGameplayAbilitySpec* Spec = AbilityComponent->FindAbilitySpecFromClass(AbilityClass);
	if (!Spec)
	{
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = AbilityComponent->AbilityActorInfo.Get();
	const UGameplayAbility* Ability = Spec->GetPrimaryInstance() ? Spec->GetPrimaryInstance() : Spec->Ability.Get();
	return Ability && Ability->CheckCost(Spec->Handle, ActorInfo) && Ability->CheckCooldown(Spec->Handle, ActorInfo);
}
