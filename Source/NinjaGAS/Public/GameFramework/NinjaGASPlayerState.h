// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Components/PlayerStateComponent.h"
#include "GameFramework/PlayerState.h"
#include "NinjaGASPlayerState.generated.h"

class UAbilitySystemComponent;

/**
 * Player State that contains a Gameplay Ability System.
 *
 * It allows subclasses to define their own Ability System Component type using the "AbilityComponentName",
 * or even fully disabling the component's creation, in case it will be provided via Modular Gameplay.
 *
 * This Player state is also compatible with Modular Gameplay Features, so it will broadcast the proper
 * Modular Gameplay Events during, from the proper lifecycle methods.
 */
UCLASS()
class NINJAGAS_API ANinjaGASPlayerState : public APlayerState, public IAbilitySystemInterface
{
	
	GENERATED_BODY()

public:

	/** Name used to create the Ability Component. Can be used to overwrite the base class. */
	static FName AbilityComponentName;

	ANinjaGASPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Player State implementation
	virtual void PostInitProperties() override;
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void CopyProperties(APlayerState* TargetPlayerState) override;
	virtual void Reset() override;
	// -- End Player State implementation
	
	// -- Begin Ability System implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// -- End Ability System implementation

protected:

	/**
	 * Sets how the Ability System component will replicate data to clients.
	 *
	 * As per Dan Tranek's GAS Documentation, these are their descriptions and recommendations:
	 * 
	 * - Full:		Single Player, every Gameplay Effect is replicated to every client.
	 * 
	 * - Mixed:		Player Actors in Multiplayer, Gameplay Effects are replicated to owning client.
	 *				Gameplay Tags and Cues will replicate to everyone.
	 *				
	 * - Minimal	AI Actors in Multiplayer, Gameplay Effects are never replicated.
	 *				Gameplay Tags and Cues will replicate to everyone.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability System")
	EGameplayEffectReplicationMode AbilityReplicationMode;

	/**
	 * Dispatches the Copy event to all Player State Components.
	 */
	virtual void DispatchCopyToPlayerStateComponents(APlayerState* TargetPlayerState);
	
	/**
	 * Dispatches the Reset event to all Player State components.
	 */
	virtual void DispatchResetPlayerStateComponents();

	/**
	 * Finds the equivalent Player State Component on the target.
	 * 
	 * Uses the static finder by class name, but if you have a different strategy, such as
	 * deliberately pointing out the components, then you can override this method.
	 */
	virtual UPlayerStateComponent* GetPlayerStateComponent(APlayerState* TargetPlayerState, const UPlayerStateComponent* Reference) const;
	
	/**
	 * Helper function to collect all player stat components.
	 */
	void ForEachPlayerStateComponent(const TFunctionRef<void(UPlayerStateComponent*)>& Function) const;
	
private:

	/** Hard reference to the Ability System Component used by the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
};
