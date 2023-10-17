// Author: DarcJC

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "ZenoTestMeshActor.generated.h"

UCLASS()
class ZENORUNTIME_API AZenoTestMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZenoTestMeshActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
