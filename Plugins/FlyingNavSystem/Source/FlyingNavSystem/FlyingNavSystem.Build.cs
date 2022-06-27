// Copyright Ben Sutherland 2021. All rights reserved.

using UnrealBuildTool;

public class FlyingNavSystem : ModuleRules
{
	public FlyingNavSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[] {"Core", "CoreUObject", "Engine", "NavigationSystem"}); 
		PrivateDependencyModuleNames.AddRange(new[] {"RHI", "RenderCore", "AIModule"}); // RHI: FDynamicPrimitiveUniformBuffer, RenderCore: FIndexBuffer and FVertexFactory

		PublicIncludePathModuleNames.AddRange(new[] {"AIModule"}); // AIModule has GraphAStar.h, which is header-only.
		
		// Uncomment for benchmark project
		// PublicDefinitions.Add("PATH_BENCHMARK=1"); 
		
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new[] {"UnrealEd"});
		}
	}
}
