#include "ZenoEditorCommand.h"

#include "UI/Style/ZenoEditorWidgetStyle.h"

#define LOCTEXT_NAMESPACE "FZenoEditorCommand"

FZenoEditorCommand::FZenoEditorCommand()
	: TCommands(
		TEXT("ZenoEditorCommand"),
		LOCTEXT("CommandName", "Zeno Editor"),
		NAME_None,
		FZenoEditorWidgetStyle::TypeName)
{
}

void FZenoEditorCommand::RegisterCommands()
{
	UI_COMMAND(Debug, "Debug", "Debug", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportHeightField, "Import", "Import height field subject from zeno.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportHeightField, "Export", "Export height field data to zeno.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(LandscapeSplineImport, "Import Spline", "Import .zspline into LandscapeSplineActor", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
