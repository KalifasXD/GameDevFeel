// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelArbiters.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "Steps/FeelStep_Hitstop.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelArbiterTests
{
	/** Stand-in for world settings or actors: any object whose "dilation" lives in a map. */
	struct FFakeClocks
	{
		TMap<UObject*, float> Dilations;
		int32 Writes = 0;

		float Read(UObject* Target) const
		{
			const float* Value = Dilations.Find(Target);
			return Value ? *Value : 1.0f;
		}

		void Write(UObject* Target, float Dilation)
		{
			Dilations.Add(Target, Dilation);
			++Writes;
		}
	};

	FFeelTimeRequest MakeRequest(float Dilation, int32 Priority)
	{
		FFeelTimeRequest Request;
		Request.Merge(Dilation, Priority);
		return Request;
	}

	void ApplyFrame(FFeelTimeArbiter& Arbiter, FFakeClocks& Clocks)
	{
		Arbiter.Apply(
			[&Clocks](UObject* Target) { return Clocks.Read(Target); },
			[&Clocks](UObject* Target, float Dilation) { Clocks.Write(Target, Dilation); });
	}

	UObject* MakeClock()
	{
		return NewObject<UFeelRecipe>(GetTransientPackage());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCameraStrongestWinsTest, "FeelKit.Arbiter.CameraStrongestWins", FEEL_TEST_FLAGS)
bool FFeelCameraStrongestWinsTest::RunTest(const FString& Parameters)
{
	FFeelFrameOutput A;
	A.CameraLocationOffset = FVector(10.0, 0.0, 0.0);
	A.CameraRotationOffset = FRotator(1.0, 0.0, 0.0);
	A.FieldOfViewOffset = 2.0f;

	FFeelFrameOutput B;
	B.CameraLocationOffset = FVector(0.0, 0.0, -20.0);
	B.CameraRotationOffset = FRotator(0.0, -3.0, 0.0);
	B.FieldOfViewOffset = -1.0f;

	const TArray<const FFeelFrameOutput*> Inputs = { &A, &B };
	FFeelFrameOutput Result;
	FeelArbiters::ResolveCamera(Inputs, EFeelCameraArbitration::StrongestWins, FFeelCameraCaps(), Result);

	TestEqual(TEXT("Largest location offset wins"), Result.CameraLocationOffset, FVector(0.0, 0.0, -20.0));
	TestEqual(TEXT("Largest rotation offset wins"), Result.CameraRotationOffset, FRotator(0.0, -3.0, 0.0));
	TestEqual(TEXT("Largest field of view change wins"), Result.FieldOfViewOffset, 2.0f);

	const TArray<const FFeelFrameOutput*> Single = { &A };
	FeelArbiters::ResolveCamera(Single, EFeelCameraArbitration::StrongestWins, FFeelCameraCaps(), Result);
	TestEqual(TEXT("A single instance passes through"), Result.CameraLocationOffset, A.CameraLocationOffset);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCameraAdditiveCappedTest, "FeelKit.Arbiter.CameraAdditiveCapped", FEEL_TEST_FLAGS)
bool FFeelCameraAdditiveCappedTest::RunTest(const FString& Parameters)
{
	FFeelCameraCaps Caps;
	Caps.MaxLocationOffset = 30.0f;
	Caps.MaxRotationOffset = 8.0f;
	Caps.MaxFieldOfViewOffset = 15.0f;

	FFeelFrameOutput A;
	A.CameraLocationOffset = FVector(5.0, 0.0, 0.0);
	A.CameraRotationOffset = FRotator(2.0, 0.0, 0.0);
	A.FieldOfViewOffset = 4.0f;

	FFeelFrameOutput B = A;

	TArray<const FFeelFrameOutput*> Inputs = { &A, &B };
	FFeelFrameOutput Result;
	FeelArbiters::ResolveCamera(Inputs, EFeelCameraArbitration::AdditiveCapped, Caps, Result);
	TestEqual(TEXT("Below the caps, locations add"), Result.CameraLocationOffset, FVector(10.0, 0.0, 0.0));
	TestEqual(TEXT("Below the caps, rotations add"), Result.CameraRotationOffset, FRotator(4.0, 0.0, 0.0));
	TestEqual(TEXT("Below the caps, field of view adds"), Result.FieldOfViewOffset, 8.0f);

	A.CameraLocationOffset = FVector(20.0, 0.0, 0.0);
	B.CameraLocationOffset = FVector(20.0, 0.0, 0.0);
	A.CameraRotationOffset = FRotator(5.0, -6.0, 0.0);
	B.CameraRotationOffset = FRotator(5.0, -6.0, 0.0);
	A.FieldOfViewOffset = 10.0f;
	B.FieldOfViewOffset = 10.0f;
	FeelArbiters::ResolveCamera(Inputs, EFeelCameraArbitration::AdditiveCapped, Caps, Result);
	TestTrue(TEXT("Location is capped by length"), FMath::IsNearlyEqual(Result.CameraLocationOffset.Size(), 30.0, 0.001));
	TestEqual(TEXT("Pitch is capped"), Result.CameraRotationOffset.Pitch, 8.0);
	TestEqual(TEXT("Negative yaw is capped"), Result.CameraRotationOffset.Yaw, -8.0);
	TestEqual(TEXT("Field of view is capped"), Result.FieldOfViewOffset, 15.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelScreenStrongestWinsTest, "FeelKit.Arbiter.ScreenStrongestWins", FEEL_TEST_FLAGS)
bool FFeelScreenStrongestWinsTest::RunTest(const FString& Parameters)
{
	FFeelFrameOutput Red;
	Red.FlashColor = FLinearColor::Red;
	Red.FlashAlpha = 0.4f;

	FFeelFrameOutput Blue;
	Blue.FlashColor = FLinearColor::Blue;
	Blue.FlashAlpha = 0.7f;

	const TArray<const FFeelFrameOutput*> Inputs = { &Red, &Blue };
	FFeelFrameOutput Result;
	FeelArbiters::ResolveScreen(Inputs, Result);
	TestEqual(TEXT("Strongest flash alpha wins"), Result.FlashAlpha, 0.7f);
	TestEqual(TEXT("Strongest flash keeps its color"), Result.FlashColor, FLinearColor::Blue);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTimePriorityTest, "FeelKit.Arbiter.TimePriority", FEEL_TEST_FLAGS)
bool FFeelTimePriorityTest::RunTest(const FString& Parameters)
{
	using namespace FeelArbiterTests;

	UObject* Clock = MakeClock();
	FFakeClocks Clocks;
	FFeelTimeArbiter Arbiter;

	Arbiter.Submit(Clock, MakeRequest(0.01f, 0));
	Arbiter.Submit(Clock, MakeRequest(0.5f, 1));
	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("Higher priority wins even with a weaker dilation"), Clocks.Read(Clock), 0.5f);

	Arbiter.Submit(Clock, MakeRequest(0.3f, 2));
	Arbiter.Submit(Clock, MakeRequest(0.1f, 2));
	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("Equal priority keeps the strongest dilation"), Clocks.Read(Clock), 0.1f);

	FFeelOutputAccumulator Accumulator;
	UFeelStep_GlobalHitstop* Hitstop = NewObject<UFeelStep_GlobalHitstop>(GetTransientPackage());
	Hitstop->TimeDilation = 0.05f;
	FFeelStepEvalContext Context;
	Context.Intensity = 1.0f;
	Hitstop->Evaluate(Context, Accumulator);
	TestTrue(TEXT("Global Hitstop requests global time"), Accumulator.Output.GlobalTimeDilation.bActive && !Accumulator.Output.TargetTimeDilation.bActive);
	TestEqual(TEXT("Global Hitstop at full intensity uses its dilation"), Accumulator.Output.GlobalTimeDilation.Dilation, 0.05f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTimeRestoreTest, "FeelKit.Arbiter.TimeRestore", FEEL_TEST_FLAGS)
bool FFeelTimeRestoreTest::RunTest(const FString& Parameters)
{
	using namespace FeelArbiterTests;

	UObject* Clock = MakeClock();
	FFakeClocks Clocks;
	Clocks.Write(Clock, 0.8f);
	FFeelTimeArbiter Arbiter;

	// Stop: the request disappears, the game's previous dilation comes back.
	Arbiter.Submit(Clock, MakeRequest(0.05f, 0));
	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("Hitstop applies"), Clocks.Read(Clock), 0.05f);
	TestTrue(TEXT("Arbiter owns the clock"), Arbiter.IsOwning(Clock));

	Arbiter.Submit(Clock, MakeRequest(0.2f, 0));
	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("A later request changes the dilation"), Clocks.Read(Clock), 0.2f);

	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("Previous dilation restored on stop, not an intermediate value"), Clocks.Read(Clock), 0.8f);
	TestFalse(TEXT("Arbiter releases the clock"), Arbiter.IsOwningAny());

	// Cancel: world teardown restores everything at once.
	UObject* OtherClock = MakeClock();
	Arbiter.Submit(Clock, MakeRequest(0.05f, 0));
	Arbiter.Submit(OtherClock, MakeRequest(0.1f, 0));
	ApplyFrame(Arbiter, Clocks);
	Arbiter.RestoreAll([&Clocks](UObject* Target, float Dilation) { Clocks.Write(Target, Dilation); });
	TestEqual(TEXT("RestoreAll restores the first clock"), Clocks.Read(Clock), 0.8f);
	TestEqual(TEXT("RestoreAll restores the second clock"), Clocks.Read(OtherClock), 1.0f);
	TestFalse(TEXT("RestoreAll releases everything"), Arbiter.IsOwningAny());

	// Target destroyed: nothing is written to it and ownership is dropped.
	UObject* DoomedClock = MakeClock();
	Arbiter.Submit(DoomedClock, MakeRequest(0.05f, 0));
	ApplyFrame(Arbiter, Clocks);
	DoomedClock->MarkAsGarbage();
	const int32 WritesBefore = Clocks.Writes;
	ApplyFrame(Arbiter, Clocks);
	TestEqual(TEXT("Destroyed targets are not written to"), Clocks.Writes, WritesBefore);
	TestFalse(TEXT("Destroyed targets are released"), Arbiter.IsOwningAny());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCooldownTest, "FeelKit.Runtime.Cooldown", FEEL_TEST_FLAGS)
