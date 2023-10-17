#include "ZenoGrassViewExtension.h"

#include "PrimitiveSceneInfo.h"
#include "ScenePrivate.h"
#include "SceneRendering.h"
#include "ZenoMeshElementCollector.h"

FZenoGrassViewExtension::FZenoGrassViewExtension(const FAutoRegister& AutoReg, UWorld* InWorld):
	FWorldSceneViewExtension(AutoReg, InWorld)
{
}

void FZenoGrassViewExtension::AddGroundPrimitiveComponent_RenderThread(UPrimitiveComponent* InPrimitive)
{
	check(IsValid(InPrimitive));
	GroundPrimitives.Add(InPrimitive);
}

void FZenoGrassViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
}

void FZenoGrassViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
}

void FZenoGrassViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	Scene = InViewFamily.Scene != nullptr ? InViewFamily.Scene->GetRenderScene() : nullptr;
}

void FZenoGrassViewExtension::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
}

void FZenoGrassViewExtension::PostRenderBasePassMobile_RenderThread(FRHICommandList& RHICmdList, FSceneView& InView)
{
}

void FZenoGrassViewExtension::PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView,
                                                                      const FRenderTargetBindingSlots& RenderTargets,
                                                                      TRDGUniformBufferRef<
	                                                                      FSceneTextureUniformParameters> SceneTextures)
{
	if (Scene == nullptr)
	{
		return;
	}
	
	FGrassPassParameters* PassParameters = GraphBuilder.AllocParameters<FGrassPassParameters>();
	PassParameters->Test = 1;
	PassParameters->RenderTargets = RenderTargets;

	GraphBuilder.AddPass(RDG_EVENT_NAME("Grass"), PassParameters, ERDGPassFlags::Raster, [this, PassParameters] (FRHICommandList& RHICmdList) 
	{
		const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FGrassShaderVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FGrassShaderGS> GeometryShader(GlobalShaderMap);
		TShaderMapRef<FGrassShaderFS> FragmentShader(GlobalShaderMap);
		
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = FragmentShader.GetPixelShader();

		for (FMeshBatch* MeshBatch : Scene->StaticMeshes)
		{
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = MeshBatch->VertexFactory->GetDeclaration(EVertexInputStreamType::Default);
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
			SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *PassParameters);
			SetShaderParameters(RHICmdList, FragmentShader, FragmentShader.GetPixelShader(), *PassParameters);
		}
	});
}

void FZenoGrassViewExtension::CleanGroundPrimitives()
{
	GroundPrimitives.RemoveAll([](const TSoftObjectPtr<UPrimitiveComponent>& Item)
	{
		return !Item.IsValid();
	});
}

IMPLEMENT_GLOBAL_SHADER(FGrassShaderVS, "/ZenoEngine/Private/Grass.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FGrassShaderGS, "/ZenoEngine/Private/Grass.usf", "MainGS", SF_Geometry);
IMPLEMENT_GLOBAL_SHADER(FGrassShaderFS, "/ZenoEngine/Private/Grass.usf", "MainPS", SF_Pixel);
