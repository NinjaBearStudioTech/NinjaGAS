// Ninja Bear Studio Inc., all rights reserved.
#include "AI/StateTree/StateTreeAbilityCooldownConsideration.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

float FStateTreeAbilityCooldownConsideration::GetScore(FStateTreeExecutionContext& Context) const
{
	float Score = Super::GetScore(Context);

	const UObject* Owner = Context.GetOwner();
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent(Owner);

	if (IsValid(AbilitySystemComponent))
	{
		const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		const FGameplayTagContainer AbilityCooldownTags = InstanceData.AbilityCooldownTags;

		if (IsCooldownActive(AbilitySystemComponent, AbilityCooldownTags))
		{
			Score = InstanceData.ScoreWhenOnCooldown;
		}
		else
		{
			Score = InstanceData.ScoreWhenAvailable;
		}
	}
	
	return Score;
}

UAbilitySystemComponent* FStateTreeAbilityCooldownConsideration::GetAbilitySystemComponent(const UObject* Owner)
{
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	if (Owner->Implements<UAbilitySystemInterface>())
	{
		return Cast<IAbilitySystemInterface>(Owner)->GetAbilitySystemComponent();
	}

	if (Owner->IsA<AController>())
	{
		const APawn* PawnOwner = Cast<AController>(Owner)->GetPawn();
		return GetAbilitySystemComponent(PawnOwner); 
	}

	return nullptr;
}

bool FStateTreeAbilityCooldownConsideration::IsCooldownActive(const UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTagContainer& AbilityCooldownTags)
{
	if (!IsValid(AbilitySystemComponent) || !AbilityCooldownTags.IsValid())
	{
		return false;
	}

	const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(AbilityCooldownTags);
	const TArray<FActiveGameplayEffectHandle> ActiveEffects = AbilitySystemComponent->GetActiveEffects(Query);
	return ActiveEffects.Num() > 0;
}