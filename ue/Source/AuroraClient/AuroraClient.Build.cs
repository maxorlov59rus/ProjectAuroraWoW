using UnrealBuildTool;

public class AuroraClient : ModuleRules
{
	public AuroraClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Sockets",
			"Networking",
			"UMG",
			"Slate",
			"SlateCore",
			"Json",
			"JsonUtilities",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"AuroraClient",
		});
	}
}
