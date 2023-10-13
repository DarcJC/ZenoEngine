#include "ZenoEssentialMenu.h"

#include <array>
#include <fstream>

#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeSplineControlPoint.h"
#include "LandscapeSplinesComponent.h"
#include "Thief.h"
#include "ZenoEditor.h"
#include "ZenoEditorCommand.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Blueprint/ZenoEditorLandscapeLibrary.h"
#include "Blueprint/ZenoEditorTypes.h"

#define LOCTEXT_NAMESPACE "FZenoEssentialMenu"

void FZenoEssentialMenu::Register()
{
	FZenoEditorExtenderServiceBase::Register();
	MenuBarExtender->AddMenuBarExtension(
		"Help",
		EExtensionHook::Before,
		CommandList,
		FMenuBarExtensionDelegate::CreateRaw(this, &FZenoEssentialMenu::ExtendMenuBar));
}

void FZenoEssentialMenu::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FZenoEssentialMenu::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().LandscapeSplineImport,
	                       FExecuteAction::CreateRaw(this, &FZenoEssentialMenu::Action_ImportSplineData));
}

void FZenoEssentialMenu::ExtendMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(
		LOCTEXT("Zeno", "Zeno"),
		LOCTEXT("MenuTooltip", "Zeno utilities"),
		FNewMenuDelegate::CreateRaw(this, &FZenoEssentialMenu::ExtendPullDownMenu),
		ZenoHookLabel,
		ZenoHookLabel);
}

void FZenoEssentialMenu::ExtendPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuSeparator("Terrain");
	Builder.AddMenuEntry(FZenoEditorCommand::Get().LandscapeSplineImport);
}

