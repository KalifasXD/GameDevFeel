// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSceneViewExtension.h"

#include "FeelFrameOutput.h"
#include "FeelSubsystem.h"
#include "SceneView.h"

FFeelSceneViewExtension::FFeelSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld, UFeelSubsystem* InSubsystem)
	: FWorldSceneViewExtension(AutoRegister, InWorld)
	, Subsystem(InSubsystem)
{
}

void FFeelSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (const UFeelSubsystem* PinnedSubsystem = Subsystem.Get())
	{
		PinnedSubsystem->GetScreenOutput().ApplyOverlay(InView.OverlayColor);
	}
}
