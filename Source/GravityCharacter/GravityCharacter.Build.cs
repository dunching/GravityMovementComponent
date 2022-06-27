// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GravityCharacter : ModuleRules
{
    public GravityCharacter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HeadMountedDisplay",
            "GravityMovementcomponent",
            "AIModule"
        });
    }
}
