// Copyright 2026 Billo. All Rights Reserved.

#include "SFeelIntensityGraph.h"

#include "FeelEditorColors.h"
#include "FeelEvaluator.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

namespace FeelIntensityGraph
{
	constexpr float LeftMargin = 36.0f;
	constexpr float RightMargin = 12.0f;
	constexpr float TopMargin = 22.0f;
	constexpr float BottomMargin = 18.0f;
}

void SFeelIntensityGraph::Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState)
{
	State = InState;
	SetClipping(EWidgetClipping::ClipToBounds);
}

FVector2D SFeelIntensityGraph::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return FVector2D(300.0, 120.0);
}

int32 SFeelIntensityGraph::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	using namespace FeelIntensityGraph;

	const FVector2f Size = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SmallFont = FAppStyle::GetFontStyle(TEXT("SmallFont"));
	const ESlateDrawEffect Effect = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	auto DrawBox = [&](int32 Layer, float X, float Y, float W, float H, const FLinearColor& Color)
	{
		if (W > 0.0f && H > 0.0f)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(W, H), FSlateLayoutTransform(FVector2f(X, Y))), WhiteBrush, Effect, Color);
		}
	};
	auto DrawText = [&](int32 Layer, float X, float Y, const FString& Text, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeText(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(FMath::Max(Size.X - X, 1.0f), 16.0f), FSlateLayoutTransform(FVector2f(X, Y))), Text, SmallFont, Effect, Color);
	};
	auto DrawLine = [&](int32 Layer, const FVector2f& A, const FVector2f& B, const FLinearColor& Color, float Thickness)
	{
		TArray<FVector2f> Points;
		Points.Add(A);
		Points.Add(B);
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), MoveTemp(Points), Effect, Color, true, Thickness);
	};

	DrawBox(LayerId, 0.0f, 0.0f, Size.X, Size.Y, FLinearColor(0.018f, 0.018f, 0.018f));

	const UFeelRecipe* Recipe = State->GetRecipe();
	// This preview play's length, random durations included.
	const float Length = Recipe ? State->GetPlaybackLength() : 0.0f;
	const float PlotLeft = LeftMargin;
	const float PlotTop = TopMargin;
	const float PlotWidth = FMath::Max(Size.X - LeftMargin - RightMargin, 1.0f);
	const float PlotHeight = FMath::Max(Size.Y - TopMargin - BottomMargin, 1.0f);

	if (!Recipe || Length <= 0.0f)
	{
		DrawText(LayerId + 1, 10.0f, 8.0f, LOCTEXT("GraphEmpty", "Add tracks to see each channel's combined intensity over time.").ToString(), FLinearColor(0.6f, 0.6f, 0.6f));
		return LayerId + 2;
	}

	const int32 NumSamples = FMath::Clamp(FMath::FloorToInt32(PlotWidth / 3.0f), 16, 512);
	TArray<FFeelChannelIntensity> Channels;
	FFeelEvaluator::SampleChannelIntensities(*Recipe, NumSamples, State->GetPreviewParams(), Channels);

	float MaxValue = 1.0f;
	for (const FFeelChannelIntensity& Channel : Channels)
	{
		for (float Sample : Channel.Samples)
		{
			MaxValue = FMath::Max(MaxValue, Sample);
		}
	}

	auto ValueToY = [&](float Value) { return PlotTop + PlotHeight - (FMath::Max(Value, 0.0f) / MaxValue) * PlotHeight; };

	// Grid and labels.
	for (float GridValue : { 0.0f, 0.5f, 1.0f, MaxValue })
	{
		const float Y = ValueToY(GridValue);
		DrawLine(LayerId + 1, FVector2f(PlotLeft, Y), FVector2f(PlotLeft + PlotWidth, Y), FLinearColor(1.0f, 1.0f, 1.0f, GridValue == 0.0f ? 0.25f : 0.08f), 1.0f);
		DrawText(LayerId + 1, 4.0f, Y - 7.0f, FString::Printf(TEXT("%.1f"), GridValue), FLinearColor(0.55f, 0.55f, 0.55f));
	}
	DrawText(LayerId + 1, PlotLeft, PlotTop + PlotHeight + 2.0f, TEXT("0s"), FLinearColor(0.55f, 0.55f, 0.55f));
	DrawText(LayerId + 1, PlotLeft + PlotWidth - 34.0f, PlotTop + PlotHeight + 2.0f, FString::Printf(TEXT("%.2fs"), Length), FLinearColor(0.55f, 0.55f, 0.55f));

	// Channel curves and legend.
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	float LegendX = PlotLeft;
	for (const FFeelChannelIntensity& Channel : Channels)
	{
		const FLinearColor Color = FeelEditorColors::GetChannelColor(Channel.Channel);

		TArray<FVector2f> Points;
		Points.Reserve(Channel.Samples.Num());
		for (int32 SampleIndex = 0; SampleIndex < Channel.Samples.Num(); ++SampleIndex)
		{
			const float Alpha = static_cast<float>(SampleIndex) / static_cast<float>(Channel.Samples.Num() - 1);
			Points.Add(FVector2f(PlotLeft + Alpha * PlotWidth, ValueToY(Channel.Samples[SampleIndex])));
		}
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 2, AllottedGeometry.ToPaintGeometry(), MoveTemp(Points), Effect, Color, true, 2.0f);

		const FString Label = Channel.Channel.IsValid() ? Channel.Channel.ToString() : LOCTEXT("NoChannel", "(no channel)").ToString();
		DrawBox(LayerId + 3, LegendX, 6.0f, 10.0f, 10.0f, Color);
		DrawText(LayerId + 3, LegendX + 14.0f, 3.0f, Label, FLinearColor(0.8f, 0.8f, 0.8f));
		LegendX += 14.0f + FontMeasure->Measure(Label, SmallFont).X + 16.0f;
	}

	// Playhead.
	const float PlayheadX = PlotLeft + FMath::Clamp(State->GetTime() / Length, 0.0f, 1.0f) * PlotWidth;
	DrawLine(LayerId + 4, FVector2f(PlayheadX, PlotTop), FVector2f(PlayheadX, PlotTop + PlotHeight), FLinearColor(1.0f, 0.3f, 0.2f), 2.0f);

	return LayerId + 5;
}

#undef LOCTEXT_NAMESPACE
