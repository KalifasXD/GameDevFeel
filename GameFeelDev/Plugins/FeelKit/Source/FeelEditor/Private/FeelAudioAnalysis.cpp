// Copyright 2026 Billo. All Rights Reserved.

#include "FeelAudioAnalysis.h"

#include "Sound/SoundCue.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Sound/SoundWave.h"
#include "UObject/ObjectKey.h"

float FFeelAudioEnvelope::SampleLoudness(float Time, float Window) const
{
	if (Peaks.Num() == 0 || Time < 0.0f || Time > Duration)
	{
		return 0.0f;
	}

	const int32 FirstBin = FMath::Clamp(FMath::FloorToInt32((Time - Window * 0.5f) / BinSeconds), 0, Peaks.Num() - 1);
	const int32 LastBin = FMath::Clamp(FMath::FloorToInt32((Time + Window * 0.5f) / BinSeconds), FirstBin, Peaks.Num() - 1);
	float Loudest = 0.0f;
	for (int32 Bin = FirstBin; Bin <= LastBin; ++Bin)
	{
		Loudest = FMath::Max(Loudest, Peaks[Bin]);
	}
	return Loudest;
}

TOptional<float> FFeelAudioEnvelope::FindNearestOnset(float Time, float MaxDistance) const
{
	TOptional<float> Nearest;
	float NearestDistance = MaxDistance;
	for (float Onset : Onsets)
	{
		const float Distance = FMath::Abs(Onset - Time);
		if (Distance <= NearestDistance)
		{
			NearestDistance = Distance;
			Nearest = Onset;
		}
	}
	return Nearest;
}

FFeelAudioEnvelope FFeelAudioAnalysis::BuildEnvelope(TConstArrayView<int16> Samples, uint32 SampleRate, int32 NumChannels)
{
	FFeelAudioEnvelope Envelope;
	if (SampleRate == 0 || NumChannels <= 0 || Samples.Num() < NumChannels)
	{
		return Envelope;
	}

	const int32 NumFrames = Samples.Num() / NumChannels;
	const int32 FramesPerBin = FMath::Max(1, FMath::RoundToInt32(SampleRate * FFeelAudioEnvelope::BinSeconds));
	Envelope.Duration = static_cast<float>(NumFrames) / static_cast<float>(SampleRate);
	Envelope.Peaks.Reserve(NumFrames / FramesPerBin + 1);

	float Loudest = 0.0f;
	for (int32 FrameStart = 0; FrameStart < NumFrames; FrameStart += FramesPerBin)
	{
		const int32 FrameEnd = FMath::Min(FrameStart + FramesPerBin, NumFrames);
		int32 Peak = 0;
		for (int32 SampleIndex = FrameStart * NumChannels; SampleIndex < FrameEnd * NumChannels; ++SampleIndex)
		{
			Peak = FMath::Max(Peak, FMath::Abs(static_cast<int32>(Samples[SampleIndex])));
		}
		const float Level = static_cast<float>(Peak) / 32768.0f;
		Envelope.Peaks.Add(Level);
		Loudest = FMath::Max(Loudest, Level);
	}

	if (Loudest <= KINDA_SMALL_NUMBER)
	{
		return Envelope;
	}
	for (float& Peak : Envelope.Peaks)
	{
		Peak /= Loudest;
	}

	// Onsets: a bin much louder than the average of the preceding 50 ms, and loud enough to matter. At most one onset
	// per 60 ms so one hit gives one onset.
	constexpr int32 HistoryBins = 10;
	constexpr float RiseFactor = 2.0f;
	constexpr float MinLevel = 0.15f;
	constexpr float MinGap = 0.06f;
	float LastOnset = -1.0f;
	for (int32 Bin = 0; Bin < Envelope.Peaks.Num(); ++Bin)
	{
		float History = 0.0f;
		const int32 HistoryStart = FMath::Max(0, Bin - HistoryBins);
		for (int32 Previous = HistoryStart; Previous < Bin; ++Previous)
		{
			History += Envelope.Peaks[Previous];
		}
		History = Bin > HistoryStart ? History / static_cast<float>(Bin - HistoryStart) : 0.0f;

		const float Level = Envelope.Peaks[Bin];
		const float OnsetTime = Bin * FFeelAudioEnvelope::BinSeconds;
		if (Level >= MinLevel && Level > History * RiseFactor + 0.02f && (LastOnset < 0.0f || OnsetTime - LastOnset >= MinGap))
		{
			Envelope.Onsets.Add(OnsetTime);
			LastOnset = OnsetTime;
		}
	}
	return Envelope;
}

const USoundWave* FFeelAudioAnalysis::ResolveSoundWave(const USoundBase* Sound)
{
	if (const USoundWave* Wave = Cast<USoundWave>(Sound))
	{
		return Wave;
	}

	// A Sound Cue: the longest wave it can play. Cues that pick between waves show the longest one.
	const USoundCue* Cue = Cast<USoundCue>(Sound);
	if (!Cue || !Cue->FirstNode)
	{
		return nullptr;
	}
	TArray<const USoundNodeWavePlayer*> WavePlayers;
	Cue->RecursiveFindNode<USoundNodeWavePlayer>(Cue->FirstNode, WavePlayers);
	const USoundWave* Longest = nullptr;
	for (const USoundNodeWavePlayer* WavePlayer : WavePlayers)
	{
		const USoundWave* Wave = WavePlayer ? WavePlayer->GetSoundWave() : nullptr;
		if (Wave && (!Longest || Wave->Duration > Longest->Duration))
		{
			Longest = Wave;
		}
	}
	return Longest;
}

const FFeelAudioEnvelope* FFeelAudioAnalysis::GetEnvelope(const USoundBase* Sound)
{
	const USoundWave* Wave = ResolveSoundWave(Sound);
	if (!Wave)
	{
		return nullptr;
	}

	struct FCachedEnvelope
	{
		FGuid SourceId;
		TOptional<FFeelAudioEnvelope> Envelope;
	};
	static TMap<FObjectKey, FCachedEnvelope> Cache;

	// A reimport changes the compressed data guid.
	const FGuid SourceId = Wave->CompressedDataGuid;
	FCachedEnvelope& Cached = Cache.FindOrAdd(FObjectKey(Wave));
	if (Cached.SourceId == SourceId && Cached.SourceId.IsValid())
	{
		return Cached.Envelope.GetPtrOrNull();
	}

	Cached.SourceId = SourceId;
	Cached.Envelope.Reset();

	TArray<uint8> RawPcm;
	uint32 SampleRate = 0;
	uint16 NumChannels = 0;
	if (Wave->GetImportedSoundWaveData(RawPcm, SampleRate, NumChannels) && NumChannels > 0 && RawPcm.Num() >= 2)
	{
		const TConstArrayView<int16> Samples(reinterpret_cast<const int16*>(RawPcm.GetData()), RawPcm.Num() / 2);
		Cached.Envelope = BuildEnvelope(Samples, SampleRate, NumChannels);
	}
	return Cached.Envelope.GetPtrOrNull();
}
