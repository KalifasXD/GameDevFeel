// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "UObject/WeakObjectPtr.h"

class UFeelSubsystem;

/**
 * Layers FeelKit screen flashes over the views of one game world.
 * Uses the same FFeelFrameOutput::ApplyOverlay as the editor preview, on top of any camera fade.
 */
class FFeelSceneViewExtension : public FWorldSceneViewExtension
{
public:
	FFeelSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld, UFeelSubsystem* InSubsystem);

	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;

private:
	TWeakObjectPtr<UFeelSubsystem> Subsystem;
};
