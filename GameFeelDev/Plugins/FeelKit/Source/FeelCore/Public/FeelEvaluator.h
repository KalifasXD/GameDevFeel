// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortTypes.h"
#include "FeelParameters.h"

class IFeelOutputSink;
class UFeelRecipe;
class UFeelStep;
struct FFeelTrack;

/** Per-evaluation inputs that are not part of the recipe. */
struct FEELCORE_API FFeelEvalParams
{
	/** Intensity passed by the caller. Multiplied with the recipe default intensity and each track curve. */
	float Intensity = 1.0f;

	/** Seed of the playing instance, combined with each track seed. */
	int32 InstanceSeed = 0;

	/** Editor preview only: when any track is soloed, evaluate only soloed tracks. */
	bool bRespectSolo = false;

	/**
	 * Distance in centimeters from the target to the nearest local player camera, measured when the instance started.
	 * Used by the Max Distance condition. Negative means unknown (no local camera); the condition then passes.
	 */
	float TargetDistance = -1.0f;

	/** Whether the target belongs to a local player, measured when the instance started. Used by the Local Player Only condition. */
	bool bTargetIsLocalPlayer = true;

	/** Parameter values of this play, by name. Null or missing names use the recipe's defaults. */
	const TMap<FName, float>* ParameterValues = nullptr;

	/** When set, only tracks that apply to this actor of the play are evaluated. Unset evaluates every track. */
	TOptional<EFeelTrackTarget> TargetFilter;

	/** Whether the play has an instigator. Tracks that apply to the instigator are skipped without one. */
	bool bHasInstigator = true;

	/** The play context's Direction, normalized, in world space. Zero when none. */
	FVector WorldDirection = FVector::ZeroVector;

	/** The play context's Direction in the view space of the target's local player at play start (X forward, Y right, Z up). Zero when none. */
	FVector ViewDirection = FVector::ZeroVector;

	/** Direction from the play context's Location to the target, in the same view space. Zero when no location was passed. */
	FVector ViewDirectionFromLocation = FVector::ZeroVector;

	/** Whether the play has been released (sustained recipes only). Decides whether a release recipe plays. */
	bool bReleased = false;

	/** Whether the release came from the recipe's Release Parameter reaching Release At: On Full Release plays, not On Early Release. */
	bool bReleaseReached = false;

	/** Nesting depth when evaluating a recipe inside another recipe's track. */
	int32 NestingDepth = 0;

	/** Per-track strength multipliers decided at run time, such as the flash limiter's. Missing entries are 1. */
	TConstArrayView<float> TrackScales;

	/** The same multipliers for the tracks of the release recipe. */
	TConstArrayView<float> ReleaseTrackScales;

	/** Comfort settings to apply. Without scales, evaluation is neutral. */
	FFeelComfortContext Comfort;
};

/** One channel's combined intensity across a recipe, for the intensity graph. */
struct FEELCORE_API FFeelChannelIntensity
{
	FGameplayTag Channel;

	/** Strongest track intensity of this channel at evenly spaced times from 0 to the recipe length, comfort included. */
	TArray<float> Samples;
};

/**
 * Shared, stateless recipe evaluation. Given a recipe, a time and comfort settings, sends every active track's
 * contribution to a sink. Used by both the runtime and the editor preview.
 */
class FEELCORE_API FFeelEvaluator
{
public:
	/** Evaluates all active tracks at Time (seconds from recipe start). Returns the number of tracks evaluated. */
	static int32 Evaluate(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params, IFeelOutputSink& Sink);

	/** Nominal check without random duration: true when Time lies within [StartTime, EndTime] of a track with a positive duration. */
	static bool IsTrackActiveAt(const FFeelTrack& Track, float Time);

	/** Nominal normalized track time at Time, clamped to [0, 1], without random duration. */
	static float GetTrackAlpha(const FFeelTrack& Track, float Time);

	/** Track length for this play: the track duration times its random duration scale. */
	static float GetTrackDuration(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params);

	/** Track end time for this play. */
	static float GetTrackEndTime(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params);

	/** Recipe length for this play: the latest track end, random durations included. */
	static float GetRecipeDuration(const UFeelRecipe& Recipe, const FFeelEvalParams& Params);

	/** True when Time lies within this play's [StartTime, EndTime] of a track with a positive length. */
	static bool IsTrackActiveAt(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params);

	/** This play's normalized track time at Time, clamped to [0, 1]. */
	static float GetTrackAlpha(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params);

