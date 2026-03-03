// Copyright (c) Ninja Bear Studio Inc.
// 
// This file incorporates portions of code from:
//   Copyright (c) Dan Kestranek (https://github.com/tranek)
//   Copyright (c) Jared Taylor (https://github.com/Vaei/)
//
// The incorporated portions are licensed under the MIT License.
// The full MIT license text is included in THIRD_PARTY_NOTICES.md.
//
#pragma once

#include "Abilities/GameplayAbilityRepAnimMontage.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "FAbilityMontageReplication.generated.h"

class UPackageMap;
class UNinjaGASAbilitySystemComponent;

/**
 * Data about montages that were played locally.
 * This means all montages in case of server, and predictive montages in case of client. 
 */
USTRUCT()
struct NINJAGAS_API FGameplayAbilityLocalAnimMontageForMesh
{
	GENERATED_BODY();

	UPROPERTY()
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY()
	FGameplayAbilityLocalAnimMontage LocalMontageInfo;

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh = nullptr)
		: Mesh(InMesh)
	{
	}

	FGameplayAbilityLocalAnimMontageForMesh(USkeletalMeshComponent* InMesh, const FGameplayAbilityLocalAnimMontage& InLocalMontageInfo)
		: Mesh(InMesh), LocalMontageInfo(InLocalMontageInfo)
	{
	}
};

USTRUCT()
struct NINJAGAS_API FPlayTagGameplayAbilityRepAnimMontage : public FGameplayAbilityRepAnimMontage
{
	GENERATED_BODY()

	UPROPERTY()
	bool bOverrideBlendIn;

	UPROPERTY()
	FMontageBlendSettings BlendInOverride;

	FPlayTagGameplayAbilityRepAnimMontage()
		: bOverrideBlendIn(false)
		, BlendInOverride({})
	{}
	
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);
};

/**
* Data about montages that is replicated to simulated clients.
 */
USTRUCT()
struct NINJAGAS_API FGameplayAbilityRepAnimMontageForMesh : public FFastArraySerializerItem
{
	GENERATED_BODY();

	UPROPERTY()
	USkeletalMeshComponent* Mesh;

	UPROPERTY()
	FPlayTagGameplayAbilityRepAnimMontage RepMontageInfo;

	UPROPERTY()
	uint8 AnimationReplicationId = 0;
	
	UPROPERTY(NotReplicated)
	uint8 LastAnimationReplicationId = 0;
	
	FGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* InMesh = nullptr)
		: Mesh(InMesh)
	{
	}
	
	void UpdateReplicationID()
	{
		AnimationReplicationId = AnimationReplicationId < UINT8_MAX ? AnimationReplicationId + 1 : 0;
	}
	
	bool IsSynchronized() const
	{
		return AnimationReplicationId == LastAnimationReplicationId;
	}
};

/** 
 * Container with all replicated montages playing on clients with custom meshes.
 */
USTRUCT()
struct FGameplayAbilityRepAnimMontageContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGameplayAbilityRepAnimMontageForMesh> Entries;

	FGameplayAbilityRepAnimMontageContainer();
	
	FGameplayAbilityRepAnimMontageContainer(UNinjaGASAbilitySystemComponent* NewAbilitySystemComponent);
	
	void SetAbilitySystemComponent(UNinjaGASAbilitySystemComponent* NewAbilitySystemComponent);
	FGameplayAbilityRepAnimMontageForMesh& GetGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* Mesh);
	void MarkMontageDirty(FGameplayAbilityRepAnimMontageForMesh& Entry);
	
	// -- Begin FFastArraySerializer implementation
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	// -- End FFastArraySerializer implementation	
	
private:
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UNinjaGASAbilitySystemComponent> AbilitySystemComponent;
	
};

template<>
struct TStructOpsTypeTraits<FPlayTagGameplayAbilityRepAnimMontage> : TStructOpsTypeTraitsBase2<FPlayTagGameplayAbilityRepAnimMontage>
{
	enum
	{
		WithNetSerializer = true,
	};
};

template<>
struct TStructOpsTypeTraits<FGameplayAbilityRepAnimMontageContainer> : TStructOpsTypeTraitsBase2<FGameplayAbilityRepAnimMontageContainer>
{
	enum
	{
		WithNetDeltaSerializer  = true,
	};
};