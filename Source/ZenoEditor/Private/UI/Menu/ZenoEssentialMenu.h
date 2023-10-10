#pragma once
#include "UI/Menu/ZenoEditorMenuExtender.h"

class FZenoEssentialMenu final : public FZenoEditorExtenderServiceBase, public TGetFromThis<FZenoEssentialMenu>
{
public:
	// +Interface from IZenoEditorExtenderService
	virtual void Register() override;
	virtual void Unregister() override;
	virtual void MapAction() override;
	// -Interface from IZenoEditorExtenderService

	void ExtendMenuBar(FMenuBarBuilder& Builder);
	void ExtendPullDownMenu(FMenuBuilder& Builder);

	void Action_ImportSplineData();
};
