// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

class UFeelSubsystem;

/** One widget over the game view that draws every floating text of a world's Number Pop steps. */
class SFeelNumberPopLayer : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelNumberPopLayer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UFeelSubsystem* InSubsystem);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override { return FVector2D::ZeroVector; }
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TWeakObjectPtr<UFeelSubsystem> Subsystem;
};
