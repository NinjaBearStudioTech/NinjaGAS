// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AddToProjectConfig.h"
#include "ModuleDescriptor.h"
#include "AbilitySystem/NinjaGASAttributeSet.h"
#include "UObject/Object.h"

class UNinjaGASAttributeSet;

/**
 * The Dialog used to create new Attribute Sets based on Ninja GAS.
 *
 * Built using "SNewClassDialog" as a starting reference, which is the dialog used when you
 * click "New C++ Class" in the Editor.
 *
 * The purpose of this dialog is to create an Attribute Set class (or update its contents),
 * based on the Data Table that would be used to initialize its values.
 *
 * Providing a Data Table to an Attribute Set that was already created in the main project
 * path would recreate its contents. The dialog can only modify Attribute Sets in the main
 * project folders. Plugins always must be excluded.
 */
class SNewNinjaAttributeSetDialog final : public SCompoundWidget
{

public:
	
	SLATE_BEGIN_ARGS(SNewNinjaAttributeSetDialog)
		: _AttributeSetClass(UNinjaGASAttributeSet::StaticClass())
	{}

	/**
	 * The class we want to create the Attribute Set from.
	 * 
	 * By default, it's always the Ninja GAS one, but we may want to allow users to select
	 * different base Attribute Sets in the future, so let's leave this flexible.
	 */
	SLATE_ARGUMENT(const UClass*, AttributeSetClass);

	/**
	 * The initial path to use as the destination for the new Attribute Set class.
	 * If this is not specified, we will work out a suitable default from the available project modules.
	 */
	SLATE_ARGUMENT(FString, InitialPath)

	/**
	 * The Attribute Set name is extracted from the provided Data Table.
	 * 
	 * A Data Table created from "AttributeRowTable" has a required naming convention that defines
	 * the name of the Attribute Set that it initializes. We want to use that as the name for our class.
	 */
	SLATE_ARGUMENT(FString, AttributeSetName)
		
	/**
	 * Event called when the Attribute Set is successfully added to the project.
	 * At this point we probably want to open the IDE to show the newly created Attribute Set.
	 */
	SLATE_EVENT(FOnAddedToProject, OnAddedToProject)
		
	SLATE_END_ARGS()

	/** Constructs this widget with the provided arguments. */
	void Construct(const FArguments& InArgs);

protected:

	/** Collects all available modules (game and plugin) that can be used for class creation. */
	void CollectAvailableModules();

	/** Selects the most adequate module to get started, along with the absolute classpath. */
	void SetDefaultValues(const FArguments& InArgs);
	
private:

	/** Information about the currently available modules for this project */
	TArray<TSharedPtr<FModuleContextInfo>> AvailableModules;	

	/** Information about the currently selected module; used for class validation. */
	TSharedPtr<FModuleContextInfo> SelectedModuleInfo;
	
	/** The path to place the files for the Attribute Set class being generated. */
	FString NewClassPath;
	
};
