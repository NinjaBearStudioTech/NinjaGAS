// Ninja Bear Studio Inc., all rights reserved.
#include "NinjaGASSubsystem.h"

#include "AbilitySystemGlobals.h"

void UNinjaGASSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UAbilitySystemGlobals::Get().InitGlobalData();
}
