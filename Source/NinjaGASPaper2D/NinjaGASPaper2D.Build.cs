// Ninja Bear Studio Inc., all rights reserved.
using UnrealBuildTool;

public class NinjaGASPaper2D : ModuleRules
{
    public NinjaGASPaper2D(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new []
        {
            "Core",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "NinjaGAS",
            "Paper2D"
        });

        PrivateDependencyModuleNames.AddRange(new []
        {
            "CoreUObject",
            "Engine"
        });
    }
}