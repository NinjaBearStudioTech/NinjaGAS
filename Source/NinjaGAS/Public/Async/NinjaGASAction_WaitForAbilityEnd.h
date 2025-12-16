// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "NinjaGASAction_WaitForAbilityEnd.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityEndAsyncActionSignature, bool, bWasCancelled);

/**
 * Waits for an ability to end.
 */
UCLASS()
class NINJAGAS_API UNinjaGASAction_WaitForAbilityEnd : public UBlueprintAsyncActionBase
{
	
	GENERATED_BODY()

public:

	/** The ability has successfully ended. */
	UPROPERTY(BlueprintAssignable)
	FAbilityEndAsyncActionSignature OnAbilityEnded;

	/**
	 * Creates the Action to wait for an Ability to end.
	 *
	 * @param AbilityOwner		Owner of the Ability System Component, must be valid.
	 * @param AbilityCriteria	Gameplay Tag Query that will match ability tags.
	 * @return					Configured instance of the async action.
	 */
	UFUNCTION(BlueprintCallable, Category = "NBS|GAS|Async|Wait for Ability End", DisplayName = "Wait For Ability End", meta = (DefaultToSelf = "AbilityOwner", BlueprintInternalUseOnly = "true"))
	static UNinjaGASAction_WaitForAbilityEnd* CreateAction(AActor* AbilityOwner, FGameplayTagQuery AbilityCriteria);
	
	virtual void Activate() override;
	virtual void BeginDestroy() override;

protected:

	/** Receives an ability that ended and checks if the criteria matches. */
	virtual void HandleAbilityEnded(const FAbilityEndedData& AbilityEndedData);
	
private:

	/** Handle for the ability delegate. */
	FDelegateHandle AbilityEndedDelegateHandle;
	
	/** Actor that owns the Ability System Component. */
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemPtr = nullptr;

	/** Criteria used to match the ability; */
	FGameplayTagQuery AbilityCriteria = FGameplayTagQuery::EmptyQuery;
	
};
