// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelEditorSettings.h"
#include "FeelRecipeEditorState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelEditorSnapTest, "FeelKit.Editor.SnapToFrames", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelEditorSnapTest::RunTest(const FString& Parameters)
{
	UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
	const bool bOriginalSnap = Settings->bSnapToFrames;
	const int32 OriginalFrameRate = Settings->SnapFrameRate;

	const FFeelRecipeEditorState State(nullptr);

	Settings->bSnapToFrames = true;
	Settings->SnapFrameRate = 10;
	TestEqual(TEXT("10 fps rounds down to the nearest frame"), State.SnapTime(0.34f), 0.3f);
	TestEqual(TEXT("10 fps rounds up to the nearest frame"), State.SnapTime(0.36f), 0.4f);

	Settings->SnapFrameRate = 4;
	TestEqual(TEXT("4 fps snaps to quarter seconds"), State.SnapTime(0.6f), 0.5f);

	Settings->SnapFrameRate = 60;
	TestEqual(TEXT("60 fps snaps to 1/60 s"), State.SnapTime(0.3475f), 0.35f);

	Settings->bSnapToFrames = false;
	TestEqual(TEXT("Snapping off leaves time unchanged"), State.SnapTime(0.3475f), 0.3475f);
	TestEqual(TEXT("Negative times clamp to zero"), State.SnapTime(-1.0f), 0.0f);

	Settings->bSnapToFrames = bOriginalSnap;
	Settings->SnapFrameRate = OriginalFrameRate;
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
