// Copyright (c) Ninja Bear Studio Inc.
#include "Types/FAbilityMontageReplication.h"

#include "AbilitySystem/NinjaGASAbilitySystemComponent.h"

bool FPlayTagGameplayAbilityRepAnimMontage::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityRepAnimMontage::NetSerialize(Ar, Map, bOutSuccess);

	Ar << bOverrideBlendIn;
	Ar << BlendInOverride.Blend.BlendTime;
	Ar << BlendInOverride.Blend.BlendOption;
	Ar << BlendInOverride.Blend.CustomCurve;
	Ar << BlendInOverride.BlendMode;
	Ar << BlendInOverride.BlendProfile;
	
	return true;
}

FGameplayAbilityRepAnimMontageContainer::FGameplayAbilityRepAnimMontageContainer()
{
}

FGameplayAbilityRepAnimMontageContainer::FGameplayAbilityRepAnimMontageContainer(UNinjaGASAbilitySystemComponent* NewAbilitySystemComponent)
{
	AbilitySystemComponent = NewAbilitySystemComponent;
}

void FGameplayAbilityRepAnimMontageContainer::SetAbilitySystemComponent(UNinjaGASAbilitySystemComponent* NewAbilitySystemComponent)
{\
	check(IsValid(NewAbilitySystemComponent));
	AbilitySystemComponent = NewAbilitySystemComponent;
}

FGameplayAbilityRepAnimMontageForMesh& FGameplayAbilityRepAnimMontageContainer::GetGameplayAbilityRepAnimMontageForMesh(USkeletalMeshComponent* Mesh)
{
	for (FGameplayAbilityRepAnimMontageForMesh& Entry : Entries)
	{
		if (Entry.Mesh == Mesh)
		{
			return Entry;
		}
	}

	FGameplayAbilityRepAnimMontageForMesh& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Mesh = Mesh;
	return NewEntry;
}

void FGameplayAbilityRepAnimMontageContainer::MarkMontageDirty(FGameplayAbilityRepAnimMontageForMesh& Entry)
{
	Entry.UpdateReplicationID();
	MarkItemDirty(Entry);
}

void FGameplayAbilityRepAnimMontageContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		AbilitySystemComponent->PostAnimationEntryChange(Entries[Idx]);
		Entries[Idx].LastAnimationReplicationId = Entries[Idx].AnimationReplicationId; 
	}
}

void FGameplayAbilityRepAnimMontageContainer::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		AbilitySystemComponent->PostAnimationEntryChange(Entries[Idx]);
		Entries[Idx].LastAnimationReplicationId = Entries[Idx].AnimationReplicationId;
	}	
}

bool FGameplayAbilityRepAnimMontageContainer::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FastArrayDeltaSerialize<FGameplayAbilityRepAnimMontageForMesh, FGameplayAbilityRepAnimMontageContainer>(Entries, DeltaParams, *this);	
}
