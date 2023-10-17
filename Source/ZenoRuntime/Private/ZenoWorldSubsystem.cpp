// Author: DarcJC


#include "ZenoWorldSubsystem.h"

#include "SceneViewExtension.h"

TSharedRef<FZenoGrassViewExtension> UZenoWorldSubsystem::GetGrassViewExtension() const
{
	return GrassViewExtension.ToSharedRef();
}

void UZenoWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	check(nullptr != World);

	GrassViewExtension = FSceneViewExtensions::NewExtension<FZenoGrassViewExtension>(World);
}

void UZenoWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
