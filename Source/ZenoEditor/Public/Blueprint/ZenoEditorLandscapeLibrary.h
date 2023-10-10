#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoEditorLandscapeLibrary.generated.h"

class ALandscapeProxy;
class ALandscape;
class ILandscapeSplineInterface;
class ULandscapeSplinesComponent;
class ULandscapeSplineControlPoint;

UCLASS()
class ZENOEDITOR_API UZenoEditorLandscapeLibrary : public UObject
{
	GENERATED_BODY()

public:
	static void ExpandHeightmapData(const TArray<uint16>& InHeightMap, const FIntVector2& InTargetResolution,
	                                TArray<uint16>& OutHeightMap);

	static void MakeLandscapeActorStreamable(ALandscape* LandscapeTarget);

	static bool IsGridBased(const UWorld* World);

	static ILandscapeSplineInterface* SetupSplineForLandscape(ALandscapeProxy* Landscape);

	static void SnapControlPointToGround(ULandscapeSplineControlPoint* ControlPoint, bool bEnableTransaction = false);

	static void Thief_AddControlPointToSplineComponent(ULandscapeSplinesComponent* SplinesComponent, ULandscapeSplineControlPoint* ControlPoint);
};
