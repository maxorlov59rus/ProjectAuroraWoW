using UnrealBuildTool;
using System.Collections.Generic;

public class AuroraClientEditorTarget : TargetRules
{
	public AuroraClientEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("AuroraClient");
	}
}
