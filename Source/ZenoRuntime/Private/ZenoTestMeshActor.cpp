// Author: DarcJC


#include "ZenoTestMeshActor.h"

#include "EngineUtils.h"
#include "LandscapeProxy.h"
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

	UWorld* World = GetWorld();
	check(World);
	
	UZenoWorldSubsystem* ZenoWorldSubsystem = World->GetSubsystem<UZenoWorldSubsystem>();
	if (IsValid(GetStaticMeshComponent()))
	{
		// ENQUEUE_RENDER_COMMAND(AddGround)([ZenoWorldSubsystem, this] (FRHICommandList&)
		// {
			ZenoWorldSubsystem->GetGrassViewExtension()->AddGroundPrimitiveComponent_RenderThread(GetStaticMeshComponent());
		// });
	}

	// ENQUEUE_RENDER_COMMAND(AddGroundLandscape)([this, World, ZenoWorldSubsystem] (FRHICommandList&)
	// {
		// for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
		// {
		// 	TArray<UPrimitiveComponent*> Components;
		// 	It->GetComponents(UPrimitiveComponent::StaticClass(), Components);
		// 	for (UPrimitiveComponent* Component : Components)
		// 	{
		// 		ZenoWorldSubsystem->GetGrassViewExtension()->AddGroundPrimitiveComponent_RenderThread(Component);
		// 	}
		// }
	// });
}

// Called every frame
void AZenoTestMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