	/**
	 * Call intensity x recipe default intensity x track curve x parameter mappings x random intensity, at Time,
	 * before comfort.
	 */
	static float ComputeTrackIntensity(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params);

	/** Value of a declared parameter for this play: the play's value or the default, clamped to the parameter's range. */
	static float GetParameterValue(const FFeelRecipeParameter& Parameter, const FFeelEvalParams& Params);

	/**
	 * Whether the recipe's Release Parameter has reached Release At for this play. False without a sustain region, without
	 * a Release Parameter, or when the parameter is not declared. The runtime and the editor preview both release with it.
	 */
	static bool IsReleaseParameterReached(const UFeelRecipe& Recipe, const FFeelEvalParams& Params);

	/** Product of a track's parameter mapping multipliers. Mappings to undeclared parameters are ignored. */
	static float ComputeParameterScale(const UFeelRecipe& Recipe, const FFeelTrack& Track, const FFeelEvalParams& Params);

	/** This play's random intensity multiplier of a track, between its RandomIntensity Min and Max. */
	static float GetRandomIntensityScale(const FFeelTrack& Track, int32 TrackIndex, const FFeelEvalParams& Params);

	/**
	 * False for muted tracks, tracks without a step, non-soloed tracks while soloing, tracks filtered out by
	 * TargetFilter, and tracks whose conditions fail.
	 */
	static bool ShouldEvaluateTrack(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params);

	/**
	 * Track conditions (FFeelConditions): platform filter, local player only, max distance and chance.
	 * Chance is rolled from the instance seed, the track index and the track seed, so one play gives the same answer on
	 * every frame and when scrubbing, while each new play rolls again.
	 */
	static bool PassesConditions(const FFeelTrack& Track, int32 TrackIndex, const FFeelEvalParams& Params);

	/** The chance roll of PassesConditions, in [0, 1). The track plays when the roll is below its Chance. */
	static float GetChanceRoll(const FFeelTrack& Track, int32 TrackIndex, int32 InstanceSeed);

	/** A deterministic value in [0, 1) for one play, one track and one purpose (Salt). */
	static float GetTrackRoll(const FFeelTrack& Track, int32 TrackIndex, int32 InstanceSeed, uint32 Salt);

	/** Whether any track of the recipe applies to the play's instigator. */
	static bool HasInstigatorTracks(const UFeelRecipe& Recipe);

	/**
	 * Step to play for a track under comfort settings: the track's step, its substitute, or null
	 * when the track is skipped. OutComfortScale receives the comfort multiplier to apply to that step.
	 */
	static UFeelStep* ResolveStep(const FFeelTrack& Track, const FFeelComfortContext& Comfort, float& OutComfortScale);

	/** The step object a track plays in this play: ResolveStep, then any step-level choice (such as a random choice). */
	static UFeelStep* ResolveTrackStep(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params, float& OutComfortScale);

	/**
	 * Evaluation settings for the recipe a Play Recipe track plays at Time: this track's intensity, a seed of its own,
	 * one level deeper. Returns false when the step has no recipe or the nesting limit is reached.
	 */
	static bool MakeNestedParams(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params, const class UFeelStep_Recipe& Step, FFeelEvalParams& OutNestedParams);

	/**
	 * Release recipe this play plays from Sustain End: On Full Release when the Release Parameter released it, On Early
	 * Release when the game did. Null before the release, without a sustain region, when the chosen setting is empty or
	 * names the recipe itself, and inside nested recipes.
	 */
	static const UFeelRecipe* GetReleaseRecipe(const UFeelRecipe& Recipe, const FFeelEvalParams& Params);

	/**
	 * Evaluation settings for a release recipe: this play's intensity times the recipe default intensity, a seed of its
	 * own, one level deeper, not released itself, every track on the play's target. Returns false at the nesting limit.
	 */
	static bool MakeReleaseParams(const UFeelRecipe& Recipe, const FFeelEvalParams& Params, FFeelEvalParams& OutReleaseParams);

	/**
	 * Samples each channel's combined intensity (strongest track per channel) at NumSamples evenly spaced times,
	 * respecting mute, solo, conditions, parameters, random ranges and comfort. Channels are sorted by tag name.
	 */
	static void SampleChannelIntensities(const UFeelRecipe& Recipe, int32 NumSamples, const FFeelEvalParams& Params, TArray<FFeelChannelIntensity>& OutChannels);
};
