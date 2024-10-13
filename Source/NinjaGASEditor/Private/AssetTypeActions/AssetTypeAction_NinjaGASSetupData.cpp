// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "AssetTypeActions/AssetTypeAction_NinjaGASSetupData.h"

#include "NinjaGASEditor.h"
#include "Data/NinjaGASDataAsset.h"

FText FAssetTypeAction_NinjaGASSetupData::GetName() const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GASSetupData_Title", "G.A.S. Setup");
}

uint32 FAssetTypeAction_NinjaGASSetupData::GetCategories()
{
    return FNinjaGASEditorModule::GetAbilityAssetsCategory();
}

FColor FAssetTypeAction_NinjaGASSetupData::GetTypeColor() const
{
    return FColor(106, 117, 94);
}

FText FAssetTypeAction_NinjaGASSetupData::GetAssetDescription(const FAssetData& AssetData) const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GASSetupData_Description", 
        "Bundles Attribute Sets, Gameplay Effects and Gameplay Abilities that can be assigned to an owner/avatar.");
}

UClass* FAssetTypeAction_NinjaGASSetupData::GetSupportedClass() const
{
    return UNinjaGASDataAsset::StaticClass();
}

const TArray<FText>& FAssetTypeAction_NinjaGASSetupData::GetSubMenus() const
{
	static const TArray SubMenus
	{
		NSLOCTEXT("AssetTypeActions", "AssetTypeActions_NinjaGASSubMenu", "Ninja G.A.S.")
	};

	return SubMenus;
}
