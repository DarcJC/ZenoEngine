// Author: DarcJC

#pragma once

#include "CoreMinimal.h"
#include "LandscapeSplineActor.h"
#include "UObject/Object.h"
#include "ZenoEditorTypes.generated.h"

class ALandscapeProxy;

UCLASS()
class ZENOEDITOR_API UZenoLandscapeSplineImportSettings : public UObject
{
	GENERATED_BODY()

public:
	// Target landscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Target Landscape")
	TSoftObjectPtr<ALandscapeProxy> Landscape;
	
	// File Path
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Spline File Import Path")
	FFilePath FilePath;

	// Snap to landscape (turn on if the spline is not matching the height of the terrain)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Snap to landscape", Category = "Options")
	bool bSnapToLandscape = true;

	// Half width will random in range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Half Width Range", Category = "Options")
	FVector2D HalfWidthRange = FVector2D(1500.f, 2500.f);

	// Should spline mesh cast shadow on landscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Cast Shadow", Category = "Options")
	bool bCastShadow = false;

	// The layer this spline blend on the landscape
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Layer Name", Category = "Options")
	FName LayerName = EName::None;
	
};
