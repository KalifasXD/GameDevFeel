// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "FeelActorDelivery.h"
#include "FeelAudioDelivery.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "FeelTrackLifecycle.h"
#include "FeelWidgetDelivery.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundClass.h"
#include "Steps/FeelStep_ActorExtras.h"
#include "Steps/FeelStep_AudioMix.h"
#include "Steps/FeelStep_CameraExtras.h"
#include "Steps/FeelStep_Meta.h"
#include "Steps/FeelStep_ScreenColor.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "Steps/FeelStep_Widget.h"
#include "Tests/FeelLifecycleTestTypes.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelSensesTests
{
	FFeelStepEvalContext MakeContext(float Alpha, float Duration = 1.0f, int32 Seed = 3)
	{
		FFeelStepEvalContext Context;
		Context.Duration = Duration;
		Context.Alpha = Alpha;
		Context.LocalTime = Alpha * Duration;
		Context.Intensity = 1.0f;
		Context.Seed = Seed;
		return Context;
	}

	FFeelFrameOutput EvaluateStep(const UFeelStep& Step, const FFeelStepEvalContext& Context)
	{
		FFeelOutputAccumulator Accumulator;
		Step.Evaluate(Context, Accumulator);
		return Accumulator.Output;
	}

	UFeelRecipe* MakeFlashRecipe(float StartTime, float Duration)
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = StartTime;
		Track.Duration = Duration;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSensesOutputTest, "FeelKit.Output.SensesChannels", FEEL_TEST_FLAGS)
