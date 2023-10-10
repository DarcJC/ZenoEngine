using System.IO;
using UnrealBuildTool;

public class ZenoEditor : ModuleRules
{
    public ZenoEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseRTTI = true;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ZenoRuntime",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "InputCore",
                "Json",
                "ImageWrapper",
                "Engine",
                "Slate",
                "UnrealEd",
                "SlateCore",
                "LevelEditor",
                "DesktopPlatform",
                "MeshDescription",
                "StaticMeshDescription",
                "AssetTools",
                "DeveloperSettings",
                "ContentBrowser",
                "Kismet",
                "PropertyEditor",
                "MeshUtilities",
				"DesktopPlatform",
				"UObjectPlugin",
                "EditorFramework",
                "Foliage",
                "FoliageEdit",
                "Landscape",
                "LandscapeEditor",
                "RawMesh",
                "ImageWrapper",
                "TypedElementFramework",
                "TypedElementRuntime",
            }
        );
        
        #if UE_5_2_OR_LATER
        PublicDefinitions.AddRange(new string[]
        {
            "UE_5_2_OR_LATER=1",
        });
        #endif

        OptimizeCode = CodeOptimization.InNonDebugBuilds;

        // PrivatePCHHeaderFile = "Private/ZenoEditorPrivatePCH.h";
    }
}