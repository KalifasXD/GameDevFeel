// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FFeelRecipeEditorState;
class SFeelTimelineTrackArea;

/**
 * Recipe timeline: a transport, snapping, comfort and
 * Play in PIE toolbar above the track area, with an intensity curve lane under the selected track.
 */
class SFeelTimeline : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelTimeline) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	/** Rebuilds the preview parameter sliders when the recipe's declared parameters change. */
	void RefreshParameterSliders();
	TSharedRef<SWidget> MakeAddTrackMenu();
	void OnStepClassPicked(UClass* StepClass);
	FText GetTimeText() const;
	TSharedRef<SWidget> MakePreviewComfortMenu();
	FText GetPreviewComfortText() const;
	// FEELKIT_PRO_BEGIN
	TSharedRef<SWidget> MakeRecentPlaysMenu();
	FText GetRecentPlaysText() const;
	// FEELKIT_PRO_END
	/** Toolbar above the tracks, built like Sequencer's: playback (play, stop, release, loop), snap, frame rate, comfort, recent plays, Play in PIE, capture, fit, and the time. */
	TSharedRef<SWidget> MakeToolbar();
	TSharedRef<SWidget> MakeFrameRateMenu();
	FText GetFrameRateText() const;

	TSharedPtr<FFeelRecipeEditorState> State;
	TSharedPtr<SFeelTimelineTrackArea> TrackArea;
	TSharedPtr<class SBox> ParameterSliderBox;

	/** Names and ranges of the parameters the sliders were built for. */
	FString ParameterSliderSignature;
	bool bParameterSlidersBuilt = false;
};
