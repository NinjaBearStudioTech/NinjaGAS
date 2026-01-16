// Ninja Bear Studio Inc., all rights reserved.
// ReSharper disable InconsistentNaming
using UnrealBuildTool;

public class NinjaGAS : ModuleRules
{
	public NinjaGAS(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new []
		{
			"AIModule",
			"Core", 
			"InputCore", 
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"GameplayStateTreeModule",
			"ModularGameplay",
			"NetCore",
			"StateTreeModule"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"CoreUObject",
			"Engine"
		});	
	}
}