// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "AbilityAnimationMontageAwareInterface.generated.h"

class USkeletalMeshComponent;
class UAnimMontage;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UAbilityAnimationMontageAwareInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines a Gameplay Ability that will be aware of its current Animation Montages.
 */
class NINJAGAS_API IAbilityAnimationMontageAwareInterface
{
	
	GENERATED_BODY()

public:

	/** Sets the animation mesh and animation montage played by the ability. */
	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* Mesh, UAnimMontage* Montage) = 0;
	
};