void FZenoEssentialMenu::Action_ImportSplineData()
{
	UZenoLandscapeSplineImportSettings* Settings = NewObject<UZenoLandscapeSplineImportSettings>();
	if (!UZenoCommonBlueprintLibrary::OpenSettingsModal(Settings, LOCTEXT("SplineImport", "Spline Import")))
	{
		UE_LOG(ZenoEditor, Display, TEXT("[SplineImport] Aborted."));
		return;
	}

	// Check has selected landscape
	if (!Settings->Landscape.IsValid())
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("InvalidLandscape",
		                                                      "You must select a landscape instance first."));
		return;
	}

	// Read file as bytes
	TArray<uint8> ByteArray;
	ByteArray.Reserve(1024 * 3 + 32); // Default reserve for 1024 points
	// std::ifstream ByteStream(*Settings->FilePath.FilePath, std::ios::in | std::ios::binary);
	// for (std::istreambuf_iterator<char> Iter (ByteStream); Iter != std::istreambuf_iterator<char>(); ++Iter)
	// {
	// 	ByteArray.Add(static_cast<uint8>(*Iter));
	// }
	FFileHelper::LoadFileToArray(ByteArray, *Settings->FilePath.FilePath);

	// Check minimum size
	if (ByteArray.Num() < 32)
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("BadFileSize", "The file been selected seems broken."));
		return;
	}

	// Check header
	static FString RequiredFileHeader = "zeno2020";
	static SIZE_T FileMarkerLen = RequiredFileHeader.Len();
	const FString FileHeader(FileMarkerLen, reinterpret_cast<ANSICHAR*>(ByteArray.GetData()));
	if (FileHeader != RequiredFileHeader)
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("NotASplineFile",
		                                                      "The file been selected isn't a zespline file."));
		return;
	}

	// Get meta data
	const uint64 PointNum = *reinterpret_cast<uint64*>(ByteArray.GetData() + FileMarkerLen + 1); // PointNum: Base + 8

	// Check data integrity
	if (PointNum == 0)
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("EmptyFile", "The file is empty."));
		return;
	}
	const uint64 SizeExcepted = PointNum * sizeof(std::array<float, 3>) + 32; // Body + Header(32)
	if (ByteArray.Num() < SizeExcepted)
	{
		UZenoCommonBlueprintLibrary::ShowNotification(LOCTEXT("BadFileIntegrity", "The file contains broken data."));
		return;
	}

	// Get points data
	TArray<std::array<float, 3>> PositionData;
	PositionData.SetNumUninitialized(PointNum);
	FMemory::ParallelMemcpy(PositionData.GetData(), ByteArray.GetData() + 32, PointNum * sizeof(std::array<float, 3>));

	// Get a spline owner
	ILandscapeSplineInterface* SplineOwner = UZenoEditorLandscapeLibrary::SetupSplineForLandscape(
		Settings->Landscape.Get());
	ULandscapeSplinesComponent* SplinesComponent = SplineOwner->GetSplinesComponent();

	// Create a new SplineComponent
	// Only if SplineOwner is a ALandscapeProxy might get into this branch
	if (!SplinesComponent)
	{
		FScopedTransaction Transaction(LOCTEXT("CreateSplineComponent", "Create Spline Component"));
		SplineOwner->CreateSplineComponent();
		SplinesComponent = SplineOwner->GetSplinesComponent();
		check(SplinesComponent);
	}

	// Add points to landscape
	{
		FScopedTransaction Transaction(LOCTEXT("CreateNewSpline", "Create New Spline"));
		SplinesComponent->Modify();
		ULandscapeSplineControlPoint* LastControlPoint = nullptr;
		ULandscapeSplineSegment* LastSegment = nullptr;
		for (int32 i = 0; i < PointNum; ++i)
		{
			auto& Point = PositionData[i]; // Y-axis is upward
			ULandscapeSplineControlPoint* NewControlPoint = NewObject<ULandscapeSplineControlPoint>(
				SplinesComponent, EName::None, RF_Transactional);
			SplinesComponent->GetControlPoints().Add(NewControlPoint);

			NewControlPoint->Modify();

			FVector PointLocation(Point.at(0), Point.at(2), Point.at(1)); // Inverse
			// PointLocation = Settings->Landscape->LandscapeActorToWorld().TransformPosition(PointLocation);
			FVector Scale = Settings->Landscape->GetActorScale();
			PointLocation = PointLocation * Scale;
			NewControlPoint->Location = PointLocation;

			const double HalfWidth = FMath::RandRange(Settings->HalfWidthRange.X, Settings->HalfWidthRange.Y);
			NewControlPoint->Width = HalfWidth;

			NewControlPoint->bCastShadow = Settings->bCastShadow;
			NewControlPoint->LayerName = Settings->LayerName;

			if (Settings->bSnapToLandscape)
			{
				UZenoEditorLandscapeLibrary::SnapControlPointToGround(NewControlPoint, false);
			}

			// Control two control points
			if (nullptr != LastControlPoint)
			{
				ULandscapeSplineSegment* NewSegment = NewObject<ULandscapeSplineSegment>(
					SplinesComponent, NAME_None, RF_Transactional);
				SplinesComponent->GetSegments().Add(NewSegment);

				NewSegment->Connections[0].ControlPoint = LastControlPoint;
				NewSegment->Connections[1].ControlPoint = NewControlPoint;

				NewSegment->Connections[0].SocketName = LastControlPoint->
					GetBestConnectionTo(NewControlPoint->Location);
				NewSegment->Connections[1].SocketName = NewControlPoint->
					GetBestConnectionTo(LastControlPoint->Location);

				FVector StartLocation;
				FRotator StartRotation;
				LastControlPoint->GetConnectionLocationAndRotation(NewSegment->Connections[0].SocketName,
				                                                   StartLocation,
				                                                   StartRotation);
				FVector EndLocation;
				FRotator EndRotation;
				NewControlPoint->GetConnectionLocationAndRotation(NewSegment->Connections[1].SocketName,
				                                                  EndLocation,
				                                                  EndRotation);

				NewSegment->Connections[0].TangentLen = static_cast<float>((EndLocation - StartLocation).Size());
				NewSegment->Connections[1].TangentLen = NewSegment->Connections[0].TangentLen;

				NewSegment->Connections[0].ControlPoint->ConnectedSegments.Add(FLandscapeSplineConnection(NewSegment, 0));
				NewSegment->Connections[1].ControlPoint->ConnectedSegments.Add(FLandscapeSplineConnection(NewSegment, 1));

				NewSegment->AutoFlipTangents();

				LastControlPoint->AutoCalcRotation();
				LastControlPoint->UpdateSplinePoints();
				NewControlPoint->AutoCalcRotation();
				NewControlPoint->UpdateSplinePoints();

				NewSegment->UpdateSplinePoints();

				LastSegment = NewSegment;
			}

			LastControlPoint = NewControlPoint;
		}
	}

	// Feedback to user viewport
	if (ALandscape* Landscape = Cast<ALandscape>(Settings->Landscape.Get()); Landscape && Landscape->HasLayersContent()
		&& Landscape->GetLandscapeSplinesReservedLayer())
	{
		FScopedTransaction Transaction(LOCTEXT("UpdateSplineLayer", "Update Landscape Spline Reserved Layer"));
		Landscape->RequestSplineLayerUpdate();
	}

	// Synchronizing with rendering
	if (!SplinesComponent->IsRegistered())
	{
		SplinesComponent->RegisterComponent();
	}
	else
	{
		SplinesComponent->MarkRenderStateDirty();
	}

	GEditor->RedrawLevelEditingViewports();
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("Zeno", FZenoEssentialMenu);
