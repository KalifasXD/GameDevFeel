// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UFeelRecipe;

/** One track drawn in a recipe thumbnail: a bar over the recipe's length, shaped by the track's intensity curve. */
struct FFeelThumbnailBar
{
	/** Channel color of the track. */
	FLinearColor Color = FLinearColor::White;

	/** Start and end of the track as fractions of the recipe length (0 to 1). */
	float StartX = 0.0f;
	float EndX = 0.0f;

	/** Intensity samples across the bar (0 to 1), evenly spaced from its start to its end. */
	TArray<float, TInlineAllocator<16>> Heights;

	/** Muted or stepless tracks are drawn faded. */
	bool bFaded = false;
};

/** Everything a recipe thumbnail draws, computed without any rendering so it can be tested. */
struct FFeelThumbnailLayout
{
	TArray<FFeelThumbnailBar> Bars;

	/** Tracks that did not fit, shown as "+N". */
	int32 HiddenTrackCount = 0;

	bool bHasSustain = false;
	float SustainStartX = 0.0f;
	float SustainEndX = 0.0f;

	/** Recipe length in seconds. */
	float Length = 0.0f;

	/** Color of the recipe's feeling; gray when it has none. */
	FLinearColor FeelingColor = FLinearColor::White;

	/** Library recipes are marked with a lock. */
	bool bLibrary = false;
};

namespace FeelThumbnailLayout
{
	/** Builds the drawing data for a recipe. Tracks beyond MaxBars are counted in HiddenTrackCount. */
	FFeelThumbnailLayout Build(const UFeelRecipe& Recipe, int32 MaxBars = 8, int32 SamplesPerBar = 16);
}
