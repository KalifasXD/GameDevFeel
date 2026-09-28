// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelAudioAnalysis.h"
#include "FeelComfortAudit.h"
#include "FeelEvaluator.h"
#include "FeelGifWriter.h"
#include "FeelPlayCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSettings.h"
#include "FeelTags.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelProofEditorTests
{
	UFeelStep_ScreenFlash* AddFlash(UFeelRecipe* Recipe, float StartTime, float Duration, const FLinearColor& Color = FLinearColor::White, float Opacity = 0.5f)
	{
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->Color = Color;
		Flash->MaxOpacity = Opacity;
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Flash;
	}

	/** Reference GIF LZW decoder, independent of the encoder. */
	TArray<uint8> DecodeLzw(const TArray<uint8>& Data, int32 MinCodeSize)
	{
		TArray<uint8> Out;
		const int32 ClearCode = 1 << MinCodeSize;
		const int32 EndCode = ClearCode + 1;
		int32 CodeSize = MinCodeSize + 1;
		TArray<TArray<uint8>> Table;
		auto ResetTable = [&]()
		{
			Table.Reset();
			for (int32 Index = 0; Index < ClearCode; ++Index)
			{
				Table.Add({ static_cast<uint8>(Index) });
			}
			Table.AddDefaulted(2);
			CodeSize = MinCodeSize + 1;
		};
		ResetTable();

		int32 BitPosition = 0;
		int32 Previous = -1;
		while (BitPosition + CodeSize <= Data.Num() * 8)
		{
			int32 Code = 0;
			for (int32 Bit = 0; Bit < CodeSize; ++Bit)
			{
				const int32 Position = BitPosition + Bit;
				Code |= ((Data[Position / 8] >> (Position % 8)) & 1) << Bit;
			}
			BitPosition += CodeSize;

			if (Code == ClearCode)
			{
				ResetTable();
				Previous = -1;
				continue;
			}
			if (Code == EndCode)
			{
				break;
			}

			TArray<uint8> Entry;
			if (Code < Table.Num())
			{
				Entry = Table[Code];
				if (Previous >= 0)
				{
					TArray<uint8> NewEntry = Table[Previous];
					NewEntry.Add(Entry[0]);
					Table.Add(MoveTemp(NewEntry));
				}
			}
			else if (Previous >= 0)
			{
				Entry = Table[Previous];
				Entry.Add(Table[Previous][0]);
				Table.Add(Entry);
			}
			else
			{
				return TArray<uint8>();
			}
			Out.Append(Entry);
			Previous = Code;
			if (Table.Num() == (1 << CodeSize) && CodeSize < 12)
			{
				++CodeSize;
			}
		}
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelGifWriterTest, "FeelKit.Editor.GifWriter", FEEL_TEST_FLAGS)
bool FFeelGifWriterTest::RunTest(const FString& Parameters)
{
	using namespace FeelProofEditorTests;

	// Uniform, repeating and noisy data, the last long enough to fill the code table and force clears.
	TArray<TArray<uint8>> Inputs;
	Inputs.Add({ 7 });
	Inputs.AddDefaulted_GetRef().Init(42, 5000);
	{
		TArray<uint8>& Pattern = Inputs.AddDefaulted_GetRef();
		for (int32 Index = 0; Index < 20000; ++Index)
		{
			Pattern.Add(static_cast<uint8>(Index % 13));
		}
	}
	{
		FRandomStream Random(1234);
		TArray<uint8>& Noise = Inputs.AddDefaulted_GetRef();
		for (int32 Index = 0; Index < 200000; ++Index)
		{
			Noise.Add(static_cast<uint8>(Random.RandRange(0, 251)));
		}
	}

	for (const TArray<uint8>& Input : Inputs)
	{
		const TArray<uint8> Compressed = FFeelGifWriter::CompressLzw(Input);
		const TArray<uint8> Decoded = DecodeLzw(Compressed, 8);
		TestTrue(FString::Printf(TEXT("LZW round trip of %d indices"), Input.Num()), Decoded == Input);
	}

	// Adaptive palette: a frame with a few distinct colors keeps them exactly (within dithering of one 5-bit step).
	{
		TArray<TArray<FColor>> PaletteFrames;
		TArray<FColor>& Frame = PaletteFrames.AddDefaulted_GetRef();
		const FColor Distinct[] = { FColor(200, 30, 40), FColor(20, 180, 60), FColor(30, 60, 220), FColor(250, 250, 250), FColor(10, 10, 10) };
		for (int32 Pixel = 0; Pixel < 1000; ++Pixel)
		{
			Frame.Add(Distinct[Pixel % UE_ARRAY_COUNT(Distinct)]);
		}
		const FFeelGifPalette Palette = FFeelGifWriter::BuildPalette(PaletteFrames);
		TestTrue(TEXT("Palette has at least the distinct colors"), Palette.Colors.Num() >= UE_ARRAY_COUNT(Distinct) && Palette.Colors.Num() <= 256);
		int32 WorstError = 0;
		for (int32 Pixel = 0; Pixel < Frame.Num(); ++Pixel)
		{
			const FColor& Mapped = Palette.Colors[Palette.Map(Frame[Pixel], Pixel % 7, Pixel / 7)];
			WorstError = FMath::Max3(WorstError, FMath::Abs(Mapped.R - Frame[Pixel].R), FMath::Max(FMath::Abs(Mapped.G - Frame[Pixel].G), FMath::Abs(Mapped.B - Frame[Pixel].B)));
		}
		TestTrue(FString::Printf(TEXT("Distinct colors map closely (worst channel error %d)"), WorstError), WorstError <= 12);

		// A smooth gradient uses many palette entries instead of a few bands.
		TArray<TArray<FColor>> GradientFrames;
		TArray<FColor>& Gradient = GradientFrames.AddDefaulted_GetRef();
		for (int32 Pixel = 0; Pixel < 256 * 16; ++Pixel)
		{
			const uint8 Level = static_cast<uint8>(Pixel / 16);
			Gradient.Add(FColor(Level, static_cast<uint8>(255 - Level), 128));
		}
		const FFeelGifPalette GradientPalette = FFeelGifWriter::BuildPalette(GradientFrames);
		TestTrue(FString::Printf(TEXT("Gradient palette uses many colors (%d)"), GradientPalette.Colors.Num()), GradientPalette.Colors.Num() >= 30);
	}

	TArray<TArray<FColor>> Frames;
	Frames.AddDefaulted_GetRef().Init(FColor::Red, 8 * 4);
	Frames.AddDefaulted_GetRef().Init(FColor::Blue, 8 * 4);
	const TArray<uint8> Gif = FFeelGifWriter::Encode(8, 4, Frames, 5);
	TestTrue(TEXT("GIF starts with the GIF89a signature"), Gif.Num() > 6 && FMemory::Memcmp(Gif.GetData(), "GIF89a", 6) == 0);
	TestEqual(TEXT("GIF ends with the trailer"), Gif.Num() > 0 ? Gif.Last() : 0, static_cast<uint8>(0x3B));
	TestEqual(TEXT("Width stored little endian"), Gif.Num() > 7 ? static_cast<int32>(Gif[6] | (Gif[7] << 8)) : 0, 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelAudioEnvelopeTest, "FeelKit.Editor.AudioEnvelope", FEEL_TEST_FLAGS)
bool FFeelAudioEnvelopeTest::RunTest(const FString& Parameters)
{
	// One second of stereo audio at 8 kHz: quiet, a loud hit at 0.2 s decaying over 0.1 s, a second hit at 0.6 s.
	constexpr int32 SampleRate = 8000;
	TArray<int16> Samples;
	for (int32 Frame = 0; Frame < SampleRate; ++Frame)
	{
		const float Time = static_cast<float>(Frame) / SampleRate;
		float Level = 0.01f;
		if (Time >= 0.2f && Time < 0.3f)
		{
			Level = 1.0f - (Time - 0.2f) * 8.0f;
		}
		else if (Time >= 0.6f && Time < 0.7f)
		{
			Level = 0.5f;
		}
		const int16 Value = static_cast<int16>(FMath::Sin(Frame * 0.7f) * Level * 30000.0f);
		Samples.Add(Value);
		Samples.Add(Value);
	}

	const FFeelAudioEnvelope Envelope = FFeelAudioAnalysis::BuildEnvelope(Samples, SampleRate, 2);
	TestEqual(TEXT("Duration"), Envelope.Duration, 1.0f, 0.001f);
	TestEqual(TEXT("One peak per 5 ms"), Envelope.Peaks.Num(), 200);
	TestEqual(TEXT("Loudest bin normalized to 1"), Envelope.SampleLoudness(0.2f, 0.01f), 1.0f, 0.05f);
	TestTrue(TEXT("Quiet part stays quiet"), Envelope.SampleLoudness(0.1f, 0.01f) < 0.05f);
	TestEqual(TEXT("Second hit at half level"), Envelope.SampleLoudness(0.65f, 0.01f), 0.5f, 0.05f);
	TestEqual(TEXT("Outside the sound is silent"), Envelope.SampleLoudness(2.0f, 0.1f), 0.0f);

	if (TestEqual(TEXT("Two onsets"), Envelope.Onsets.Num(), 2))
	{
		TestEqual(TEXT("First onset at the first hit"), Envelope.Onsets[0], 0.2f, 0.01f);
		TestEqual(TEXT("Second onset at the second hit"), Envelope.Onsets[1], 0.6f, 0.01f);
	}
	const TOptional<float> Nearest = Envelope.FindNearestOnset(0.58f, 0.05f);
	TestTrue(TEXT("Nearest onset found within range"), Nearest.IsSet() && FMath::IsNearlyEqual(Nearest.GetValue(), 0.6f, 0.01f));
	TestFalse(TEXT("No onset out of range"), Envelope.FindNearestOnset(0.4f, 0.05f).IsSet());

	TestEqual(TEXT("Silence gives no onsets"), FFeelAudioAnalysis::BuildEnvelope(TArray<int16>({ 0, 0, 0, 0 }), SampleRate, 2).Onsets.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortAuditTest, "FeelKit.Editor.ComfortAudit", FEEL_TEST_FLAGS)
bool FFeelComfortAuditTest::RunTest(const FString& Parameters)
{
	using namespace FeelProofEditorTests;
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();

	TStrongObjectPtr<UFeelRecipe> Clean(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddFlash(Clean.Get(), 0.0f, 0.2f);
	AddFlash(Clean.Get(), 0.5f, 0.2f);
	TArray<FFeelComfortAuditFinding> Findings;
	FFeelComfortAudit::AuditRecipe(*Clean, *Settings, Findings);
	TestEqual(TEXT("Two flashes half a second apart are fine"), Findings.Num(), 0);

	TStrongObjectPtr<UFeelRecipe> Rapid(NewObject<UFeelRecipe>(GetTransientPackage()));
	for (int32 Index = 0; Index < 4; ++Index)
	{
		AddFlash(Rapid.Get(), Index * 0.2f, 0.1f);
	}
	Findings.Reset();
	FFeelComfortAudit::AuditRecipe(*Rapid, *Settings, Findings);
	TestEqual(TEXT("Four flashes in 0.6 s are reported once"), Findings.Num(), 1);

	TestTrue(TEXT("Pure red is saturated red"), FFeelComfortAudit::IsSaturatedRed(FLinearColor(1.0f, 0.1f, 0.1f)));
	TestFalse(TEXT("Orange is not saturated red"), FFeelComfortAudit::IsSaturatedRed(FLinearColor(1.0f, 0.5f, 0.0f)));
	TestFalse(TEXT("Black is not saturated red"), FFeelComfortAudit::IsSaturatedRed(FLinearColor::Black));

	TStrongObjectPtr<UFeelRecipe> Red(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddFlash(Red.Get(), 0.0f, 0.2f, FLinearColor(1.0f, 0.0f, 0.0f), 0.9f);
	AddFlash(Red.Get(), 1.0f, 0.2f, FLinearColor(1.0f, 0.0f, 0.0f), 0.4f);
	Findings.Reset();
	FFeelComfortAudit::AuditRecipe(*Red, *Settings, Findings);
	if (TestEqual(TEXT("Only the strong red flash is reported"), Findings.Num(), 1))
	{
		TestEqual(TEXT("The finding names the track"), Findings[0].TrackIndex, 0);
	}

	// A shake moved onto a flash channel escapes the Camera Shake comfort setting.
	TStrongObjectPtr<UFeelRecipe> Escaped(NewObject<UFeelRecipe>(GetTransientPackage()));
	{
		FFeelTrack& Track = Escaped->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ProceduralShake>(Escaped.Get());
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 0.3f;
	}
	Findings.Reset();
	FFeelComfortAudit::AuditRecipe(*Escaped, *Settings, Findings);
	TestTrue(TEXT("A track outside its step's comfort group is reported"), Findings.ContainsByPredicate([](const FFeelComfortAuditFinding& Finding) { return Finding.TrackIndex == 0 && Finding.bWarning; }));

	// A sustain loop that repeats flashes too fast.
	TStrongObjectPtr<UFeelRecipe> Loop(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddFlash(Loop.Get(), 0.0f, 0.05f);
	AddFlash(Loop.Get(), 0.1f, 0.05f);
	Loop->bSustain = true;
	Loop->SustainStart = 0.0f;
	Loop->SustainEnd = 0.4f;
	Findings.Reset();
	FFeelComfortAudit::AuditRecipe(*Loop, *Settings, Findings);
	TestEqual(TEXT("Two flashes every 0.4 s while held are reported"), Findings.Num(), 1);

	// Project settings.
	TStrongObjectPtr<UFeelSettings> Weak(NewObject<UFeelSettings>(GetTransientPackage()));
	Weak->DefaultComfortScales.bLimitFlashes = true;
	Weak->ReducedFlashingPreset.bLimitFlashes = true;
	Weak->ReducedFlashingPreset.Flashes = 0.5f;
	Weak->ReducedMotionPreset.CameraShake = 0.5f;
	Weak->bApplyComfortToEngineCameraShakes = true;
	Weak->bApplyComfortToEngineForceFeedback = true;
	Findings.Reset();
	FFeelComfortAudit::AuditSettings(*Weak, Findings);
	TestEqual(TEXT("Sensible settings give no findings"), Findings.Num(), 0);

	Weak->DefaultComfortScales.bLimitFlashes = false;
	Weak->ReducedMotionPreset.CameraShake = 1.0f;
	Weak->ReducedMotionPreset.CameraMotion = 1.0f;
	Weak->bApplyComfortToEngineCameraShakes = false;
	Findings.Reset();
	FFeelComfortAudit::AuditSettings(*Weak, Findings);
	TestEqual(TEXT("Limiter off, weak Reduced Motion and engine shakes unscaled are reported"), Findings.Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCaptureReplayTest, "FeelKit.Editor.CaptureReplayAndDecisions", FEEL_TEST_FLAGS)
bool FFeelCaptureReplayTest::RunTest(const FString& Parameters)
{
	using namespace FeelProofEditorTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddFlash(Recipe.Get(), 0.0f, 1.0f, FLinearColor::White, 1.0f);
	AddFlash(Recipe.Get(), 0.0f, 1.0f, FLinearColor::White, 1.0f);
	FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
	Parameter.Name = TEXT("Strength");
	Parameter.DefaultValue = 1.0f;
	Parameter.MaxValue = 1.0f;
	Recipe->Tracks[0].ParameterMappings.AddDefaulted_GetRef().Parameter = Parameter.Name;

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(Recipe.Get());
	State->SetTime(0.5f);
	TestEqual(TEXT("Normal preview: full flash"), State->GetOutput().FlashAlpha, 1.0f, 0.001f);
	TestTrue(TEXT("A normally playing track has no decision"), State->GetTrackDecision(0).IsEmpty());

	// A recorded play: half intensity, the first track suppressed by the limiter, the second softened.
	FFeelPlayCapture Capture;
	Capture.Recipe = Recipe.Get();
	Capture.Seed = 99;
	Capture.Intensity = 0.5f;
	Capture.ParameterValues.Add(Parameter.Name, 1.0f);
	Capture.TrackScales = { 0.0f, 0.5f };
	State->LoadCapture(Capture);
	TestNotNull(TEXT("Replay is active"), State->GetActiveCapture());

	State->SetTime(0.5f);
	TestEqual(TEXT("Replay uses the recorded intensity and limiter decisions"), State->GetOutput().FlashAlpha, 0.25f, 0.001f);
	TestEqual(TEXT("Replay uses the recorded seed"), State->GetPreviewParams().InstanceSeed, 99);

	bool bSilenced = false;
	TestTrue(TEXT("Suppressed track explains itself"), State->GetTrackDecision(0, &bSilenced).ToString().Contains(TEXT("suppressed")));
	TestTrue(TEXT("Suppressed track is silenced"), bSilenced);
	TestTrue(TEXT("Softened track explains itself"), State->GetTrackDecision(1, &bSilenced).ToString().Contains(TEXT("softened")));
	TestFalse(TEXT("Softened track still plays"), bSilenced);

	// Slider edits during a replay change the replayed play, not the normal preview values.
	Capture.TrackScales = { 1.0f, 0.0f };
	State->LoadCapture(Capture);
	State->SetTime(0.5f);
	State->SetPreviewParameterValue(Parameter.Name, 0.5f);
	TestEqual(TEXT("Parameter edits apply to the replay"), State->GetActiveCapture()->ParameterValues.FindRef(Parameter.Name), 0.5f);
	TestEqual(TEXT("Replayed output follows the edit"), State->GetOutput().FlashAlpha, 0.25f, 0.001f);

	State->ClearCapture();
	TestNull(TEXT("Replay cleared"), State->GetActiveCapture());
	State->SetTime(0.5f);
	TestEqual(TEXT("Normal preview is back with its own values"), State->GetOutput().FlashAlpha, 1.0f, 0.001f);

	// Decisions for muted tracks and chance.
	Recipe->Tracks[0].bEnabled = false;
	TestTrue(TEXT("Muted track"), State->GetTrackDecision(0).ToString().Contains(TEXT("Muted")));
	Recipe->Tracks[0].bEnabled = true;
	Recipe->Tracks[1].Conditions.Chance = 0.0f;
	TestTrue(TEXT("Chance 0 is skipped"), State->GetTrackDecision(1, &bSilenced).ToString().Contains(TEXT("chance")));
	TestTrue(TEXT("Skipped track is silenced"), bSilenced);

	// Output bypass for Off/On captures.
	Recipe->Tracks[1].Conditions.Chance = 1.0f;
	State->SetOutputBypassed(true);
	TestEqual(TEXT("Bypassed preview has no output"), State->GetOutput().FlashAlpha, 0.0f);
	State->SetOutputBypassed(false);
	TestTrue(TEXT("Output returns after bypass"), State->GetOutput().FlashAlpha > 0.0f);
	return true;
}

#endif
