// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class zhanshi : ModuleRules
{
	public zhanshi(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.AddRange(new string[] { "zhanshi/Public" });
		PrivateIncludePaths.AddRange(new string[] { "zhanshi/Private" });
		
		PublicDependencyModuleNames.AddRange(new string[] { "Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"GameplayTags",
			"GameplayAbilities",
			"GameplayTasks",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"OnlineSubsystemSteam",
			"UMG",
			"Slate",
			"SlateCore"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
