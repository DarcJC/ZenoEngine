// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoEditorLandscapeLibrary.h"

#include "Landscape.h"
#include "LandscapeImportHelper.h"
#include "LandscapeProxy.h"
#include "LandscapeSplineActor.h"
#include "LandscapeSplineControlPoint.h"
#include "LandscapeSplinesComponent.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeSubsystem.h"
#include "Thief.h"
#include "ActorPartition/ActorPartitionSubsystem.h"

#define LOCTEXT_NAMESPACE "UZenoEditorLandscapeLibrary"

void UZenoEditorLandscapeLibrary::ExpandHeightmapData(const TArray<uint16>& InHeightMap,
                                                      const FIntVector2& InTargetResolution, TArray<uint16>& OutHeightMap)
{
	check(InHeightMap != OutHeightMap);

	const int32 Size = sqrt(InHeightMap.Num());
	const FLandscapeImportResolution ImportResolution(Size, Size);
	const FLandscapeImportResolution TargetResolution(InTargetResolution.X, InTargetResolution.Y);

	FLandscapeImportHelper::TransformHeightmapImportData(InHeightMap, OutHeightMap, ImportResolution,
		TargetResolution,
		ELandscapeImportTransformType::Resample);
}

void UZenoEditorLandscapeLibrary::MakeLandscapeActorStreamable(ALandscape* LandscapeTarget)
{
	check(LandscapeTarget);
	ULandscapeInfo* LandscapeInfo = LandscapeTarget->GetLandscapeInfo();
	check(LandscapeInfo);
	
	LandscapeTarget->Modify();

	LandscapeTarget->InitializeProxyLayersWeightmapUsage();
	LandscapeTarget->bIncludeGridSizeInNameForLandscapeActors = true;

	FIntRect Extent;
	LandscapeInfo->GetLandscapeExtent(Extent.Min.X, Extent.Min.Y, Extent.Max.X, Extent.Max.Y);

	const UWorld* World = LandscapeTarget->GetWorld();
	check(World);

	UActorPartitionSubsystem* ActorPartitionSubsystem = World->GetSubsystem<UActorPartitionSubsystem>();
	check(ActorPartitionSubsystem);

	TArray<ULandscapeComponent*> LandscapeComponents;
	LandscapeComponents.Reserve(LandscapeInfo->XYtoComponentMap.Num());
	LandscapeInfo->ForAllLandscapeComponents([&LandscapeComponents](ULandscapeComponent* Component)
	{
		LandscapeComponents.Add(Component);
	});

	TSet<ALandscapeProxy*> ProxiesToDelete;
	FActorPartitionGridHelper::ForEachIntersectingCell(ALandscapeStreamingProxy::StaticClass(), Extent, World->PersistentLevel, [ActorPartitionSubsystem, LandscapeInfo, &LandscapeComponents, &ProxiesToDelete] (const UActorPartitionSubsystem::FCellCoord& CellCoord, const FIntRect& CellBounds)
	{
		
		TMap<ULandscapeComponent*, UMaterialInterface*> ComponentMaterials;
		TMap<ULandscapeComponent*, UMaterialInterface*> ComponentHoleMaterials;
		TMap <ULandscapeComponent*, TMap<int32, UMaterialInterface*>> ComponentLODMaterials;

		TArray<ULandscapeComponent*> ComponentsToMove;
		for (int32 i = 0; i < LandscapeComponents.Num();)
		{
			ULandscapeComponent* LandscapeComponent = LandscapeComponents[i];
			if (CellBounds.Contains(LandscapeComponent->GetSectionBase()))
			{
				ComponentMaterials.FindOrAdd(LandscapeComponent, LandscapeComponent->GetLandscapeMaterial());
				ComponentHoleMaterials.FindOrAdd(LandscapeComponent, LandscapeComponent->GetLandscapeHoleMaterial());
				TMap<int32, UMaterialInterface*>& LODMaterials = ComponentLODMaterials.FindOrAdd(LandscapeComponent);
				for (int32 LODIndex = 0; LODIndex <= 8; ++LODIndex)
				{
					LODMaterials.Add(LODIndex, LandscapeComponent->GetLandscapeMaterial(static_cast<int8>(LODIndex)));
				}

				ComponentsToMove.Add(LandscapeComponent);
				LandscapeComponents.RemoveAtSwap(i);
				ProxiesToDelete.Add(LandscapeComponent->GetTypedOuter<ALandscapeProxy>());
			}
			else
			{
				i++;
			}
		}

		if (ComponentsToMove.Num()) {}
		return true;
	});
}

