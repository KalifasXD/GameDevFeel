// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USoundBase;
class USoundWave;

/** Loudness of one sound over time, read from its imported audio. */
struct FFeelAudioEnvelope
{
	/** Seconds per bin. */
	static constexpr float BinSeconds = 0.005f;

	/** Peak level of each bin, normalized so the loudest bin is 1. */
	TArray<float> Peaks;

	/** Times in seconds where the sound suddenly gets louder (hits, footsteps, beats). */
	TArray<float> Onsets;

	float Duration = 0.0f;

	/** Highest peak within Window seconds centered on Time. 0 outside the sound. */
	float SampleLoudness(float Time, float Window) const;

	/** The onset nearest to Time within MaxDistance seconds, or unset. */
	TOptional<float> FindNearestOnset(float Time, float MaxDistance) const;
};

/** Reads sound wave audio for the timeline: waveforms, snapping to hits and tracks built from sounds. Editor only. */
class FFeelAudioAnalysis
{
public:
	/**
	 * Envelope of a sound, computed once and cached until the sound changes. Sound Waves are read directly; Sound Cues use
	 * the longest wave they can play. Null for sounds without imported audio (procedural sounds, MetaSounds).
	 */
	static const FFeelAudioEnvelope* GetEnvelope(const USoundBase* Sound);

	/** The wave a sound plays: the sound itself, or the longest wave in a Sound Cue. */
	static const USoundWave* ResolveSoundWave(const USoundBase* Sound);

	/** Builds an envelope from interleaved 16-bit PCM. Exposed for tests. */
	static FFeelAudioEnvelope BuildEnvelope(TConstArrayView<int16> Samples, uint32 SampleRate, int32 NumChannels);
};
