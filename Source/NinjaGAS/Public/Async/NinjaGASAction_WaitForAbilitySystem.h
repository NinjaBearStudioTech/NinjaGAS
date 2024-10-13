// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Engine/CancellableAsyncAction.h"
#include "NinjaGASAction_WaitForAbilitySystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityAsyncActionSignature);

/**
 * Waits for an Ability System to become available and initialized.
 */
UCLASS()
class NINJAGAS_API UNinjaGASAction_WaitForAbilitySystem : public UCancellableAsyncAction
{
	
	GENERATED_BODY()

public:

	/** The task has completed and the ASC is available. */
	UPROPERTY(BlueprintAssignable)
	FAbilityAsyncActionSignature OnCompleted;

	/** The action has failed. */
	UPROPERTY(BlueprintAssignable)
	FAbilityAsyncActionSignature OnCancelled;

	/** The action has timed-out. */
	UPROPERTY(BlueprintAssignable)
	FAbilityAsyncActionSignature OnTimedOut;

	/**
	 * Creates the Action to wait for an Ability System Component in an Actor.
	 *
	 * @param AbilityOwner		Owner of the Ability System Component, must be valid.
	 * @param CheckInterval		How often the task will check for a valid Ability System Component.
	 * @param MaxWait			Maximum time to wait in seconds.
	 * @return					Configured instance of the async action.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS|Async", DisplayName = "Wait For Ability System", meta = (DefaultToSelf = "AbilityOwner", BlueprintInternalUseOnly = "true"))
	static UNinjaGASAction_WaitForAbilitySystem* CreateAction(AActor* AbilityOwner, float CheckInterval, float MaxWait = 2.f);
	
	// -- Begin UObject implementation
	virtual UWorld* GetWorld() const override;
	// -- End UObject implementation

	// -- Begin Cancellable Async Action implementation
	virtual void Activate() override;
	virtual void Cancel() override;
	// -- End Cancellable Async Action implementation

	/**
	 * Provides the Ability Owner set to this task.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja GAS|Async")
	AActor* GetAbilityOwner() const;
	
protected:

	/**
	 * Sets the world obtained from the provided context.
	 */
	virtual void SetWorld(UWorld* World);

	/**
	 * Sets the timer's interval and max wait.
	 */
	virtual void SetTimerParameters(float InCheckInterval, float InMaxWait = 2.f);	

	/**
	 * Accumulates the Elapsed time based on the current timer settings.
	 * Cancels the Action if the elapsed time exceeds the maximum wait.
	 */
	void AccumulateElapsedTime();
	
private:

	/** How long this action waits to check for the desired object. */
	float CheckInterval = 0.f;

	/** Time since the search started. */
	float ElapsedTime = 0.f;
	
	/** Maximum wait time. */
	float MaxWait = 2.f;
	
	/** Access to the world that can provide timers. */
	TWeakObjectPtr<UWorld> WorldPtr = nullptr;	

	/** Actor that owns the Ability System Component. */
	TWeakObjectPtr<AActor> AbilityOwnerPtr = nullptr;
	
	/** Handle for the Ability System Component lookup. */
	FTimerHandle LookupHandle;
	
};
