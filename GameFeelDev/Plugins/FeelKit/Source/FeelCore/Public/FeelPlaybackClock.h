// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UFeelRecipe;

/**
 * Recipe time of one play, shared by the runtime and the editor preview.
 * Without a sustain region, recipe time simply advances. With one, time loops inside the region until the play is
 * released, then continues through the rest of the recipe.
 */
struct FEELCORE_API FFeelPlaybackClock
{
	/** Seconds into the recipe. */
	float RecipeTime = 0.0f;

	/** Once released, the sustain region no longer loops. */
	bool bReleased = false;

	/** Advances by real seconds. Returns true when time wrapped from the end of the sustain region back to its start. */
	bool Advance(const UFeelRecipe& Recipe, float DeltaSeconds);

	/** Stops looping; the play continues to the end of the recipe. */
	void Release() { bReleased = true; }

	/** Whether the recipe has a usable sustain region. */
	static bool HasSustain(const UFeelRecipe& Recipe);

	/** Time wrapped into the sustain region, when it passed the region end. Returns true if it wrapped. */
	static bool WrapIntoSustain(const UFeelRecipe& Recipe, float& InOutTime);
};
