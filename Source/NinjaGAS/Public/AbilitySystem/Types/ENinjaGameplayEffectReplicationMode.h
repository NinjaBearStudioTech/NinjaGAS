// Ninja Bear Studio Inc., all rights reserved.
#pragma once

/**
 * How gameplay effects will be replicated to clients
 * Analogous to the core "ENinjaGameplayEffectReplicationMode". 
 */
UENUM(BlueprintType)
enum class ENinjaGameplayEffectReplicationMode : uint8
{
	/**
	 * Only replicate minimal gameplay effect info.
	 * Note: this does not work for Owned AbilitySystemComponents (Use Mixed instead).
	 */
	Minimal,
	
	/**
	 * Only replicate minimal gameplay effect info to simulated proxies but full info to owners and autonomous proxies
	 */
	Mixed,
	
	/**
	 * Replicate full gameplay info to all.
	 */
	Full,
};
