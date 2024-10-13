// Ninja Bear Studio Inc. 2024, all rights reserved.
// ReSharper disable InconsistentNaming
using UnrealBuildTool;

public class NinjaGASEditor : ModuleRules
{
    public NinjaGASEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
	        new []
	        {
		        "Core", 
		        "NinjaGAS"
	        }
        );

        PrivateDependencyModuleNames.AddRange(
            new []
            {
	            "AssetTools",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd"
            }
        );
    }
}