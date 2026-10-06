// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/RichCurve.h"
#include "FeelComfortTypes.h"
#include "FeelFrameOutput.h"
#include "FeelPlayCapture.h"
#include "FeelTrackLifecycle.h"
#include "UObject/WeakObjectPtr.h"

class UFeelRecipe;
class USceneComponent;
class UWorld;
struct FFeelContext;
struct FFeelEvalParams;

/**
 * Shared state of one open recipe editor: selection, preview playback and undoable track edits.
 * The timeline, preview viewport and details panel all read from and write through this object.
 */
class FFeelRecipeEditorState : public TSharedFromThis<FFeelRecipeEditorState>
{
public:
	explicit FFeelRecipeEditorState(UFeelRecipe* InRecipe);
	~FFeelRecipeEditorState();

	UFeelRecipe* GetRecipe() const { return Recipe.Get(); }

	/**
	 * Whether this recipe must not be changed: a recipe of the FeelKit library, unless Allow Library Editing is on in the
	 * editor preferences. Every editing method does nothing while it is true.
	 */
	bool IsReadOnly() const;

	int32 GetSelectedTrack() const { return SelectedTrack; }
	void SetSelectedTrack(int32 TrackIndex);
	bool IsValidTrack(int32 TrackIndex) const;

	/** Broadcast when the selected track changes. */
	FSimpleMulticastDelegate OnSelectionChanged;

	/** Broadcast after tracks are added, removed or reordered, and after undo or redo. */
	FSimpleMulticastDelegate OnTracksChanged;

	/** World and component where side-effect steps (such as sounds) play during preview. Set by the preview viewport. */
	void SetPreviewScene(UWorld* InWorld, USceneComponent* InTargetComponent);

	/** Comfort applied to the preview. Unset means neutral: every effect at full strength. */
	void SetPreviewComfortPreset(TOptional<EFeelBuiltInComfortPreset> InPreset);
	TOptional<EFeelBuiltInComfortPreset> GetPreviewComfortPreset() const { return PreviewComfortPreset; }

	/** Copies the selected track to the track clipboard. */
	void CopySelectedTrack();

	/** Copies every track to the track clipboard. */
	void CopyAllTracks();

	bool CanPaste() const;

	/** Pastes clipboard tracks after the selection, starting at the playhead. One undo step. */
	void PasteTracks();

	/** Evaluation settings the preview uses: solo, preview seed, preview parameter values and the preview comfort preset. */
	FFeelEvalParams GetPreviewParams() const;

	/** Value the preview uses for a recipe parameter: the slider value, or the parameter's default. */
	float GetPreviewParameterValue(FName ParameterName) const;

	/** Sets a preview slider value and re-evaluates the preview. Does not modify the recipe. */
	void SetPreviewParameterValue(FName ParameterName, float Value);

	/** Puts every preview slider back to its parameter's default. */
	void ResetPreviewParameters();

	/**
	 * Moment capture: the preview replays a play recorded in game, with its seed, intensity, parameters, comfort and flash
	 * limiter decisions, so it shows the same frames the game showed. Edits to the recipe apply to the replay.
	 */
	void LoadCapture(const FFeelPlayCapture& Capture);

	/** Returns the preview to its own settings. */
	void ClearCapture();

	/** The play being replayed, if any. */
	const FFeelPlayCapture* GetActiveCapture() const { return ActiveCapture.GetPtrOrNull(); }

	/** Broadcast when a capture is loaded or cleared. */
	FSimpleMulticastDelegate OnCaptureChanged;

	/** Opens the recipe of a captured play in its editor and starts replaying the play there. */
	static void OpenCapture(const FFeelPlayCapture& Capture);

	/** Makes an editor state findable by OpenCapture. Called by the recipe editor after creating its state. */
	static void Register(const TSharedRef<FFeelRecipeEditorState>& State);

	/** The state of an open recipe editor for Recipe, if any. */
	static TSharedPtr<FFeelRecipeEditorState> FindOpenState(const UFeelRecipe* Recipe);

	/**
	 * What happens to a track in the current preview play, when it differs from simply playing: muted, not soloed, skipped
	 * by a condition, removed or substituted by comfort, softened or suppressed by the flash limiter, or the option a
	 * random choice picked. Empty when the track plays normally. bOutSilenced says whether the track produces no output.
	 */
	FText GetTrackDecision(int32 TrackIndex, bool* bOutSilenced = nullptr) const;

	/** While bypassed the preview shows no FeelKit output, for Off/On comparisons. */
	void SetOutputBypassed(bool bBypassed);

	/** Asks the preview viewport to render the recipe Off and On side by side into an animated GIF. */
	FSimpleMulticastDelegate OnCaptureGifRequested;

