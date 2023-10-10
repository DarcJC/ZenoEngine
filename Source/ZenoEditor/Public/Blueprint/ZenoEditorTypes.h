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
};
