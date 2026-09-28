// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FFeelRecipeEditorState;
class IDetailsView;

/**
 * Details layout inside the recipe editor.
 * With no track selected it shows the recipe settings; with a track selected it shows that track and its step.
 * Editing goes through the recipe object, so undo and step ownership work as for any asset property.
 */
class FFeelRecipeDetails : public IDetailCustomization
{
public:
	explicit FFeelRecipeDetails(const TWeakPtr<FFeelRecipeEditorState>& InState);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	/** Registers the recipe layout and FeelKit's curve rows on a details view that shows a recipe. */
	static void RegisterLayouts(IDetailsView& DetailsView, const TWeakPtr<FFeelRecipeEditorState>& State);

private:
	TWeakPtr<FFeelRecipeEditorState> State;
};
