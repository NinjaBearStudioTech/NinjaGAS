// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NinjaGASSubsystem.generated.h"

/**
 * Provides game instance-level functionalities for the Ability System.
 */
UCLASS()
class NINJAGAS_API UNinjaGASSubsystem : public UGameInstanceSubsystem
{
	
	GENERATED_BODY()

public:

	// -- Begin Subsystem implementation
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// -- End Subsystem implementation
	
};
