// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "Sound/SoundWave.h"

/**
 * Diagnostic (filter DiagFeel): measures every sound wave in a folder (-FeelSoundFolder=/Game/..., default
 * /Game/_FeelAudition) so sounds can be compared without listening: length, peak and loudness, how fast it attacks, how
 * long it rings, brightness (zero crossings per second) and low-end weight (share of energy below about 200 Hz).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSoundStatsDiagnostic, "DiagFeel.SoundStats", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelSoundStatsDiagnostic::RunTest(const FString& Parameters)
{
	FString Folder = TEXT("/Game/_FeelAudition");
	FParse::Value(FCommandLine::Get(), TEXT("FeelSoundFolder="), Folder);

	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	Registry.ScanPathsSynchronous({ Folder }, true);
	TArray<FAssetData> Assets;
	Registry.GetAssetsByPath(FName(*Folder), Assets, true);
	Assets.Sort([](const FAssetData& A, const FAssetData& B) { return A.AssetName.LexicalLess(B.AssetName); });

	for (const FAssetData& Asset : Assets)
	{
		const USoundWave* Wave = Cast<USoundWave>(Asset.GetAsset());
		TArray<uint8> RawPcm;
		uint32 SampleRate = 0;
		uint16 NumChannels = 0;
		if (!Wave || !const_cast<USoundWave*>(Wave)->GetImportedSoundWaveData(RawPcm, SampleRate, NumChannels) || NumChannels == 0 || SampleRate == 0)
		{
			continue;
		}

		const int16* Samples = reinterpret_cast<const int16*>(RawPcm.GetData());
		const int32 NumFrames = RawPcm.Num() / 2 / NumChannels;
		TArray<float> Mono;
		Mono.SetNumUninitialized(NumFrames);
		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			float Sum = 0.0f;
			for (int32 Channel = 0; Channel < NumChannels; ++Channel)
			{
				Sum += Samples[Frame * NumChannels + Channel] / 32768.0f;
			}
			Mono[Frame] = Sum / NumChannels;
		}

		float Peak = 0.0f;
		int32 PeakFrame = 0;
		double Energy = 0.0;
		double LowEnergy = 0.0;
		int32 Crossings = 0;
		float Low = 0.0f;
		const float LowAlpha = 1.0f - FMath::Exp(-2.0f * PI * 200.0f / static_cast<float>(SampleRate));
		for (int32 Frame = 0; Frame < NumFrames; ++Frame)
		{
			const float Value = Mono[Frame];
			if (FMath::Abs(Value) > Peak)
			{
				Peak = FMath::Abs(Value);
				PeakFrame = Frame;
			}
			Energy += Value * Value;
			Low += LowAlpha * (Value - Low);
			LowEnergy += Low * Low;
			if (Frame > 0 && ((Mono[Frame - 1] < 0.0f) != (Value < 0.0f)))
			{
				++Crossings;
			}
		}

		// Attack: time to reach half the peak. Ring: time after the peak until a 20 ms window stays 30 dB under the peak.
		int32 AttackFrame = 0;
		while (AttackFrame < NumFrames && FMath::Abs(Mono[AttackFrame]) < Peak * 0.5f)
		{
			++AttackFrame;
		}
		const int32 Window = FMath::Max<int32>(SampleRate / 50, 1);
		int32 RingEnd = PeakFrame;
		for (int32 Start = PeakFrame; Start < NumFrames; Start += Window)
		{
			double WindowEnergy = 0.0;
			const int32 End = FMath::Min(Start + Window, NumFrames);
			for (int32 Frame = Start; Frame < End; ++Frame)
			{
				WindowEnergy += Mono[Frame] * Mono[Frame];
			}
			const float WindowRms = FMath::Sqrt(static_cast<float>(WindowEnergy / FMath::Max(End - Start, 1)));
			if (WindowRms > Peak * 0.0316f)
			{
				RingEnd = End;
			}
		}

		const float Seconds = static_cast<float>(NumFrames) / SampleRate;
		const float Rms = FMath::Sqrt(static_cast<float>(Energy / FMath::Max(NumFrames, 1)));
		AddInfo(FString::Printf(TEXT("SOUNDSTATS %s len=%.3f peak=%.1fdB rms=%.1fdB attack=%.1fms ring=%.3fs bright=%.0f/s low=%.2f ch=%d rate=%d"),
			*Asset.AssetName.ToString(), Seconds,
			20.0f * FMath::LogX(10.0f, FMath::Max(Peak, 1e-6f)), 20.0f * FMath::LogX(10.0f, FMath::Max(Rms, 1e-6f)),
			1000.0f * AttackFrame / SampleRate, static_cast<float>(RingEnd - PeakFrame) / SampleRate,
			Crossings / FMath::Max(Seconds, 0.001f), Energy > 0.0 ? static_cast<float>(LowEnergy / Energy) : 0.0f,
			static_cast<int32>(NumChannels), static_cast<int32>(SampleRate)));
	}
	return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortSaveGame.h"
#include "Kismet/GameplayStatics.h"

/** Diagnostic (filter DiagFeel): prints the comfort scales saved in the first local player's comfort slot. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSavedComfortDiagnostic, "DiagFeel.SavedComfort", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelSavedComfortDiagnostic::RunTest(const FString& Parameters)
{
	const UFeelComfortSaveGame* Save = Cast<UFeelComfortSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("FeelComfort_0"), 0));
	if (!Save)
	{
		AddInfo(TEXT("SAVEDCOMFORT no save in slot FeelComfort_0"));
		return true;
	}
	const FFeelComfortScales& S = Save->Scales;
	AddInfo(FString::Printf(TEXT("SAVEDCOMFORT master %.2f shake %.2f motion %.2f flashes %.2f hitstop %.2f distortion %.2f haptics %.2f"),
		S.Master, S.CameraShake, S.CameraMotion, S.Flashes, S.HitstopAndSlowMo, S.ScreenDistortion, S.Haptics));
	return true;
}

#endif
