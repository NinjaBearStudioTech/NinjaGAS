// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SelectGameplayAbility.generated.h"

class UGameplayAbility;

/**
 * Selects a Gameplay Ability from a list, optionally forcing changes between current and new selections.
 */
UCLASS(DisplayName = "Select Gameplay Ability", Category = "Ninja GAS")
class NINJAGAS_API UBTService_SelectGameplayAbility : public UBTService
{
	
	GENERATED_BODY()

public:

	UBTService_SelectGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Tree Service implementation
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	// -- End Tree Service implementation

protected:

	/** Blackboard storing the selected ability. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Selection")
	FBlackboardKeySelector AbilityClassKey;

	/** Gameplay Abilities to pick from. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability Selection")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	static bool CanBeActivated(const UBehaviorTreeComponent& OwnerComp, const TSubclassOf<UGameplayAbility>& AbilityClass);
	
};