bool FFeelCooldownTest::RunTest(const FString& Parameters)
{
	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	Recipe->Cooldown = 1.0f;
	const FObjectKey TargetA(NewObject<UFeelRecipe>(GetTransientPackage()));
	const FObjectKey TargetB(NewObject<UFeelRecipe>(GetTransientPackage()));

	FFeelPlaybackGate Gate;
	TestTrue(TEXT("First play is allowed"), Gate.CanPlay(*Recipe, TargetA, 10.0, 0));
	Gate.NotifyPlayed(*Recipe, TargetA, 10.0);

	TestFalse(TEXT("Blocked during the cooldown"), Gate.CanPlay(*Recipe, TargetA, 10.5, 0));
	TestTrue(TEXT("Allowed once the cooldown has passed"), Gate.CanPlay(*Recipe, TargetA, 11.0, 0));
	TestTrue(TEXT("Cooldown is per target"), Gate.CanPlay(*Recipe, TargetB, 10.5, 0));

	UFeelRecipe* OtherRecipe = NewObject<UFeelRecipe>(GetTransientPackage());
	OtherRecipe->Cooldown = 1.0f;
	TestTrue(TEXT("Cooldown is per recipe"), Gate.CanPlay(*OtherRecipe, TargetA, 10.5, 0));

	Recipe->Cooldown = 0.0f;
	TestTrue(TEXT("Zero cooldown never blocks"), Gate.CanPlay(*Recipe, TargetA, 10.1, 0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMaxConcurrentTest, "FeelKit.Runtime.MaxConcurrent", FEEL_TEST_FLAGS)
bool FFeelMaxConcurrentTest::RunTest(const FString& Parameters)
{
	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	const FObjectKey Target(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelPlaybackGate Gate;

	Recipe->MaxConcurrent = 2;
	TestTrue(TEXT("Below the limit"), Gate.CanPlay(*Recipe, Target, 0.0, 1));
	TestFalse(TEXT("At the limit"), Gate.CanPlay(*Recipe, Target, 0.0, 2));

	Recipe->MaxConcurrent = 0;
	TestTrue(TEXT("Zero means unlimited"), Gate.CanPlay(*Recipe, Target, 0.0, 100));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelOverlayCompositeTest, "FeelKit.Output.OverlayComposite", FEEL_TEST_FLAGS)
bool FFeelOverlayCompositeTest::RunTest(const FString& Parameters)
{
	FFeelFrameOutput Flash;
	Flash.FlashColor = FLinearColor::White;
	Flash.FlashAlpha = 0.5f;

	FLinearColor NoFade = FLinearColor::Transparent;
	Flash.ApplyOverlay(NoFade);
	TestEqual(TEXT("Without a fade the overlay is the flash"), NoFade, FLinearColor(1.0f, 1.0f, 1.0f, 0.5f));

	// Half black fade under a half white flash: scene * 0.25 + 0.5 white.
	FLinearColor HalfFade(0.0f, 0.0f, 0.0f, 0.5f);
	Flash.ApplyOverlay(HalfFade);
	TestEqual(TEXT("Combined alpha"), HalfFade.A, 0.75f);
	TestEqual(TEXT("Combined color keeps the white contribution"), HalfFade.R * HalfFade.A, 0.5f);

	FFeelFrameOutput NoFlash;
	FLinearColor FullFade(0.0f, 0.0f, 0.0f, 1.0f);
	NoFlash.ApplyOverlay(FullFade);
	TestEqual(TEXT("No flash leaves the game's fade untouched"), FullFade, FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
