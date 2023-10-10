#pragma once

class FZenoEditorCommand : public TCommands<FZenoEditorCommand>
{
public:
	FZenoEditorCommand();
	
	virtual void RegisterCommands() override;

public:

	// Debug
	TSharedPtr<FUICommandInfo> Debug;
	// Landscape
	TSharedPtr<FUICommandInfo> ImportHeightField;
	TSharedPtr<FUICommandInfo> ExportHeightField;
	// Import
	TSharedPtr<FUICommandInfo> LandscapeSplineImport;
};
