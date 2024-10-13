// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"

class FNinjaGASEditorModule : public IModuleInterface
{
public:
	
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

	/** Provides the asset categories supported by this editor module. */
	static EAssetTypeCategories::Type GetAbilityAssetsCategory()
	{
		return AbilityAssetsCategory;
	}

private:

	/** All asset categories managed by this editor module. */
	static EAssetTypeCategories::Type AbilityAssetsCategory;	

	/** All asset type actions registered by this module. */
	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;

	void RegisterAssetTypeActions(IAssetTools& AssetTools, const TSharedRef<IAssetTypeActions>& Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}
};
