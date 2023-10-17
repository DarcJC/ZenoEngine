// Author: DarcJC

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ZenoWorldSubsystem.generated.h"


class FZenoGrassViewExtension;

UCLASS()
class ZENORUNTIME_API UZenoWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	TSharedRef<FZenoGrassViewExtension> GetGrassViewExtension() const;
	
protected:
	// +Implementation of USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// -Implementation of USubsystem

private:
	TSharedPtr<FZenoGrassViewExtension> GrassViewExtension;
};