bool FFeelSensesOutputTest::RunTest(const FString& Parameters)
{
	FFeelOutputAccumulator Accumulator;
	Accumulator.AddScreenTint(FLinearColor::Red, 0.3f);
	Accumulator.AddScreenTint(FLinearColor::Blue, 0.6f);
	Accumulator.AddScreenTint(FLinearColor::Green, 0.2f);
	TestTrue(TEXT("The strongest tint wins"), Accumulator.Output.TintColor.Equals(FLinearColor::Blue) && FMath::IsNearlyEqual(Accumulator.Output.TintWeight, 0.6f));

	Accumulator.AddScreenFade(FLinearColor::Black, 0.5f);
	Accumulator.AddScreenFlash(FLinearColor::White, 0.5f);
	FLinearColor Overlay(0.0f, 0.0f, 0.0f, 0.0f);
	Accumulator.Output.ApplyOverlay(Overlay);
	TestEqual(TEXT("Fade and flash combine: 1 - 0.5 x 0.5"), Overlay.A, 0.75f, 0.001f);
	TestEqual(TEXT("The flash is drawn over the fade"), Overlay.R, 0.5f / 0.75f, 0.001f);

	Accumulator.AddPostProcess(EFeelPostProcessParameter::Saturation, 0.2f, 0.8f);
	Accumulator.AddTargetTransform(FVector(1.0, 0.0, 0.0), FRotator(0.0, 5.0, 0.0));
	Accumulator.AddTargetTransform(FVector(2.0, 0.0, 0.0), FRotator(0.0, 5.0, 0.0));
	TestTrue(TEXT("Transform offsets add"), Accumulator.Output.TargetLocationOffset.Equals(FVector(3.0, 0.0, 0.0)) && FMath::IsNearlyEqual(Accumulator.Output.TargetRotationOffset.Yaw, 10.0));

	int32 BlendCount = 0;
	bool bSawSaturation = false;
	bool bSawTint = false;
	Accumulator.Output.ForEachPostProcessBlend([&](FPostProcessSettings& Settings, float Weight)
	{
		++BlendCount;
		bSawSaturation |= Settings.bOverride_ColorSaturation && FMath::IsNearlyEqual(Settings.ColorSaturation.X, 0.2f) && FMath::IsNearlyEqual(Weight, 0.8f);
		bSawTint |= Settings.bOverride_SceneColorTint && FMath::IsNearlyEqual(Weight, 0.6f);
	});
	TestEqual(TEXT("One blend each for saturation and tint"), BlendCount, 2);
	TestTrue(TEXT("Saturation blends with its weight"), bSawSaturation);
	TestTrue(TEXT("Tint blends with its weight"), bSawTint);

	USoundClass* ClassA = NewObject<USoundClass>(GetTransientPackage());
	FFeelSoundClassAdjust Duck;
	Duck.SoundClass = ClassA;
	Duck.Volume = 0.4f;
	FFeelSoundClassAdjust Bend;
	Bend.SoundClass = ClassA;
	Bend.Volume = 0.7f;
	Bend.Pitch = 0.6f;
	Bend.LowPassFrequency = 900.0f;
	Accumulator.AddSoundClassAdjust(Duck);
	Accumulator.AddSoundClassAdjust(Bend);
	TestEqual(TEXT("Adjusts of one class merge into one"), Accumulator.Output.SoundClassAdjusts.Num(), 1);
	TestEqual(TEXT("The lowest volume wins"), Accumulator.Output.SoundClassAdjusts[0].Volume, 0.4f);
	TestEqual(TEXT("The pitch farthest from 1 wins"), Accumulator.Output.SoundClassAdjusts[0].Pitch, 0.6f);
	TestEqual(TEXT("The lowest cutoff wins"), Accumulator.Output.SoundClassAdjusts[0].LowPassFrequency, 900.0f);

	TestEqual(TEXT("20 kHz means no filtering"), FFeelAudioDelivery::FindLowPassLevel(20000.0f), 0);
	TestEqual(TEXT("Cutoffs snap to the nearest level"), FFeelAudioDelivery::FindLowPassLevel(1100.0f), 4);
	TestEqual(TEXT("Very low cutoffs use the strongest level"), FFeelAudioDelivery::FindLowPassLevel(100.0f), FFeelAudioDelivery::GetLowPassLevels().Num() - 1);

	Accumulator.AddWidgetTransform(FVector2D(4.0, 0.0), FVector2D(0.1, 0.1), 3.0f);
	Accumulator.AddWidgetTransform(FVector2D(1.0, 0.0), FVector2D(0.1, 0.1), 0.0f);
	Accumulator.AddWidgetColor(FLinearColor::Red, 0.4f);
	TestTrue(TEXT("Widget transforms add"), Accumulator.Output.WidgetTranslation.Equals(FVector2D(5.0, 0.0)) && Accumulator.Output.WidgetScaleDelta.Equals(FVector2D(0.2, 0.2)));
	TestEqual(TEXT("Widget color weight is kept"), Accumulator.Output.WidgetColorWeight, 0.4f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSensesStepsTest, "FeelKit.Steps.Senses", FEEL_TEST_FLAGS)
bool FFeelSensesStepsTest::RunTest(const FString& Parameters)
{
	using namespace FeelSensesTests;

	// Camera Roll: rolls, side chosen by the seed.
	TStrongObjectPtr<UFeelStep_CameraRoll> Roll(NewObject<UFeelStep_CameraRoll>(GetTransientPackage()));
	Roll->Shape = EFeelMotionShape::Smooth;
	const double RollA = EvaluateStep(*Roll, MakeContext(0.5f, 1.0f, 2)).CameraRotationOffset.Roll;
	const double RollB = EvaluateStep(*Roll, MakeContext(0.5f, 1.0f, 3)).CameraRotationOffset.Roll;
	TestEqual(TEXT("Camera Roll reaches its angle"), FMath::Abs(RollA), 4.0, 0.01);
	TestTrue(TEXT("Camera Roll side depends on the seed"), RollA * RollB < 0.0);

	// Camera Zoom: holds the full change in the middle, returns at the end.
	TStrongObjectPtr<UFeelStep_CameraZoom> Zoom(NewObject<UFeelStep_CameraZoom>(GetTransientPackage()));
	TestEqual(TEXT("Camera Zoom holds its change"), EvaluateStep(*Zoom, MakeContext(0.5f)).FieldOfViewOffset, -10.0f, 0.01f);
	TestEqual(TEXT("Camera Zoom returns at the end"), EvaluateStep(*Zoom, MakeContext(1.0f)).FieldOfViewOffset, 0.0f, 0.01f);

	// Look-at Nudge: turns toward the location; nothing without one.
	TStrongObjectPtr<UFeelStep_LookAtNudge> Nudge(NewObject<UFeelStep_LookAtNudge>(GetTransientPackage()));
	Nudge->Shape = EFeelMotionShape::Smooth;
	FFeelStepEvalContext NudgeContext = MakeContext(0.5f);
	TestTrue(TEXT("Look-at Nudge does nothing without a location or direction"), EvaluateStep(*Nudge, NudgeContext).CameraRotationOffset.IsNearlyZero());
	NudgeContext.ViewDirectionFromLocation = FVector(0.0, -1.0, 0.0); // The location is to the right.
	TestTrue(TEXT("Look-at Nudge turns toward the location"), EvaluateStep(*Nudge, NudgeContext).CameraRotationOffset.Yaw > 0.0);

	// Screen color steps.
	TStrongObjectPtr<UFeelStep_Desaturate> Desaturate(NewObject<UFeelStep_Desaturate>(GetTransientPackage()));
	const FFeelFrameOutput DesaturateOutput = EvaluateStep(*Desaturate, MakeContext(0.5f));
	const FFeelPostProcessContribution Saturation = DesaturateOutput.GetPostProcess(EFeelPostProcessParameter::Saturation);
	TestEqual(TEXT("Desaturate blends toward reduced saturation"), Saturation.Value, 0.2f, 0.001f);
	TestTrue(FString::Printf(TEXT("Desaturate is at full weight at its peak (weight %f)"), Saturation.Weight), Saturation.Weight > 0.99f);

	TStrongObjectPtr<UFeelStep_ScreenFade> Fade(NewObject<UFeelStep_ScreenFade>(GetTransientPackage()));
	TestEqual(TEXT("Screen Fade holds full opacity"), EvaluateStep(*Fade, MakeContext(0.5f)).FadeAlpha, 1.0f, 0.01f);
	TestEqual(TEXT("Screen Fade starts clear"), EvaluateStep(*Fade, MakeContext(0.0f)).FadeAlpha, 0.0f, 0.01f);

	// Mesh Wobble: moves in the middle, settles at the end.
	TStrongObjectPtr<UFeelStep_MeshWobble> Wobble(NewObject<UFeelStep_MeshWobble>(GetTransientPackage()));
	TestTrue(TEXT("Mesh Wobble tilts during the track"), !EvaluateStep(*Wobble, MakeContext(0.12f)).TargetRotationOffset.IsNearlyZero());
	TestTrue(TEXT("Mesh Wobble settles by the end"), EvaluateStep(*Wobble, MakeContext(1.0f)).TargetRotationOffset.IsNearlyZero());
	TestTrue(TEXT("Mesh Wobble is the same when scrubbed again"), EvaluateStep(*Wobble, MakeContext(0.37f)).TargetRotationOffset.Equals(EvaluateStep(*Wobble, MakeContext(0.37f)).TargetRotationOffset));

	// Light Flash with flicker: deterministic pattern.
	TStrongObjectPtr<UFeelStep_LightFlash> Light(NewObject<UFeelStep_LightFlash>(GetTransientPackage()));
	Light->bFlicker = true;
	int32 OnSlots = 0;
	bool bDeterministic = true;
	for (int32 Sample = 0; Sample < 40; ++Sample)
	{
		const float Alpha = 0.1f + 0.02f * Sample;
		const float Delta = EvaluateStep(*Light, MakeContext(Alpha)).LightIntensityDelta;
		bDeterministic &= FMath::IsNearlyEqual(Delta, EvaluateStep(*Light, MakeContext(Alpha)).LightIntensityDelta);
		OnSlots += Delta > 0.0f ? 1 : 0;
	}
	TestTrue(TEXT("Light flicker switches on and off"), OnSlots > 5 && OnSlots < 35);
	TestTrue(TEXT("Light flicker is the same when scrubbed again"), bDeterministic);

	// Audio steps.
	USoundClass* Class = NewObject<USoundClass>(GetTransientPackage());
	TStrongObjectPtr<UFeelStep_SoundClassDuck> Duck(NewObject<UFeelStep_SoundClassDuck>(GetTransientPackage()));
	Duck->SoundClass = Class;
	const FFeelFrameOutput DuckOutput = EvaluateStep(*Duck, MakeContext(0.5f));
	TestTrue(TEXT("Sound Class Duck lowers the volume while held"), DuckOutput.SoundClassAdjusts.Num() == 1 && FMath::IsNearlyEqual(DuckOutput.SoundClassAdjusts[0].Volume, 0.4f, 0.01f));

	TStrongObjectPtr<UFeelStep_PitchBend> Bend(NewObject<UFeelStep_PitchBend>(GetTransientPackage()));
	Bend->SoundClass = Class;
	TestEqual(TEXT("Pitch Bend changes the pitch while held"), EvaluateStep(*Bend, MakeContext(0.5f)).SoundClassAdjusts[0].Pitch, 0.7f, 0.01f);

	TStrongObjectPtr<UFeelStep_LowPassSweep> LowPass(NewObject<UFeelStep_LowPassSweep>(GetTransientPackage()));
	LowPass->SoundClass = Class;
	TestEqual(TEXT("Low-pass Sweep reaches its cutoff while held"), EvaluateStep(*LowPass, MakeContext(0.5f)).SoundClassAdjusts[0].LowPassFrequency, 800.0f, 1.0f);
	TestTrue(TEXT("Low-pass Sweep is open at the start"), EvaluateStep(*LowPass, MakeContext(0.0f)).SoundClassAdjusts[0].LowPassFrequency > 19000.0f);

	// Widget steps.
	TStrongObjectPtr<UFeelStep_WidgetPunch> Punch(NewObject<UFeelStep_WidgetPunch>(GetTransientPackage()));
	Punch->Shape = EFeelMotionShape::Smooth;
	TestTrue(TEXT("Widget Punch scales the widget"), EvaluateStep(*Punch, MakeContext(0.5f)).WidgetScaleDelta.Equals(FVector2D(0.2, 0.2), 0.01));
	TStrongObjectPtr<UFeelStep_WidgetShake> Shake(NewObject<UFeelStep_WidgetShake>(GetTransientPackage()));
	TestTrue(TEXT("Widget Shake settles by the end"), EvaluateStep(*Shake, MakeContext(1.0f)).WidgetTranslation.IsNearlyZero());
	TStrongObjectPtr<UFeelStep_WidgetFlash> WidgetFlash(NewObject<UFeelStep_WidgetFlash>(GetTransientPackage()));
	TestTrue(TEXT("Widget Flash tints during the track"), EvaluateStep(*WidgetFlash, MakeContext(0.15f)).WidgetColorWeight > 0.5f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSensesDeliveryTest, "FeelKit.Output.TransformLightWidgetDelivery", FEEL_TEST_FLAGS)
bool FFeelSensesDeliveryTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	AActor* Actor = World->SpawnActor<AActor>();
	USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
	Actor->SetRootComponent(Root);
	Root->RegisterComponent();
	UPointLightComponent* Light = NewObject<UPointLightComponent>(Actor, TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->RegisterComponent();
	Light->SetIntensity(100.0f);

	FFeelActorDelivery Delivery;

	// Transform offsets are removed from wherever the component is, so movement from elsewhere survives.
	Delivery.BeginFrame();
	Delivery.AddTransform(Root, FVector(0.0, 0.0, 10.0), FRotator::ZeroRotator);
	Delivery.EndFrame();
	TestTrue(TEXT("Offset applied"), Root->GetRelativeLocation().Equals(FVector(0.0, 0.0, 10.0), 0.001));

	Root->SetRelativeLocation(Root->GetRelativeLocation() + FVector(100.0, 0.0, 0.0)); // Something else moves the actor.
	Delivery.BeginFrame();
	Delivery.AddTransform(Root, FVector(0.0, 0.0, 5.0), FRotator::ZeroRotator);
	Delivery.EndFrame();
	TestTrue(TEXT("Offsets follow a moving component"), Root->GetRelativeLocation().Equals(FVector(100.0, 0.0, 5.0), 0.001));

	Delivery.BeginFrame();
	Delivery.AddLight(Root, 2.0f, FLinearColor::Red, 1.0f);
	Delivery.EndFrame();
	TestTrue(TEXT("Transform restored while keeping the outside movement"), Root->GetRelativeLocation().Equals(FVector(100.0, 0.0, 0.0), 0.001));
	TestEqual(TEXT("Light intensity multiplied"), Light->Intensity, 300.0f, 0.01f);

	Delivery.BeginFrame();
	Delivery.EndFrame();
	TestEqual(TEXT("Light intensity restored"), Light->Intensity, 100.0f, 0.01f);
	TestTrue(TEXT("Light color restored"), Light->GetLightColor().Equals(FLinearColor::White, 0.01f));

	// Widgets.
	UTextBlock* Text = NewObject<UTextBlock>(GetTransientPackage());
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FFeelWidgetDelivery WidgetDelivery;
	WidgetDelivery.BeginFrame();
	WidgetDelivery.AddWidget(Text, FVector2D(10.0, 0.0), FVector2D(0.5, 0.5), 0.0f, FLinearColor::Red, 1.0f);
	WidgetDelivery.EndFrame();
	TestTrue(TEXT("Widget moved and scaled"), Text->GetRenderTransform().Translation.Equals(FVector2D(10.0, 0.0)) && Text->GetRenderTransform().Scale.Equals(FVector2D(1.5, 1.5)));
	TestTrue(TEXT("Widget tinted"), Text->GetColorAndOpacity().GetSpecifiedColor().Equals(FLinearColor::Red, 0.01f));
	WidgetDelivery.BeginFrame();
	WidgetDelivery.EndFrame();
	TestTrue(TEXT("Widget transform restored"), Text->GetRenderTransform().Translation.IsZero() && Text->GetRenderTransform().Scale.Equals(FVector2D(1.0, 1.0)));
	TestTrue(TEXT("Widget color restored"), Text->GetColorAndOpacity().GetSpecifiedColor().Equals(FLinearColor::White, 0.01f));

	World->RemoveFromRoot();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMetaStepsTest, "FeelKit.Steps.RecipeAndRandomChoice", FEEL_TEST_FLAGS)
bool FFeelMetaStepsTest::RunTest(const FString& Parameters)
{
	using namespace FeelSensesTests;

	// Random choice: weights, stable per play, varies across plays.
	TStrongObjectPtr<UFeelRecipe> ChoiceRecipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	UFeelStep_RandomChoice* Choice = NewObject<UFeelStep_RandomChoice>(ChoiceRecipe.Get());
	UFeelStep_ScreenFlash* OptionA = NewObject<UFeelStep_ScreenFlash>(Choice);
	UFeelStep_ScreenFlash* OptionB = NewObject<UFeelStep_ScreenFlash>(Choice);
	Choice->Options.Add({ OptionA, 3.0f });
	Choice->Options.Add({ OptionB, 1.0f });
	Choice->Options.Add({ nullptr, 5.0f });
	FFeelTrack& ChoiceTrack = ChoiceRecipe->Tracks.AddDefaulted_GetRef();
	ChoiceTrack.Step = Choice;
	ChoiceTrack.Duration = 1.0f;

	TestTrue(TEXT("Empty options are never picked"), Choice->ChooseOption(0.99f) != nullptr);
	int32 PickedA = 0;
	bool bStable = true;
	FFeelEvalParams Params;
	for (int32 Seed = 0; Seed < 400; ++Seed)
	{
		Params.InstanceSeed = Seed;
		float ComfortScale = 1.0f;
		const UFeelStep* First = FFeelEvaluator::ResolveTrackStep(*ChoiceRecipe, 0, Params, ComfortScale);
		bStable &= First == FFeelEvaluator::ResolveTrackStep(*ChoiceRecipe, 0, Params, ComfortScale);
		PickedA += First == OptionA ? 1 : 0;
	}
	TestTrue(TEXT("The pick is fixed for one play"), bStable);
	TestTrue(FString::Printf(TEXT("Weights set how often options are picked (A picked %d of 400, expected about 300)"), PickedA), PickedA > 260 && PickedA < 340);

	// Play Recipe: the inner recipe plays at the track's local time and intensity.
	TStrongObjectPtr<UFeelRecipe> Inner(MakeFlashRecipe(0.2f, 0.3f));
	TStrongObjectPtr<UFeelRecipe> Outer(NewObject<UFeelRecipe>(GetTransientPackage()));
	UFeelStep_Recipe* RecipeStep = NewObject<UFeelStep_Recipe>(Outer.Get());
	RecipeStep->Recipe = Inner.Get();
	RecipeStep->IntensityScale = 0.5f;
	FFeelTrack& OuterTrack = Outer->Tracks.AddDefaulted_GetRef();
	OuterTrack.Step = RecipeStep;
	OuterTrack.StartTime = 1.0f;
	OuterTrack.Duration = 1.0f;
	OuterTrack.IntensityCurve.GetRichCurve()->Reset();

	FFeelEvalParams OuterParams;
	FFeelOutputAccumulator Before;
	FFeelEvaluator::Evaluate(*Outer, 1.1f, OuterParams, Before);
	TestEqual(TEXT("Inner track not reached yet"), Before.Output.FlashAlpha, 0.0f);
	FFeelOutputAccumulator During;
	FFeelEvaluator::Evaluate(*Outer, 1.3f, OuterParams, During);
	TestEqual(TEXT("Inner track plays at its local time with the scaled intensity"), During.Output.FlashAlpha, 0.5f, 0.001f);

	TArray<FFeelChannelIntensity> Channels;
	FFeelEvaluator::SampleChannelIntensities(*Outer, 21, OuterParams, Channels);
	TestTrue(TEXT("The intensity graph shows the inner recipe's channel"), Channels.Num() == 1 && Channels[0].Channel == FeelTags::Screen_Flash);

	// Self reference is ignored instead of recursing forever.
	RecipeStep->Recipe = Outer.Get();
	FFeelOutputAccumulator Self;
	TestEqual(TEXT("A recipe playing itself does nothing"), FFeelEvaluator::Evaluate(*Outer, 1.3f, OuterParams, Self), 0);
	RecipeStep->Recipe = Inner.Get();

	// Side effects of inner tracks start and stop through the outer lifecycle.
	UFeelTestRecorderStep* Recorder = NewObject<UFeelTestRecorderStep>(Inner.Get());
	FFeelTrack& RecorderTrack = Inner->Tracks.AddDefaulted_GetRef();
	RecorderTrack.Step = Recorder;
	RecorderTrack.StartTime = 0.1f;
	RecorderTrack.Duration = 0.2f;

	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Outer->Tracks.Num());
	auto MakeLifecycleContext = [](int32 TrackIndex, float Intensity)
	{
		FFeelContext Context;
		Context.TrackIndex = TrackIndex;
		Context.Intensity = Intensity;
		return Context;
	};
	Lifecycle.Update(*Outer, 1.05f, OuterParams, MakeLifecycleContext);
	TestEqual(TEXT("Inner side-effect track not started before its time"), Recorder->StartCount, 0);
	Lifecycle.Update(*Outer, 1.15f, OuterParams, MakeLifecycleContext);
	TestEqual(TEXT("Inner side-effect track starts at its local time"), Recorder->StartCount, 1);
	TestTrue(TEXT("Inner track indices are kept apart from outer ones"), Recorder->LastTrackIndex >= 10000);
	Lifecycle.Update(*Outer, 1.5f, OuterParams, MakeLifecycleContext);
	TestEqual(TEXT("Inner side-effect track stops at its end"), Recorder->StopCount, 1);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
