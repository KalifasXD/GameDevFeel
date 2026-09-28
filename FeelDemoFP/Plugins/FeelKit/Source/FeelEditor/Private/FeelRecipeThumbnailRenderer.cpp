// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeThumbnailRenderer.h"

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "FeelRecipe.h"
#include "FeelThumbnailLayout.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelRecipeThumbnailRenderer)

namespace FeelRecipeThumbnail
{
	/** Sequencer's colors (UE 5.6): time around the recipe, the recipe's own time, and the grid lines. */
	FLinearColor Srgb(uint8 R, uint8 G, uint8 B, float Alpha = 1.0f) { return FLinearColor(FColor(R, G, B)).CopyWithNewOpacity(Alpha); }

	void DrawRect(FCanvas& Canvas, float X, float Y, float Width, float Height, const FLinearColor& Color)
	{
		if (Width <= 0.0f || Height <= 0.0f || Color.A <= 0.0f)
		{
			return;
		}
		FCanvasTileItem Tile(FVector2D(X, Y), FVector2D(FMath::Max(Width, 1.0f), FMath::Max(Height, 1.0f)), Color);
		Tile.BlendMode = SE_BLEND_AlphaBlend;
		Canvas.DrawItem(Tile);
	}

	void DrawLine(FCanvas& Canvas, const FVector2D& From, const FVector2D& To, const FLinearColor& Color, float Thickness)
	{
		FCanvasLineItem Line(From, To);
		Line.SetColor(Color);
		Line.LineThickness = Thickness;
		Line.BlendMode = SE_BLEND_AlphaBlend;
		Canvas.DrawItem(Line);
	}
}

bool UFeelRecipeThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Cast<UFeelRecipe>(Object) != nullptr;
}

void UFeelRecipeThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Viewport, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	const UFeelRecipe* Recipe = Cast<UFeelRecipe>(Object);
	if (!Recipe || !Canvas || Width == 0 || Height == 0)
	{
		return;
	}

	using namespace FeelRecipeThumbnail;

	// A tiny Sequencer, drawn the way Unreal's own data thumbnails are (curves, sound waves): only the content, no card and
	// no text. The Content Browser adds the name, the type and the asset color line under the tile itself.
	const float Size = static_cast<float>(FMath::Min(Width, Height));
	const bool bSmall = Size < 72.0f;
	const FFeelThumbnailLayout Layout = FeelThumbnailLayout::Build(*Recipe, bSmall ? 4 : 6, 24);

	const float Left = static_cast<float>(X);
	const float Top = static_cast<float>(Y);
	const float FullWidth = static_cast<float>(Width);
	const float FullHeight = static_cast<float>(Height);
	const float Hairline = FMath::Max(Size / 128.0f, 1.0f);

	DrawRect(*Canvas, Left, Top, FullWidth, FullHeight, Srgb(25, 25, 25));

	// The recipe's feeling: a stripe of Unreal's accent color down the left edge.
	const float StripeWidth = FMath::Max(Size * 0.03f, 2.0f);
	DrawRect(*Canvas, Left, Top, StripeWidth, FullHeight, Layout.FeelingColor);

	const float Pad = FMath::Max(Size * 0.08f, 3.0f);
	const float AreaLeft = Left + StripeWidth + Pad;
	const float AreaRight = Left + FullWidth - Pad;
	const float AreaWidth = FMath::Max(AreaRight - AreaLeft, 1.0f);
	const float AreaTop = Top + Pad;
	const float AreaBottom = Top + FullHeight - Pad;
	const float AreaHeight = FMath::Max(AreaBottom - AreaTop, 1.0f);

	// The recipe's time is lighter than the time around it, with dark grid lines at the quarters, as in Sequencer.
	DrawRect(*Canvas, AreaLeft, Top, AreaWidth, FullHeight, Srgb(36, 36, 36));
	for (int32 Step = 1; Step < 4; ++Step)
	{
		DrawRect(*Canvas, AreaLeft + AreaWidth * (Step * 0.25f), Top, Hairline, FullHeight, Srgb(25, 25, 25));
	}
	DrawRect(*Canvas, AreaRight, Top, Hairline, FullHeight, Srgb(128, 32, 32));

	// One section per track, in Sequencer's track colors, with the intensity curve as a faint line inside.
	const int32 NumBars = Layout.Bars.Num();
	const float Gap = FMath::Max(Size * 0.025f, 1.0f);
	const float MaxLaneHeight = FMath::Max(Size * 0.12f, 4.0f);
	const float LaneHeight = NumBars > 0
		? FMath::Min(MaxLaneHeight, FMath::Max((AreaHeight - Gap * (NumBars - 1)) / static_cast<float>(NumBars), 3.0f))
		: MaxLaneHeight;
	const float UsedHeight = NumBars > 0 ? LaneHeight * NumBars + Gap * (NumBars - 1) : 0.0f;
	const float LanesTop = AreaTop + FMath::Max((AreaHeight - UsedHeight) * 0.5f, 0.0f);

	for (int32 BarIndex = 0; BarIndex < NumBars; ++BarIndex)
	{
		const FFeelThumbnailBar& Bar = Layout.Bars[BarIndex];
		const float LaneTop = LanesTop + (LaneHeight + Gap) * static_cast<float>(BarIndex);
		const float BarLeft = AreaLeft + Bar.StartX * AreaWidth;
		const float BarWidth = FMath::Max((Bar.EndX - Bar.StartX) * AreaWidth, Hairline * 2.0f);
		DrawRect(*Canvas, BarLeft, LaneTop, BarWidth, LaneHeight, Bar.bFaded ? Srgb(71, 71, 71) : Bar.Color);

		const int32 NumSamples = Bar.Heights.Num();
		if (!bSmall && NumSamples > 1 && LaneHeight >= 6.0f)
		{
			FVector2D Previous;
			for (int32 SampleIndex = 0; SampleIndex < NumSamples; ++SampleIndex)
			{
				const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(NumSamples - 1);
				const float Value = FMath::Clamp(Bar.Heights[SampleIndex], 0.0f, 1.0f);
				const FVector2D Point(BarLeft + Alpha * BarWidth, LaneTop + LaneHeight - Hairline - Value * (LaneHeight - 2.0f * Hairline));
				if (SampleIndex > 0)
				{
					DrawLine(*Canvas, Previous, Point, FLinearColor(1.0f, 1.0f, 1.0f, 0.3f), Hairline);
				}
				Previous = Point;
			}
		}
	}

	if (Layout.bLibrary && !bSmall)
	{
		// A small padlock marks recipes that ship with FeelKit: they are copied, not edited in place.
		const float LockSize = FMath::Max(Size * 0.08f, 6.0f);
		const float LockLeft = Left + FullWidth - Pad * 0.5f - LockSize;
		const float LockTop = Top + Pad * 0.4f;
		const FLinearColor LockColor = Srgb(192, 192, 192, 0.8f);
		const float Bar = FMath::Max(LockSize * 0.18f, 1.0f);
		DrawRect(*Canvas, LockLeft + LockSize * 0.22f, LockTop, Bar, LockSize * 0.42f, LockColor);
		DrawRect(*Canvas, LockLeft + LockSize * 0.78f - Bar, LockTop, Bar, LockSize * 0.42f, LockColor);
		DrawRect(*Canvas, LockLeft + LockSize * 0.22f, LockTop, LockSize * 0.56f, Bar, LockColor);
		DrawRect(*Canvas, LockLeft, LockTop + LockSize * 0.38f, LockSize, LockSize * 0.62f, LockColor);
	}
}
