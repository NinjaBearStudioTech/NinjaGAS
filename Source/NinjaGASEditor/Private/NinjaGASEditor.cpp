// Ninja Bear Studio Inc. 2024, all rights reserved.
#include "NinjaGASEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions/AssetTypeAction_NinjaGASSetupData.h"

#define LOCTEXT_NAMESPACE "FNinjaGASEditorModule"

IMPLEMENT_MODULE(FNinjaGASEditorModule, NinjaGASEditor)

EAssetTypeCategories::Type FNinjaGASEditorModule::AbilityAssetsCategory;

void FNinjaGASEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AbilityAssetsCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("NinjaBearStudio")), LOCTEXT("NinjaBearStudioAssetsCategory", "Ninja Bear Studio"));
	{
		RegisterAssetTypeActions(AssetTools, MakeShareable(new FAssetTypeAction_NinjaGASSetupData));
	}
}

void FNinjaGASEditorModule::ShutdownModule()
{
	if (const FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		for (TSharedPtr<IAssetTypeActions>& AssetAction : CreatedAssetTypeActions)
		{
			AssetToolsModule->Get().UnregisterAssetTypeActions(AssetAction.ToSharedRef());
		}
	}
    
	CreatedAssetTypeActions.Empty();
}

#undef LOCTEXT_NAMESPACE
    
