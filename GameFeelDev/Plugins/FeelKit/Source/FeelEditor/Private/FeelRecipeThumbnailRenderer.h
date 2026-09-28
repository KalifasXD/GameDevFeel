// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "FeelRecipeThumbnailRenderer.generated.h"

/**
 * Draws a recipe's Content Browser tile as a small picture of its timeline: a band in the recipe's feeling
 * color, one bar per track in its channel color shaped by its intensity curve, the sustain region, the length and a lock
 * on library recipes. It draws on a canvas instead of rendering a scene, so folders full of recipes stay fast.
 */
UCLASS()
class UFeelRecipeThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily) override;
};
