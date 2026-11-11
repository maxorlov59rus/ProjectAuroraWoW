using UnrealBuildTool;

public class AuroraClientTarget : TargetRules
{
	public AuroraClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("AuroraClient");
	}
}
