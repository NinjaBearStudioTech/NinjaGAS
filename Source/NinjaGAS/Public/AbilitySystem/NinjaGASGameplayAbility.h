// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interfaces/BatchGameplayAbilityInterface.h"
#include "NinjaGASGameplayAbility.generated.h"

/**
 * Base Ability Class providing all extensions available in the framework. 
 */
UCLASS()
class NINJAGAS_API UNinjaGASGameplayAbility : public UGameplayAbility, public IBatchGameplayAbilityInterface
{
	
	GENERATED_BODY()

public:

	// Begin Gameplay Ability implementation 
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// End Gameplay Ability implementation
	
	// -- Begin Batch Gameplay Ability implementation
	virtual void EndAbilityFromBatch_Implementation() override;
	// -- End Batch Gameplay Ability implementation

	/**
	 * Checks if this Gameplay Ability is passive, which is done via the passive tag.
	 * Returns true if "Ability.Passive" is found in the Ability Tags container.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja GAS|Gameplay Ability")
	bool IsPassiveAbility() const;
	
};
