// Author: DarcJC


#include "ZenoTestMeshActor.h"

#include "ZenoGrassViewExtension.h"
#include "ZenoWorldSubsystem.h"


// Sets default values
AZenoTestMeshActor::AZenoTestMeshActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZenoTestMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
	UZenoWorldSubsystem* ZenoWorldSubsystem = GetWorld()->GetSubsystem<UZenoWorldSubsystem>();
	if (IsValid(GetStaticMeshComponent()))
	{
		ZenoWorldSubsystem->GetGrassViewExtension()->AddGroundPrimitiveComponent_RenderThread(GetStaticMeshComponent());
	}
}

// Called every frame
void AZenoTestMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

