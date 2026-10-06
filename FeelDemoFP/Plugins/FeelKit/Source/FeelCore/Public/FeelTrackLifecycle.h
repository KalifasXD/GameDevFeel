// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "UObject/WeakObjectPtr.h"

class UFeelRecipe;
struct FFeelEvalParams;

/**
 * Start and stop bookkeeping for one playing instance's tracks (OnStart and OnStop).
 * Shared by the runtime subsystem and the editor preview, so side-effect steps such as sounds behave the same in both.
 */
class FEELCORE_API FFeelTrackLifecycle
{
public:
	/** Builds the context passed to a step, from the track index and the track's intensity. */
	using FMakeContext = TFunctionRef<FFeelContext(int32 TrackIndex, float Intensity)>;

	/**
	 * Forgets all tracks. Instant tracks at or before LastTime will not fire, and tracks already over by then are skipped.
	 * Use a negative LastTime to start from the beginning.
	 */
	void Reset(int32 NumTracks, float InLastTime = -1.0f);

	/**
	 * Starts tracks reached by Time and stops tracks that have ended. Tracks crossed entirely since the last update
	 * start and stop in the same update. Mute, solo, conditions and comfort (including substitutes) are respected at start.
	 */
	void Update(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params, FMakeContext MakeContext);

	/**
	 * Sustain loop: time jumped back to ToTime. Tracks starting at or after ToTime stop normally if running and become
	 * ready to start again; tracks that started earlier keep their state.
	 */
	void Rewind(const UFeelRecipe& Recipe, float ToTime, FMakeContext MakeContext);

	/**
	 * Sustain loop wrap, shared by the runtime and the editor preview: tracks inside the region update up to just before
	 * Sustain End, then the region rewinds to Sustain Start. Tracks that start at Sustain End belong to the ending, so
	 * they do not start (and stop again) on every loop.
	 */
	void WrapSustain(const UFeelRecipe& Recipe, const FFeelEvalParams& Params, FMakeContext MakeContext);

	/**
	 * Release with Jump to End on Release: time jumped forward to ToTime. Tracks that lie entirely between the last update
	 * and ToTime are skipped instead of starting and stopping at once; tracks still active at ToTime start on the next
	 * Update, and running tracks that end before ToTime stop normally then. Recipes played by tracks jump with them.
	 */
	void JumpForward(const UFeelRecipe& Recipe, float ToTime);

	/** Stops every running track. */
	void StopAll(const UFeelRecipe* Recipe, bool bInterrupted, FMakeContext MakeContext);

	/** Number of tracks currently running. */
	int32 GetNumRunning() const;

	/** Step object a running track started (its step, substitute or chosen option), or null. */
	UFeelStep* GetRunningStep(int32 TrackIndex) const;

	/** Tracks of this recipe (not inner recipes) that started during the last Update. */
	TConstArrayView<int32> GetStartedThisUpdate() const { return StartedThisUpdate; }

private:
	enum class ETrackState : uint8
	{
		NotStarted,
		Running,
		Finished,
	};

	void StopTrack(int32 TrackIndex, bool bInterrupted, FMakeContext MakeContext);

	/** Runs the tracks of a Play Recipe track's inner recipe. */
	void UpdateChild(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params, FMakeContext MakeContext);

	TArray<ETrackState> States;
	TArray<float> StartIntensities;
	TArray<TWeakObjectPtr<UFeelStep>> RunningSteps;

	TArray<int32, TInlineAllocator<4>> StartedThisUpdate;

	/** Lifecycles of inner recipes, by the index of the Play Recipe track running them. Shared so instances stay copyable. */
	TMap<int32, TSharedPtr<FFeelTrackLifecycle>> Children;

	float LastTime = -1.0f;
};
