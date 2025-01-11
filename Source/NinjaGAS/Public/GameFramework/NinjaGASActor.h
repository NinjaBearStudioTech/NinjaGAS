// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Interfaces/AbilitySystemDefaultsInterface.h"
#include "Interfaces/LazyAbilitySystemComponentOwnerInterface.h"
#include "Types/FPendingAttributeReplication.h"
#include "NinjaGASActor.generated.h"

class UNinjaGASAbilitySystemComponent;

/**
 * Base Actor class, with a pre-configured Ability System Component.
 */
UCLASS(Abstract)
class NINJAGAS_API ANinjaGASActor : public AActor, public IAbilitySystemInterface, public IAbilitySystemDefaultsInterface,
	public ILazyAbilitySystemComponentOwnerInterface
{
	
	GENERATED_BODY()

public:

	/** Name for the ASC Component, if initialized by the class. */
	static FName AbilitySystemComponentName;
	
	ANinjaGASActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Actor implementation
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitProperties() override;
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// -- End Actor implementation

	// -- Begin Ability System implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UNinjaGASDataAsset* GetAbilityData() const override;
	// -- End Ability System implementation

	// -- Begin Lazy Ability System Component Owner implementation
	virtual ELazyAbilitySystemInitializationMode GetAbilitySystemInitializationMode() const override;
	virtual void InitializeAbilitySystemComponent() override;
	virtual void SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue) override;
	virtual void ApplyPendingAttributesFromReplication() override;
	// -- End Lazy Ability System Component Owner implementation
	
protected:

	/**
	 * Determines how this actor will initialize its Ability System Component.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	ELazyAbilitySystemInitializationMode AbilitySystemInitializationMode;
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	EGameplayEffectReplicationMode AbilityReplicationMode;

	/** The class used to initialize the Ability System Component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TSubclassOf<UNinjaGASAbilitySystemComponent> AbilitySystemComponentClass;

	/** Default configuration for the Ability System. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UNinjaGASDataAsset> DefaultAbilitySetup;
	
	/**
	 * Hook invoked when the ability system component replicates.
	 */
	UFUNCTION()
	virtual void OnRep_ReplicatedActorAbilities();
	
private:

	/** The Ability System Component managed by this actor class. */
	UPROPERTY(Transient)
	TObjectPtr<UNinjaGASAbilitySystemComponent> ActorAbilities;

	/** Replicated Ability System Component, considering the creation policy. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReplicatedActorAbilities)
	TObjectPtr<UNinjaGASAbilitySystemComponent> ReplicatedActorAbilities;

	/**
	 * Attributes pending replication, that must be handled when the ASC replicates.
	 * Always empty if the ASC initialization is set to "eager".
	 */
	TArray<FPendingAttributeReplication> PendingAttributeReplications;
	
};
