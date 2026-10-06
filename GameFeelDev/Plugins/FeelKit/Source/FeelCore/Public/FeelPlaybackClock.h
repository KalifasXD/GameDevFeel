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

	/** Set by Release until ApplyPendingJump has run, so a release between frames still jumps on the next frame. */
	bool bJumpPending = false;

	/** Advances by real seconds. Returns true when time wrapped from the end of the sustain region back to its start. */
	bool Advance(const UFeelRecipe& Recipe, float DeltaSeconds);

	/** Stops looping; the play continues to the end of the recipe. A second release does nothing. */
	void Release();

	/**
	 * After a release, moves time to Sustain End when the recipe has Jump to End on Release or a release recipe. Call once
	 * per frame before Advance. Returns true when time jumped; the caller then moves its track lifecycle forward to the new
	 * time.
	 */
	bool ApplyPendingJump(const UFeelRecipe& Recipe);

	/** Whether the recipe has a usable sustain region. */
	static bool HasSustain(const UFeelRecipe& Recipe);

	/** Time wrapped into the sustain region, when it passed the region end. Returns true if it wrapped. */
	static bool WrapIntoSustain(const UFeelRecipe& Recipe, float& InOutTime);

	/**
	 * Where a release at Time jumps to: Sustain End, when the recipe has a sustain region, Jump to End on Release is on or a
	 * release recipe is set, and Time is before Sustain End. Returns false when the play keeps its time.
	 */
	static bool GetReleaseJumpTime(const UFeelRecipe& Recipe, float Time, float& OutTime);
};