	// FEELKIT_PRO_BEGIN
	/**
	 * Adds a track built from a Play Sound track's audio: its loudness over time becomes the new track's intensity curve.
	 * bForceFeedback creates a Force Feedback Curve track, otherwise a Procedural Shake track. One undo step.
	 */
	bool CreateTrackFromSound(int32 SoundTrackIndex, bool bForceFeedback);
	// FEELKIT_PRO_END

	/** The track's own intensity curve, or null when the track is invalid or uses a curve asset. */
	FRichCurve* GetEditableCurve(int32 TrackIndex) const;

	/** Adds a key, with time and value clamped to 0 to 1. One undo step. */
	bool AddCurveKey(int32 TrackIndex, float Time, float Value);

	/** Deletes a key. One undo step. */
	bool DeleteCurveKey(int32 TrackIndex, FKeyHandle Key);

	/** Changes how the curve moves from this key to the next. One undo step. */
	bool SetCurveKeyInterpMode(int32 TrackIndex, FKeyHandle Key, ERichCurveInterpMode InterpMode);

	/** Moves a key between its neighbors, value clamped to 0 to 1. Records no undo step itself: wrap drags in one transaction. */
	void MoveCurveKey(int32 TrackIndex, FKeyHandle Key, float Time, float Value);

	/** Whether a Play-In-Editor session is running. */
	bool CanPlayInPIE() const;

	/** Plays the recipe on the running session's player pawn, on its skeletal mesh when it has one. */
	void PlayInPIE();

	/** Advances playback by real (unscaled) seconds, runs track starts and stops, and re-evaluates the preview output. */
	void Tick(float DeltaRealSeconds);

	void TogglePlay();
	void Stop();

	/** Whether the preview is looping a sustain region that can be released. */
	bool CanReleaseSustain() const;

	/**
	 * Ends the sustain loop, like Release Feel at runtime: the preview plays the rest of the recipe, jumping straight to
	 * Sustain End when the recipe has Jump to End on Release.
	 */
	void ReleaseSustain();
	bool IsPlaying() const { return bPlaying; }
	bool IsLooping() const { return bLooping; }
	void SetLooping(bool bInLooping) { bLooping = bInLooping; }

	/** Current playhead time in seconds. */
	float GetTime() const { return Time; }

	/** Moves the playhead (scrub). */
	void SetTime(float InTime);

	float GetPlaybackLength() const;

	/** Combined output at the playhead. Neutral while stopped. */
	const FFeelFrameOutput& GetOutput() const { return Output; }

	/** Rounds to the nearest frame when snapping is enabled in the editor settings. */
	float SnapTime(float InTime) const;

	void AddTrack(UClass* StepClass);
	void DeleteTrack(int32 TrackIndex);
	void DuplicateTrack(int32 TrackIndex);
	void MoveTrack(int32 TrackIndex, int32 Direction);
	void ToggleMute(int32 TrackIndex);
	void ToggleSolo(int32 TrackIndex);

	/** Re-validates selection and notifies listeners after changes made elsewhere (undo, redo). */
	void HandleExternalChange();

private:
	void EvaluateOutput();
	void UpdateTrackLifecycle();
	void StopTrackLifecycle(bool bInterrupted);
	void RestartTrackLifecycle(float FromTime);
	FFeelContext MakePreviewContext(int32 TrackIndex, float TrackIntensity) const;

	TWeakObjectPtr<UFeelRecipe> Recipe;
	int32 SelectedTrack = INDEX_NONE;
	float Time = 0.0f;
	bool bPlaying = false;
	bool bLooping = false;
	bool bStopped = true;
	bool bSustainReleased = false;

	/** Whether the recipe's Release Parameter released the preview (rather than the Release button). */
	bool bReleaseReached = false;
	FFeelFrameOutput Output;

	FFeelTrackLifecycle Lifecycle;
	TWeakObjectPtr<UWorld> PreviewWorld;
	TWeakObjectPtr<USceneComponent> PreviewTarget;

	TOptional<EFeelBuiltInComfortPreset> PreviewComfortPreset;

	/** Instance seed of the current preview play. Rolled again on each play from the start and each loop, like PlayFeel. */
	int32 PreviewSeed = 0;

	/** Preview slider values by parameter name. Missing names use the parameter default. */
	TMap<FName, float> PreviewParameterValues;

	/** Flash limiter of the preview, using the preview comfort preset. */
	void ApplyPreviewFlashLimiter();

	TOptional<FFeelPlayCapture> ActiveCapture;
	TArray<float> PreviewTrackScales;
	FFeelFlashLimiter PreviewFlashLimiter;
	double PreviewClock = 0.0;
	float LastTickSeconds = 0.0f;
	float PreviewPreviousFieldOfView = 0.0f;
	bool bOutputBypassed = false;

	/** Scales of the preview preset, refreshed from the project settings on every evaluation. */
	mutable FFeelComfortScales PreviewComfortScales;
};
