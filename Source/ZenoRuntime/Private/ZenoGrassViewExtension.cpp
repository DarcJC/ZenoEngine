#include "ZenoGrassViewExtension.h"

#include "PrimitiveSceneInfo.h"
#include "ScenePrivate.h"
#include "SceneRendering.h"

static TAutoConsoleVariable<float> CVarGrassCardScale(
	TEXT("r.Zeno.GrassCardScale"),
	50.f,
	TEXT("Allow to set the size of the grass card.\n")
	TEXT("This option will not effect the performence but might remove later.\n")
	TEXT("Default value: 50.0"),
	ECVF_Scalability | ECVF_RenderThreadSafe);

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
	Scene = InViewFamily.Scene != nullptr ? InViewFamily.Scene->GetRenderScene() : nullptr;
	CurrentFrameSceneInfos.Reset();
	for (TSoftObjectPtr<UPrimitiveComponent> PrimitiveComponentPtr : GroundPrimitives)
	{
		if (UPrimitiveComponent* PrimitiveComponent = PrimitiveComponentPtr.Get(); PrimitiveComponent != nullptr && PrimitiveComponent->bRenderInMainPass)
		{
			FPrimitiveSceneProxy* SceneProxy = PrimitiveComponent->SceneProxy;

			if (nullptr == SceneProxy)
			{
				continue;
			}
			
			FPrimitiveSceneInfo* PrimitiveSceneInfo = SceneProxy->GetScene().GetPrimitiveSceneInfo(SceneProxy->GetPrimitiveComponentId());
			if (PrimitiveSceneInfo != nullptr)
				CurrentFrameSceneInfos.Add(PrimitiveSceneInfo);
		}
	}
}

void FZenoGrassViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
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
	if (Scene == nullptr || CurrentFrameSceneInfos.IsEmpty() || !InView.bIsViewInfo || !InView.bIsGameView)
	{
		return;
	}
	
	FViewInfo* ViewInfo = static_cast<FViewInfo*>(&InView);

	FGrassPassParameters* PassParameters = GraphBuilder.AllocParameters<FGrassPassParameters>();
	PassParameters->RenderTargets = RenderTargets;
	PassParameters->View.View = ViewInfo->ViewUniformBuffer;
	PassParameters->View.InstancedView = ViewInfo->GetInstancedViewUniformBuffer();
	PassParameters->ViewMatrix = FMatrix44f(ViewInfo->ViewMatrices.GetViewMatrix());
	PassParameters->ProjectionMatrix = FMatrix44f(ViewInfo->ViewMatrices.GetProjectionMatrix());
	PassParameters->ViewProjectionMatrix = FMatrix44f(ViewInfo->ViewMatrices.GetViewProjectionMatrix());
	PassParameters->GrassCardScale = CVarGrassCardScale.GetValueOnRenderThread();
	
	GraphBuilder.AddPass(RDG_EVENT_NAME("Grass"), PassParameters, ERDGPassFlags::Raster, [this, PassParameters, ViewInfo] (FRHICommandList& RHICmdList) 
	{
		const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FGrassShaderVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FGrassShaderGS> GeometryShader(GlobalShaderMap);
		TShaderMapRef<FGrassShaderFS> FragmentShader(GlobalShaderMap);
		
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI();
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.SetGeometryShader(GeometryShader.GetGeometryShader());
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = FragmentShader.GetPixelShader();

		for (FPrimitiveSceneInfo* SceneInfo : CurrentFrameSceneInfos)
		{
			if (!SceneInfo->bDrawInGame) continue;
			PassParameters->ModelMatrix = FMatrix44f(SceneInfo->Proxy->GetLocalToWorld());
			for (FStaticMeshBatch& StaticMesh : SceneInfo->StaticMeshes)
			{
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = StaticMesh.VertexFactory->GetDeclaration(EVertexInputStreamType::PositionOnly);
				if (StaticMesh.bWireframe || ViewInfo->Family->ViewMode == VMI_BrushWireframe)
				{
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe>::GetRHI();
				}
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *PassParameters);
				SetShaderParameters(RHICmdList, GeometryShader, GeometryShader.GetPixelShader(), *PassParameters);
				SetShaderParameters(RHICmdList, FragmentShader, FragmentShader.GetPixelShader(), *PassParameters);

				FVertexInputStreamArray InputStreams;
				StaticMesh.VertexFactory->GetStreams(GMaxRHIFeatureLevel, EVertexInputStreamType::PositionOnly, InputStreams);
				for (const FVertexInputStream& InputStream : InputStreams)
				{
					RHICmdList.SetStreamSource(InputStream.StreamIndex, InputStream.VertexBuffer, InputStream.Offset);
				}

				for (FMeshBatchElement& BatchElement : StaticMesh.Elements)
				{
					 RHICmdList.DrawIndexedPrimitive(
						   BatchElement.IndexBuffer->IndexBufferRHI,
						   /*BaseVertexIndex=*/ BatchElement.BaseVertexIndex,
						   /*MinIndex=*/ BatchElement.MinVertexIndex,
						   /*NumVertices=*/ StaticMesh.GetNumPrimitives() * 3,
						   /*StartIndex=*/ BatchElement.FirstIndex,
						   /*NumPrimitives=*/ BatchElement.NumPrimitives,
						   /*NumInstances=*/ BatchElement.NumInstances
					 );
				}
			}
		}
	});
}

void FZenoGrassViewExtension::CleanGroundPrimitives()
{
}

IMPLEMENT_GLOBAL_SHADER(FGrassShaderVS, "/ZenoEngine/Private/Grass.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FGrassShaderGS, "/ZenoEngine/Private/Grass.usf", "MainGS", SF_Geometry);
IMPLEMENT_GLOBAL_SHADER(FGrassShaderFS, "/ZenoEngine/Private/Grass.usf", "MainPS", SF_Pixel);
