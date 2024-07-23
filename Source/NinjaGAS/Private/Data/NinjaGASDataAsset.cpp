// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaGASDataAsset.h"

FPrimaryAssetType UNinjaGASDataAsset::AssetType = TEXT("AbilitiesDataAsset");

UNinjaGASDataAsset::UNinjaGASDataAsset()
{
}

FPrimaryAssetId UNinjaGASDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
