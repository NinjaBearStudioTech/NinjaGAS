// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "NinjaAbilitiesPlayerState.generated.h"

class UAbilitySystemComponent;

/**
 * Player State that contains a Gameplay Ability System.  
 */
UCLASS()
class NINJAGAS_API ANinjaAbilitiesPlayerState : public APlayerState, public IAbilitySystemInterface
{
	
	GENERATED_BODY()

public:

	/** Name used to create the Ability Component. Can be used to overwrite the base class. */
	static FName AbilityComponentName;

	ANinjaAbilitiesPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Ability System implementation
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	// -- End Ability System implementation

private:

	/** Hard reference to the Ability System Component used by the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
};
