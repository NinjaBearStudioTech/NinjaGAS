// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NinjaGASFunctionLibrary.generated.h"

class UNinjaGASAbilitySystemComponent;

/**
 * Helper functions for the Ninja GAS system.
 */
UCLASS()
class NINJAGAS_API UNinjaGASFunctionLibrary : public UBlueprintFunctionLibrary
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the custom (Ninja GAS) Ability System Component from an owner.
	 * Performs like the Ability System Globals version, but handles the cast for you.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja GAS", meta = (ReturnDisplayName = "ASC"))
	static UNinjaGASAbilitySystemComponent* GetCustomAbilitySystemComponentFromActor(AActor* Owner);
	
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

	/**
	 * Locally adds an <b>Actor<b> Gameplay Cue.
	 *
	 * When adding this Gameplay Cue locally, make sure to also remove it locally.
	 *
	 * @param Target					Actor receiving the local cue.
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Ninja GAS", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void AddGameplayCueLocally(AActor* Target, UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;

	/**
	 * Locally removes an <b>Actor<b> Gameplay Cue.
	 *
	 * @param Target					Actor having the local cue removed.
	 * @param GameplayCueTag			Gameplay Tag for the Gameplay Cue.
	 * @param GameplayCueParameters		Parameters for the Gameplay Cue.
	 */		
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Ninja GAS", meta = (AutoCreateRefTerm = "GameplayCueParameters"))
	void RemoveGameplayCueLocally(AActor* Target, UPARAM(meta = (Categories = "GameplayCue")) const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters) const;
};
