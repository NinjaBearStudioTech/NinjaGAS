// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "Data/NinjaGASDataAsset.h"

#include "GameplayTagContainer.h"

FPrimaryAssetType UNinjaGASDataAsset::AssetType = TEXT("AbilityBundleData");

UNinjaGASDataAsset::UNinjaGASDataAsset()
{
	InitialGameplayTags = FGameplayTagContainer::EmptyContainer;
}

FPrimaryAssetId UNinjaGASDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
