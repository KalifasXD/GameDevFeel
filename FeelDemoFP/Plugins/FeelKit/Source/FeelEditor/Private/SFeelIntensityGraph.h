// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class FFeelRecipeEditorState;

/**
 * Intensity graph: each channel's combined intensity across the recipe, with the playhead.
 * Respects mute, solo and the preview comfort preset, so it shows what the preview plays.
 */
class SFeelIntensityGraph : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelIntensityGraph) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TSharedPtr<FFeelRecipeEditorState> State;
};