bool UZenoEditorLandscapeLibrary::IsGridBased(const UWorld* World)
{
	check(IsValid(World));
	return World->GetSubsystem<ULandscapeSubsystem>()->IsGridBased();
}

ILandscapeSplineInterface* UZenoEditorLandscapeLibrary::SetupSplineForLandscape(ALandscapeProxy* Landscape)
{
	check(IsValid(Landscape));
	
	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	ensureMsgf(IsValid(LandscapeInfo), TEXT("Landscape has invalid LandscapeInfo object."));

	const UWorld* World = Landscape->GetWorld();
	const bool bIsGridBase = IsGridBased(World);

	ILandscapeSplineInterface* SplineOwner = Landscape;

	// Create ALandscapeSplineActor when using world partition
	if (bIsGridBase)
	{
		FScopedTransaction Transaction(LOCTEXT("CreateSplineActor", "Create Spline Owner"));
		const FVector NewActorLocation = Landscape->GetActorLocation();
		SplineOwner = LandscapeInfo->CreateSplineActor(NewActorLocation);
	}

	check(SplineOwner);

	return SplineOwner;
}

// From LandscapeEdModeSplineTools.cpp
void UZenoEditorLandscapeLibrary::SnapControlPointToGround(ULandscapeSplineControlPoint* ControlPoint, bool bEnableTransaction/** = false*/)
{
	int32 TransactionIndex = -1;
	if (bEnableTransaction)
	{
		TransactionIndex = GEditor->BeginTransaction(LOCTEXT("SnapControlPointToGround", "Snap Control Point To Ground"));
		check(TransactionIndex >= 0);
	}

	ULandscapeSplinesComponent* SplinesComponent = ControlPoint->GetOuterULandscapeSplinesComponent();
	SplinesComponent->Modify();
	ControlPoint->Modify();

	const FTransform LocalToWorld = SplinesComponent->GetComponentToWorld();
	const FVector Start = LocalToWorld.TransformPosition(ControlPoint->Location);
	const FVector EndDown = Start + FVector(0, 0, -HALF_WORLD_MAX);
	const FVector EndUp = Start + FVector(0, 0, HALF_WORLD_MAX);

	FHitResult HitResult;
	UWorld* World = SplinesComponent->GetWorld();
	check(World);
	if (World->LineTraceSingleByObjectType(HitResult, Start, EndDown, FCollisionObjectQueryParams(ECC_WorldStatic), FCollisionQueryParams(NAME_None, FCollisionQueryParams::GetUnknownStatId(), true)) || World->LineTraceSingleByObjectType(HitResult, Start, EndUp, FCollisionObjectQueryParams(ECC_WorldStatic), FCollisionQueryParams(NAME_None, FCollisionQueryParams::GetUnknownStatId(), true)))
	{
		ControlPoint->Location = LocalToWorld.InverseTransformPosition(HitResult.Location);
		ControlPoint->UpdateSplinePoints();
		SplinesComponent->MarkRenderStateDirty();
	}

	if (bEnableTransaction)
	{
		GEditor->EndTransaction();
	}
}

ACCESS_PRIVATE_FIELD(ULandscapeSplinesComponent, TArray<TObjectPtr<ULandscapeSplineControlPoint>>, ControlPoints);

void UZenoEditorLandscapeLibrary::Thief_AddControlPointToSplineComponent(ULandscapeSplinesComponent* SplinesComponent, ULandscapeSplineControlPoint* ControlPoint)
{
	auto& Steal = access_private::ControlPoints(*SplinesComponent);
	Steal.Add(ControlPoint);
}

#undef LOCTEXT_NAMESPACE
