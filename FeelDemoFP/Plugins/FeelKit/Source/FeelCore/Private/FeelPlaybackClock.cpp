// Copyright 2026 Billo. All Rights Reserved.

#include "FeelPlaybackClock.h"

#include "FeelRecipe.h"

bool FFeelPlaybackClock::Advance(const UFeelRecipe& Recipe, float DeltaSeconds)
{
	RecipeTime += FMath::Max(DeltaSeconds, 0.0f);
	return !bReleased && WrapIntoSustain(Recipe, RecipeTime);
}

bool FFeelPlaybackClock::HasSustain(const UFeelRecipe& Recipe)
{
	return Recipe.bSustain && Recipe.SustainEnd - Recipe.SustainStart >= UFeelRecipe::MinSustainLength;
}

bool FFeelPlaybackClock::WrapIntoSustain(const UFeelRecipe& Recipe, float& InOutTime)
{
	if (!HasSustain(Recipe) || InOutTime <= Recipe.SustainEnd)
	{
		return false;
	}

	const float Length = Recipe.SustainEnd - Recipe.SustainStart;
	InOutTime = Recipe.SustainStart + FMath::Fmod(InOutTime - Recipe.SustainStart, Length);
	return true;
}
