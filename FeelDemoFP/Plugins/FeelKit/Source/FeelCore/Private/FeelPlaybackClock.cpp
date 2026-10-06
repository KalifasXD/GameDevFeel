// Copyright 2026 Billo. All Rights Reserved.

#include "FeelPlaybackClock.h"

#include "FeelRecipe.h"

bool FFeelPlaybackClock::Advance(const UFeelRecipe& Recipe, float DeltaSeconds)
{
	RecipeTime += FMath::Max(DeltaSeconds, 0.0f);
	return !bReleased && WrapIntoSustain(Recipe, RecipeTime);
}

void FFeelPlaybackClock::Release()
{
	if (!bReleased)
	{
		bReleased = true;
		bJumpPending = true;
	}
}

bool FFeelPlaybackClock::ApplyPendingJump(const UFeelRecipe& Recipe)
{
	if (!bJumpPending)
	{
		return false;
	}
	bJumpPending = false;
	return GetReleaseJumpTime(Recipe, RecipeTime, RecipeTime);
}

bool FFeelPlaybackClock::GetReleaseJumpTime(const UFeelRecipe& Recipe, float Time, float& OutTime)
{
	// A release recipe answers the release, so it always starts at once.
	const bool bJump = Recipe.bJumpToEndOnRelease || Recipe.FullReleaseRecipe || Recipe.EarlyReleaseRecipe;
	if (!bJump || !HasSustain(Recipe) || Time >= Recipe.SustainEnd)
	{
		return false;
	}
	OutTime = Recipe.SustainEnd;
	return true;
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
