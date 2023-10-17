#include "ZenoRuntime.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FZenoRuntimeModule"

void FZenoRuntimeModule::StartupModule()
{
	IPluginManager& PluginManager = IPluginManager::Get();
	const TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin("ZenoEngine");

	check(Plugin.IsValid());

	FString PluginDirectory = Plugin->GetBaseDir();
	const FString ModuleShaderDir = FPaths::Combine(PluginDirectory, TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/ZenoEngine"), ModuleShaderDir);
}

void FZenoRuntimeModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZenoRuntimeModule, ZenoRuntime)