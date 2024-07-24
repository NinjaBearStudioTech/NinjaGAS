// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NinjaGASFunctionLibrary.generated.h"

/**
 * Helper functions for the Ninja GAS system.
 */
UCLASS()
class NINJAGAS_API UNinjaGASFunctionLibrary : public UBlueprintFunctionLibrary
{
	
	GENERATED_BODY()

public:

	/**
	 * Sends a Gameplay Event to the Owner's of an Ability System Component.
	 *
	 * @param AbilityOwner		Owner of the Ability System Component.
	 * @param EventTag			Gameplay Tag identifying the Gameplay Event.
	 * @param EventData			Payload for the Gameplay Event.
	 * @return					The number of successful ability activations triggered by the event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS", meta = (DefaultToSelf = "AbilityOwner", ReturnDisplayName = "Activations"))
	static int32 SendGameplayEventToActor(const AActor* AbilityOwner, const FGameplayTag EventTag, const FGameplayEventData& EventData);

	/**
	 * Sends a Gameplay Event to the Ability System Component.
	 *
	 * @param AbilityComponent	Ability Component receiving the event.
	 * @param EventTag			Gameplay Tag identifying the Gameplay Event.
	 * @param EventData			Payload for the Gameplay Event.
	 * @return					The number of successful ability activations triggered by the event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS", meta = (ReturnDisplayName = "Activations"))
	static int32 SendGameplayEventToComponent(UAbilitySystemComponent* AbilityComponent, const FGameplayTag EventTag, const FGameplayEventData& EventData);
	
};
