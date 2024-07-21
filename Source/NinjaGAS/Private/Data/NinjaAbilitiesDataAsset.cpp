// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaAbilitiesDataAsset.h"

FPrimaryAssetType UNinjaAbilitiesDataAsset::AssetType = TEXT("AbilitiesDataAsset");

UNinjaAbilitiesDataAsset::UNinjaAbilitiesDataAsset()
{
}

FPrimaryAssetId UNinjaAbilitiesDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(AssetType, GetFName());
}
