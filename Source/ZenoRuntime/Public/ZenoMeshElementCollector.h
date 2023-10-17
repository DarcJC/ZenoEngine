#pragma once

class FZenoMeshElementCollector : public FMeshElementCollector
{
public:
	FZenoMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel, FSceneRenderingBulkObjectAllocator& InBulkAllocator);
	using FMeshElementCollector::DeleteTemporaryProxies;
};
