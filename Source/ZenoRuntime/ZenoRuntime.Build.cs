using System.IO;
using UnrealBuildTool;

public class ZenoRuntime : ModuleRules
{
    public ZenoRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "Landscape",
                "Projects",
                "RenderCore",
                "Renderer",
                "RHICore",
                "RHI",
                "Landscape",
            }
        );
        
		// Fix for a private postprocessing header inside ViewExtension.
        var enginePath = Path.GetFullPath(Target.RelativeEnginePath);
        PublicIncludePaths.Add(enginePath + "Source/Runtime/Renderer/Private");
    }
}