#pragma once
#include "SceneViewExtension.h"
#include "ShaderParameterStruct.h"
#include "ZenoMeshElementCollector.h"

class FStaticMeshBatch;
BEGIN_SHADER_PARAMETER_STRUCT(FGrassPassParameters, ZENORUNTIME_API)
	SHADER_PARAMETER(float, Test)

	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

class FGrassShaderVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FGrassShaderVS);
	using FParameters = FGrassPassParameters;
	SHADER_USE_PARAMETER_STRUCT(FGrassShaderVS, FGlobalShader);
};

class FGrassShaderGS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FGrassShaderGS);
	using FParameters = FGrassPassParameters;
	SHADER_USE_PARAMETER_STRUCT(FGrassShaderGS, FGlobalShader);
};

class FGrassShaderFS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FGrassShaderFS);
	using FParameters = FGrassPassParameters;
	SHADER_USE_PARAMETER_STRUCT(FGrassShaderFS, FGlobalShader);
};

class FZenoGrassViewExtension : public FWorldSceneViewExtension
{
public:
	FZenoGrassViewExtension(const FAutoRegister& AutoReg, UWorld* InWorld);

	void AddGroundPrimitiveComponent_RenderThread(UPrimitiveComponent* InPrimitive);
protected:
	// +Implementation of FViewExtension
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView) override;
	virtual void PostRenderBasePassMobile_RenderThread(FRHICommandList& RHICmdList, FSceneView& InView) override;
	virtual void PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures) override;
	// -Implementation of FViewExtension

	void CleanGroundPrimitives();

private:
	TArray<TSoftObjectPtr<UPrimitiveComponent>> GroundPrimitives;
	
	FScene* Scene = nullptr;
};
