// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaGASCharacter.h"
#include "NinjaGASPlayerCharacter.generated.h"

class UNinjaGASAbilitySystemComponent;

/**
 * A specialized class that will obtain the Ability System Component from the Player State.
 */
UCLASS(Abstract)
class NINJAGAS_API ANinjaGASPlayerCharacter : public ANinjaGASCharacter
{
	
	GENERATED_BODY()

public:

	ANinjaGASPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Pawn implementation
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	// -- End Pawn implementation
	
	// -- Begin Ability System implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// -- End Ability System implementation

protected:

	// -- Begin GAS Character implementation
	virtual void SetupAbilitySystemComponent(AActor* AbilitySystemOwner) override;
	virtual void ClearAbilitySystemComponent() override;
	// -- End GAS Character implementation
	
private:

	/** Weak reference to the Character's Ability Component. */
	TWeakObjectPtr<UNinjaGASAbilitySystemComponent> CharacterAbilitiesPtr;
	
};
