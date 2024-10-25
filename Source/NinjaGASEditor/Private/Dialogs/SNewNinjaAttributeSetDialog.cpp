// Ninja Bear Studio Inc., all rights reserved.
#include "SNewNinjaAttributeSetDialog.h"

#include "GameProjectUtils.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SNewNinjaAttributeSetDialog::Construct(const FArguments& InArgs)
{
	CollectAvailableModules();
	SetDefaultValues(InArgs);
}

void SNewNinjaAttributeSetDialog::CollectAvailableModules()
{
	TArray<FModuleContextInfo> CurrentModules = GameProjectUtils::GetCurrentProjectModules();
	check(CurrentModules.Num()); // Sanity check, this should never happen.

	TArray<FModuleContextInfo> CurrentPluginModules = GameProjectUtils::GetCurrentProjectPluginModules();
	CurrentModules.Append(CurrentPluginModules);
	
	AvailableModules.Reserve(CurrentModules.Num());
	for(const FModuleContextInfo& ModuleInfo : CurrentModules)
	{
		AvailableModules.Emplace(MakeShareable(new FModuleContextInfo(ModuleInfo)));
	}

	Algo::SortBy(AvailableModules, &FModuleContextInfo::ModuleName);	
}

void SNewNinjaAttributeSetDialog::SetDefaultValues(const FArguments& InArgs)
{
	const FString AbsoluteInitialPath = FPaths::ConvertRelativePathToFull(InArgs._InitialPath);
	for(const auto& AvailableModule : AvailableModules)
	{
		if(AbsoluteInitialPath.StartsWith(AvailableModule->ModuleSourcePath))
		{
			SelectedModuleInfo = AvailableModule;
			NewClassPath = AbsoluteInitialPath;
			break;
		}
	}
}
