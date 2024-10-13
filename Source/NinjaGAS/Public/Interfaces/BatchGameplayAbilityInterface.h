// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BatchGameplayAbilityInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBatchGameplayAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines a Gameplay Ability compatible with the Ability Batching system.
 */
class NINJAGAS_API IBatchGameplayAbilityInterface
{
	
	GENERATED_BODY()

public:
	
	/**
	 * Analogous to "K2_EndAbility" function, but exposed to identify that this is triggered by a batched RPC. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Batch Gameplay Ability")
	void EndAbilityFromBatch();

};
