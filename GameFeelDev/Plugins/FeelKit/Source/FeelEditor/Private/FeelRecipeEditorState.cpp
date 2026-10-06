// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeEditorState.h"

#include "FeelLibrary.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Editor.h"
// FEELKIT_PRO_BEGIN
#include "FeelAudioAnalysis.h"
// FEELKIT_PRO_END
#include "HAL/PlatformProperties.h"
#include "Steps/FeelStep_ForceFeedbackCurve.h"
#include "Steps/FeelStep_Meta.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Engine/World.h"
#include "FeelSubsystem.h"
#include "FeelTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "FeelEditorSettings.h"
#include "FeelEvaluator.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelStep.h"
#include "FeelTrackClipboard.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

namespace FeelRecipeEditorStatePrivate
{
	/** Instance id for preview contexts. Together with the preview world it never collides with runtime instances. */
	constexpr int32 PreviewInstanceId = -1;
}

FFeelRecipeEditorState::FFeelRecipeEditorState(UFeelRecipe* InRecipe)
	: Recipe(InRecipe)
{
}

FFeelRecipeEditorState::~FFeelRecipeEditorState()
{
	StopTrackLifecycle(true);
}

void FFeelRecipeEditorState::SetSelectedTrack(int32 TrackIndex)
{
	const int32 NewSelection = IsValidTrack(TrackIndex) ? TrackIndex : INDEX_NONE;
	if (NewSelection != SelectedTrack)
	{
		SelectedTrack = NewSelection;
		OnSelectionChanged.Broadcast();
	}
}

bool FFeelRecipeEditorState::IsReadOnly() const
{
	return FeelLibrary::IsReadOnly(Recipe.Get());
}

bool FFeelRecipeEditorState::IsValidTrack(int32 TrackIndex) const
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	return CurrentRecipe && CurrentRecipe->Tracks.IsValidIndex(TrackIndex);
}

void FFeelRecipeEditorState::SetPreviewScene(UWorld* InWorld, USceneComponent* InTargetComponent)
{
	PreviewWorld = InWorld;
	PreviewTarget = InTargetComponent;
}

void FFeelRecipeEditorState::Tick(float DeltaRealSeconds)
{
	PreviewClock += DeltaRealSeconds;
	LastTickSeconds = DeltaRealSeconds;

	if (bPlaying)
	{
		const float Length = GetPlaybackLength();
		const UFeelRecipe* CurrentRecipe = GetRecipe();

		// Release Parameter, checked before the time step as at runtime, with the preview sliders' values. A replayed play
		// is released with the Release button only: its parameter values are the ones it ended with.
		if (CurrentRecipe && !bSustainReleased && !ActiveCapture.IsSet() && FFeelEvaluator::IsReleaseParameterReached(*CurrentRecipe, GetPreviewParams()))
		{
			bReleaseReached = true;
			ReleaseSustain();
		}

		Time += DeltaRealSeconds;

		// Sustain region: finish it, rewind its tracks and loop, until released. Same clock rule as the runtime.
		float WrappedTime = Time;
		if (CurrentRecipe && !bSustainReleased && FFeelPlaybackClock::WrapIntoSustain(*CurrentRecipe, WrappedTime))
		{
			Time = CurrentRecipe->SustainEnd;
			Lifecycle.WrapSustain(*CurrentRecipe, GetPreviewParams(), [this](int32 TrackIndex, float TrackIntensity)
			{
				return MakePreviewContext(TrackIndex, TrackIntensity);
			});
			ApplyPreviewFlashLimiter();
			Time = WrappedTime;
		}

		if (Time > Length)
		{
			// Let every track end normally before wrapping or stopping.
			UpdateTrackLifecycle();
			StopTrackLifecycle(false);

			if (bLooping && Length > 0.0f)
			{
				// Each loop is a new play, like PlayFeel: new seed, new chance rolls, sustain loops again. A replayed capture
				// keeps its own seed.
				if (!ActiveCapture.IsSet())
				{
					PreviewSeed = FMath::Rand();
				}
				bSustainReleased = false;
				bReleaseReached = false;
				Time = FMath::Fmod(Time, Length);
				RestartTrackLifecycle(-1.0f);
			}
			else
			{
				Time = Length;
				bPlaying = false;
			}
		}

		if (bPlaying)
		{
			UpdateTrackLifecycle();
		}
	}
	EvaluateOutput();
}

void FFeelRecipeEditorState::TogglePlay()
{
	if (bPlaying)
	{
		bPlaying = false;
		StopTrackLifecycle(true);
		return;
	}

	if (bStopped || Time >= GetPlaybackLength())
	{
		Time = 0.0f;
		if (!ActiveCapture.IsSet())
		{
			PreviewSeed = FMath::Rand();
		}
		bSustainReleased = false;
		bReleaseReached = false;
	}
	RestartTrackLifecycle(Time > 0.0f ? Time : -1.0f);
	bStopped = false;
	bPlaying = true;
}

void FFeelRecipeEditorState::Stop()
{
	StopTrackLifecycle(true);
	bPlaying = false;
	bStopped = true;
	bSustainReleased = false;
	bReleaseReached = false;
	Time = 0.0f;
	EvaluateOutput();
}

bool FFeelRecipeEditorState::CanReleaseSustain() const
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	return bPlaying && !bSustainReleased && CurrentRecipe && FFeelPlaybackClock::HasSustain(*CurrentRecipe);
}

void FFeelRecipeEditorState::ReleaseSustain()
{
	bSustainReleased = true;

	// Jump to End on Release or a release recipe: the same jump as the runtime clock, and the lifecycle skips the rest of
	// the loop.
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	float JumpTime = Time;
	if (CurrentRecipe && FFeelPlaybackClock::GetReleaseJumpTime(*CurrentRecipe, Time, JumpTime))
	{
		Time = JumpTime;
		Lifecycle.JumpForward(*CurrentRecipe, Time);
	}
}

void FFeelRecipeEditorState::SetTime(float InTime)
{
	bStopped = false;
	Time = FMath::Clamp(InTime, 0.0f, GetPlaybackLength());
	if (bPlaying)
	{
		StopTrackLifecycle(true);
		RestartTrackLifecycle(Time);
	}
	EvaluateOutput();
}

float FFeelRecipeEditorState::GetPlaybackLength() const
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	return CurrentRecipe ? FFeelEvaluator::GetRecipeDuration(*CurrentRecipe, GetPreviewParams()) : 0.0f;
}

float FFeelRecipeEditorState::SnapTime(float InTime) const
{
	const UFeelEditorSettings* Settings = GetDefault<UFeelEditorSettings>();
	if (Settings->bSnapToFrames && Settings->SnapFrameRate > 0)
	{
		InTime = FMath::GridSnap(InTime, 1.0f / static_cast<float>(Settings->SnapFrameRate));
	}
	return FMath::Max(InTime, 0.0f);
}

void FFeelRecipeEditorState::AddTrack(UClass* StepClass)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || !StepClass || !StepClass->IsChildOf(UFeelStep::StaticClass()) || StepClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddTrack", "Add Feel Track"));
	CurrentRecipe->Modify();

	UFeelStep* Step = NewObject<UFeelStep>(CurrentRecipe, StepClass, NAME_None, RF_Transactional);

	FFeelTrack NewTrack;
	NewTrack.Step = Step;
	NewTrack.Channel = Step->GetDefaultChannel();
	NewTrack.StartTime = bStopped ? 0.0f : SnapTime(Time);
	if (Step->UsesConstantIntensityByDefault())
	{
		FRichCurve* Curve = NewTrack.IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->SetKeyInterpMode(Curve->AddKey(0.0f, 1.0f), RCIM_Linear);
		Curve->SetKeyInterpMode(Curve->AddKey(1.0f, 1.0f), RCIM_Linear);
	}

	const int32 NewIndex = CurrentRecipe->Tracks.Add(NewTrack);
	OnTracksChanged.Broadcast();
	SetSelectedTrack(NewIndex);
}

void FFeelRecipeEditorState::DeleteTrack(int32 TrackIndex)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!IsValidTrack(TrackIndex))
	{
		return;
	}

	StopTrackLifecycle(true);

	const FScopedTransaction Transaction(LOCTEXT("DeleteTrack", "Delete Feel Track"));
	CurrentRecipe->Modify();
	CurrentRecipe->Tracks.RemoveAt(TrackIndex);

	if (SelectedTrack == TrackIndex)
	{
		SelectedTrack = INDEX_NONE;
	}
	else if (SelectedTrack > TrackIndex)
	{
		--SelectedTrack;
	}

	if (bPlaying)
	{
		RestartTrackLifecycle(Time);
	}
	OnTracksChanged.Broadcast();
	OnSelectionChanged.Broadcast();
}

void FFeelRecipeEditorState::DuplicateTrack(int32 TrackIndex)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!IsValidTrack(TrackIndex))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DuplicateTrack", "Duplicate Feel Track"));
	CurrentRecipe->Modify();

	FFeelTrack Copy = CurrentRecipe->Tracks[TrackIndex];
	if (Copy.Step)
	{
		Copy.Step = DuplicateObject<UFeelStep>(Copy.Step.Get(), CurrentRecipe);
		Copy.Step->SetFlags(RF_Transactional);
	}
	if (Copy.SubstituteStep)
	{
		Copy.SubstituteStep = DuplicateObject<UFeelStep>(Copy.SubstituteStep.Get(), CurrentRecipe);
		Copy.SubstituteStep->SetFlags(RF_Transactional);
	}

	const int32 NewIndex = TrackIndex + 1;
	CurrentRecipe->Tracks.Insert(Copy, NewIndex);
	OnTracksChanged.Broadcast();
	SelectedTrack = INDEX_NONE;
	SetSelectedTrack(NewIndex);
}

void FFeelRecipeEditorState::MoveTrack(int32 TrackIndex, int32 Direction)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	const int32 TargetIndex = TrackIndex + Direction;
	if (!IsValidTrack(TrackIndex) || !IsValidTrack(TargetIndex))
	{
		return;
	}

	StopTrackLifecycle(true);

	const FScopedTransaction Transaction(LOCTEXT("MoveTrack", "Reorder Feel Track"));
	CurrentRecipe->Modify();
	CurrentRecipe->Tracks.Swap(TrackIndex, TargetIndex);

	if (SelectedTrack == TrackIndex)
	{
		SelectedTrack = TargetIndex;
	}
	else if (SelectedTrack == TargetIndex)
	{
		SelectedTrack = TrackIndex;
	}

	if (bPlaying)
	{
		RestartTrackLifecycle(Time);
	}
	OnTracksChanged.Broadcast();
	OnSelectionChanged.Broadcast();
}

void FFeelRecipeEditorState::ToggleMute(int32 TrackIndex)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!IsValidTrack(TrackIndex))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("ToggleMute", "Toggle Feel Track Mute"));
	CurrentRecipe->Modify();
	CurrentRecipe->Tracks[TrackIndex].bEnabled = !CurrentRecipe->Tracks[TrackIndex].bEnabled;
	OnTracksChanged.Broadcast();
}

void FFeelRecipeEditorState::ToggleSolo(int32 TrackIndex)
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!IsValidTrack(TrackIndex))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("ToggleSolo", "Toggle Feel Track Solo"));
	CurrentRecipe->Modify();
#if WITH_EDITORONLY_DATA
	CurrentRecipe->Tracks[TrackIndex].bSolo = !CurrentRecipe->Tracks[TrackIndex].bSolo;
#endif
}

void FFeelRecipeEditorState::HandleExternalChange()
{
	if (!IsValidTrack(SelectedTrack))
	{
		SelectedTrack = INDEX_NONE;
	}

	if (bPlaying)
	{
		StopTrackLifecycle(true);
		RestartTrackLifecycle(Time);
	}
	OnTracksChanged.Broadcast();
	OnSelectionChanged.Broadcast();
}

void FFeelRecipeEditorState::EvaluateOutput()
{
	Output.Reset();

	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || bStopped)
	{
		return;
	}

	FFeelOutputAccumulator Accumulator;
	const FFeelEvalParams Params = GetOutputParams();
	FFeelEvaluator::Evaluate(*CurrentRecipe, Time, Params, Accumulator);
	Output = Accumulator.Output;

	// Motion comfort, as the runtime camera modifier applies it. Scrubbing jumps without a rate limit.
	if (Params.Comfort.Scales)
	{
		FeelComfort::ApplyMotionComfort(*Params.Comfort.Scales, Output.CameraRotationOffset, Output.FieldOfViewOffset, PreviewPreviousFieldOfView, bPlaying ? LastTickSeconds : 0.0f);
	}

	if (bOutputBypassed)
	{
		Output.Reset();
	}
}

void FFeelRecipeEditorState::SetOutputBypassed(bool bBypassed)
{
	bOutputBypassed = bBypassed;
	EvaluateOutput();
}

void FFeelRecipeEditorState::ApplyPreviewFlashLimiter()
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || ActiveCapture.IsSet())
	{
		return;
	}

	if (PreviewTrackScales.Num() != CurrentRecipe->Tracks.Num())
	{
		PreviewTrackScales.Init(1.0f, CurrentRecipe->Tracks.Num());
	}

	const FFeelEvalParams Params = GetPreviewParams();
	if (!Params.Comfort.Scales)
	{
		return;
	}

	for (int32 StartedTrack : Lifecycle.GetStartedThisUpdate())
	{
		if (Params.Comfort.GetGroup(CurrentRecipe->Tracks[StartedTrack].Channel) == EFeelComfortGroup::Flashes)
		{
			PreviewTrackScales[StartedTrack] = PreviewFlashLimiter.RegisterFlash(PreviewClock, *Params.Comfort.Scales);
		}
	}

	// The release recipe's flashes too, as at runtime.
	if (const UFeelRecipe* ReleaseRecipe = Lifecycle.GetStartedReleaseRecipe())
	{
		if (PreviewReleaseTrackScales.Num() != ReleaseRecipe->Tracks.Num())
		{
			PreviewReleaseTrackScales.Init(1.0f, ReleaseRecipe->Tracks.Num());
		}
		for (int32 StartedTrack : Lifecycle.GetReleaseStartedThisUpdate())
		{
			if (Params.Comfort.GetGroup(ReleaseRecipe->Tracks[StartedTrack].Channel) == EFeelComfortGroup::Flashes)
			{
				PreviewReleaseTrackScales[StartedTrack] = PreviewFlashLimiter.RegisterFlash(PreviewClock, *Params.Comfort.Scales);
			}
		}
	}
}

void FFeelRecipeEditorState::UpdateTrackLifecycle()
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe)
	{
		return;
	}

	const FFeelEvalParams Params = GetPreviewParams();
	Lifecycle.Update(*CurrentRecipe, Time, Params, [this](int32 TrackIndex, float TrackIntensity)
	{
		return MakePreviewContext(TrackIndex, TrackIntensity);
	});
	ApplyPreviewFlashLimiter();
}

void FFeelRecipeEditorState::StopTrackLifecycle(bool bInterrupted)
{
	Lifecycle.StopAll(GetRecipe(), bInterrupted, [this](int32 TrackIndex, float TrackIntensity)
	{
		return MakePreviewContext(TrackIndex, TrackIntensity);
	});
}

void FFeelRecipeEditorState::RestartTrackLifecycle(float FromTime)
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	Lifecycle.Reset(CurrentRecipe ? CurrentRecipe->Tracks.Num() : 0, FromTime);
	PreviewTrackScales.Init(1.0f, CurrentRecipe ? CurrentRecipe->Tracks.Num() : 0);
	PreviewReleaseTrackScales.Reset();
}

FFeelContext FFeelRecipeEditorState::MakePreviewContext(int32 TrackIndex, float TrackIntensity) const
{
	UFeelRecipe* CurrentRecipe = GetRecipe();

	FFeelContext Context;
	Context.World = PreviewWorld.Get();
	Context.TargetComponent = PreviewTarget.Get();
	Context.TargetLocation = Context.TargetComponent ? Context.TargetComponent->GetComponentLocation() : FVector::ZeroVector;
	Context.Recipe = CurrentRecipe;
	Context.ElapsedRealTime = Time;
	Context.Intensity = TrackIntensity;
	Context.InstanceId = FeelRecipeEditorStatePrivate::PreviewInstanceId;
	Context.TrackIndex = TrackIndex;
	Context.TrackDuration = CurrentRecipe ? FFeelEvaluator::GetTrackDuration(*CurrentRecipe, TrackIndex, GetPreviewParams()) : 0.0f;
	Context.Instigator = Context.Target;
	Context.PlayContext.Instigator = Context.Target;
	Context.PlayContext.Parameters = ActiveCapture.IsSet() ? ActiveCapture->ParameterValues : PreviewParameterValues;
	return Context;
}

void FFeelRecipeEditorState::SetPreviewComfortPreset(TOptional<EFeelBuiltInComfortPreset> InPreset)
{
	PreviewComfortPreset = InPreset;
	EvaluateOutput();
}

void FFeelRecipeEditorState::CopySelectedTrack()
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (CurrentRecipe && IsValidTrack(SelectedTrack))
	{
		FFeelTrackClipboard::Get().Copy(*CurrentRecipe, MakeArrayView(&SelectedTrack, 1));
	}
}

void FFeelRecipeEditorState::CopyAllTracks()
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || CurrentRecipe->Tracks.Num() == 0)
	{
		return;
	}

	TArray<int32> TrackIndices;
	for (int32 TrackIndex = 0; TrackIndex < CurrentRecipe->Tracks.Num(); ++TrackIndex)
	{
		TrackIndices.Add(TrackIndex);
	}
	FFeelTrackClipboard::Get().Copy(*CurrentRecipe, TrackIndices);
}

bool FFeelRecipeEditorState::CanPaste() const
{
	if (IsReadOnly())
	{
		return false;
	}

	return GetRecipe() && FFeelTrackClipboard::Get().HasTracks();
}

void FFeelRecipeEditorState::PasteTracks()
{
	if (IsReadOnly())
	{
		return;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	const FFeelTrackClipboard& Clipboard = FFeelTrackClipboard::Get();
	if (!CurrentRecipe || !Clipboard.HasTracks())
	{
		return;
	}

	StopTrackLifecycle(true);

	const FScopedTransaction Transaction(LOCTEXT("PasteTracks", "Paste Feel Tracks"));
	CurrentRecipe->Modify();

	const int32 InsertIndex = IsValidTrack(SelectedTrack) ? SelectedTrack + 1 : CurrentRecipe->Tracks.Num();
	const int32 FirstPasted = Clipboard.PasteInto(*CurrentRecipe, InsertIndex, bStopped ? 0.0f : SnapTime(Time));

	if (bPlaying)
	{
		RestartTrackLifecycle(Time);
	}
	OnTracksChanged.Broadcast();
	SelectedTrack = INDEX_NONE;
	SetSelectedTrack(FirstPasted);
}

FRichCurve* FFeelRecipeEditorState::GetEditableCurve(int32 TrackIndex) const
{
	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || !CurrentRecipe->Tracks.IsValidIndex(TrackIndex))
	{
		return nullptr;
	}

	FFeelTrack& Track = CurrentRecipe->Tracks[TrackIndex];
	return Track.IntensityCurve.ExternalCurve ? nullptr : Track.IntensityCurve.GetRichCurve();
}

bool FFeelRecipeEditorState::AddCurveKey(int32 TrackIndex, float KeyTime, float Value)
{
	if (IsReadOnly())
	{
		return false;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!GetEditableCurve(TrackIndex))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddCurveKey", "Add Intensity Key"));
	CurrentRecipe->Modify();

	FRichCurve* Curve = GetEditableCurve(TrackIndex);
	const FKeyHandle Key = Curve->AddKey(FMath::Clamp(KeyTime, 0.0f, 1.0f), FMath::Clamp(Value, 0.0f, 1.0f));
	Curve->SetKeyInterpMode(Key, RCIM_Linear);
	Curve->AutoSetTangents();
	return true;
}

bool FFeelRecipeEditorState::DeleteCurveKey(int32 TrackIndex, FKeyHandle Key)
{
	if (IsReadOnly())
	{
		return false;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	const FRichCurve* ExistingCurve = GetEditableCurve(TrackIndex);
	if (!ExistingCurve || !ExistingCurve->IsKeyHandleValid(Key))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteCurveKey", "Delete Intensity Key"));
	CurrentRecipe->Modify();

	FRichCurve* Curve = GetEditableCurve(TrackIndex);
	Curve->DeleteKey(Key);
	Curve->AutoSetTangents();
	return true;
}

bool FFeelRecipeEditorState::SetCurveKeyInterpMode(int32 TrackIndex, FKeyHandle Key, ERichCurveInterpMode InterpMode)
{
	if (IsReadOnly())
	{
		return false;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	const FRichCurve* ExistingCurve = GetEditableCurve(TrackIndex);
	if (!ExistingCurve || !ExistingCurve->IsKeyHandleValid(Key))
	{
		return false;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetCurveKeyInterp", "Change Intensity Key Interpolation"));
	CurrentRecipe->Modify();

	FRichCurve* Curve = GetEditableCurve(TrackIndex);
	Curve->SetKeyInterpMode(Key, InterpMode);
	Curve->AutoSetTangents();
	return true;
}

void FFeelRecipeEditorState::MoveCurveKey(int32 TrackIndex, FKeyHandle Key, float KeyTime, float Value)
{
	if (IsReadOnly())
	{
		return;
	}

	FRichCurve* Curve = GetEditableCurve(TrackIndex);
	if (!Curve || !Curve->IsKeyHandleValid(Key))
	{
		return;
	}

	// Keep the key between its neighbors so key order never changes while dragging.
	constexpr float MinKeySpacing = 0.001f;
	const float OldTime = Curve->GetKeyTime(Key);
	float LowerBound = 0.0f;
	float UpperBound = 1.0f;
	for (auto It = Curve->GetKeyHandleIterator(); It; ++It)
	{
		const FKeyHandle Other = *It;
		if (Other == Key)
		{
			continue;
		}

		const float OtherTime = Curve->GetKeyTime(Other);
		if (OtherTime <= OldTime)
		{
			LowerBound = FMath::Max(LowerBound, OtherTime + MinKeySpacing);
		}
		else
		{
			UpperBound = FMath::Min(UpperBound, OtherTime - MinKeySpacing);
		}
	}

	Curve->SetKeyTime(Key, FMath::Clamp(KeyTime, FMath::Min(LowerBound, UpperBound), FMath::Max(LowerBound, UpperBound)));
	Curve->SetKeyValue(Key, FMath::Clamp(Value, 0.0f, 1.0f));
	Curve->AutoSetTangents();
}

bool FFeelRecipeEditorState::CanPlayInPIE() const
{
	return GetRecipe() && GEditor && GEditor->PlayWorld;
}

void FFeelRecipeEditorState::PlayInPIE()
{
	UWorld* PlayWorld = GEditor ? GEditor->PlayWorld.Get() : nullptr;
	UFeelSubsystem* Subsystem = PlayWorld ? PlayWorld->GetSubsystem<UFeelSubsystem>() : nullptr;
	UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!Subsystem || !CurrentRecipe)
	{
		return;
	}

	FFeelTarget Target = FFeelTarget::FromLocalPlayerCamera(0);
	const APlayerController* PlayerController = PlayWorld->GetFirstPlayerController();
	if (APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr)
	{
		// Characters: actor effects go on the mesh, not the collision capsule.
		USkeletalMeshComponent* Mesh = Pawn->FindComponentByClass<USkeletalMeshComponent>();
		Target = Mesh ? FFeelTarget::FromComponent(Mesh) : FFeelTarget::FromActor(Pawn);
	}
	Subsystem->PlayFeel(CurrentRecipe, Target, 1.0f);
}

FFeelEvalParams FFeelRecipeEditorState::GetPreviewParams() const
{
	FFeelEvalParams Params;
	Params.bRespectSolo = true;
	Params.InstanceSeed = PreviewSeed;

	// The preview target is the player's own view: distance unknown and local player conditions pass.
	Params.TargetDistance = -1.0f;
	Params.bTargetIsLocalPlayer = true;

	// Preview parameter sliders. Tracks for the instigator play on the preview mesh too, so every track is visible.
	Params.ParameterValues = &PreviewParameterValues;
	Params.bHasInstigator = true;
	Params.TrackScales = PreviewTrackScales;

	// Release outcome, which picks the release recipe, as at runtime.
	Params.bReleased = bSustainReleased;
	Params.bReleaseReached = bSustainReleased && bReleaseReached;
	Params.ReleaseTrackScales = PreviewReleaseTrackScales;

	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	if (ActiveCapture.IsSet())
	{
		// Replay: every input of the recorded play.
		const FFeelPlayCapture& Capture = ActiveCapture.GetValue();
		Params.InstanceSeed = Capture.Seed;
		Params.Intensity = Capture.Intensity;
		Params.ParameterValues = &Capture.ParameterValues;
		Params.TargetDistance = Capture.TargetDistance;
		Params.bTargetIsLocalPlayer = Capture.bTargetIsLocalPlayer;
		Params.bHasInstigator = Capture.bHadInstigator;
		Params.ViewDirection = Capture.ViewDirection;
		Params.ViewDirectionFromLocation = Capture.ViewDirectionFromLocation;
		Params.TrackScales = Capture.TrackScales;
		Params.ReleaseTrackScales = Capture.ReleaseTrackScales;
		// Released with the Release button, a replay ends the way the recorded play did.
		Params.bReleaseReached = bSustainReleased && Capture.bReleaseReached;
		if (Capture.bHasComfort)
		{
			PreviewComfortScales = Capture.ComfortScales;
			Params.Comfort.Scales = &PreviewComfortScales;
			Params.Comfort.Mappings = Settings->ChannelComfortGroups;
		}
		return Params;
	}

	if (PreviewComfortPreset.IsSet())
	{
		PreviewComfortScales = Settings->GetPresetScales(PreviewComfortPreset.GetValue());
		Params.Comfort.Scales = &PreviewComfortScales;
		Params.Comfort.Mappings = Settings->ChannelComfortGroups;
	}
	return Params;
}

FFeelEvalParams FFeelRecipeEditorState::GetOutputParams() const
{
	FFeelEvalParams Params = GetPreviewParams();
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (CurrentRecipe && !bPlaying && !bSustainReleased && FFeelPlaybackClock::HasSustain(*CurrentRecipe) && Time >= CurrentRecipe->SustainEnd)
	{
		Params.bReleased = true;
		Params.bReleaseReached = ActiveCapture.IsSet() ? ActiveCapture->bReleaseReached : FFeelEvaluator::IsReleaseParameterReached(*CurrentRecipe, Params);
	}
	return Params;
}

float FFeelRecipeEditorState::GetPreviewParameterValue(FName ParameterName) const
{
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	// GetPreviewParams uses the replayed values while a capture is active.
	const FFeelRecipeParameter* Parameter = CurrentRecipe ? CurrentRecipe->FindParameter(ParameterName) : nullptr;
	if (!Parameter)
	{
		return 0.0f;
	}
	return FFeelEvaluator::GetParameterValue(*Parameter, GetPreviewParams());
}

void FFeelRecipeEditorState::SetPreviewParameterValue(FName ParameterName, float Value)
{
	// While replaying, the slider tweaks the replayed play.
	if (ActiveCapture.IsSet())
	{
		ActiveCapture->ParameterValues.Add(ParameterName, Value);
	}
	else
	{
		PreviewParameterValues.Add(ParameterName, Value);
	}
	EvaluateOutput();
}

namespace FeelRecipeEditorStatePrivate
{
	TArray<TWeakPtr<FFeelRecipeEditorState>>& GetLiveStates()
	{
		static TArray<TWeakPtr<FFeelRecipeEditorState>> States;
		return States;
	}
}

void FFeelRecipeEditorState::Register(const TSharedRef<FFeelRecipeEditorState>& State)
{
	TArray<TWeakPtr<FFeelRecipeEditorState>>& States = FeelRecipeEditorStatePrivate::GetLiveStates();
	States.RemoveAll([](const TWeakPtr<FFeelRecipeEditorState>& Existing) { return !Existing.IsValid(); });
	States.Add(State);
}

TSharedPtr<FFeelRecipeEditorState> FFeelRecipeEditorState::FindOpenState(const UFeelRecipe* Recipe)
{
	for (const TWeakPtr<FFeelRecipeEditorState>& WeakState : FeelRecipeEditorStatePrivate::GetLiveStates())
	{
		TSharedPtr<FFeelRecipeEditorState> State = WeakState.Pin();
		if (State.IsValid() && Recipe && State->GetRecipe() == Recipe)
		{
			return State;
		}
	}
	return nullptr;
}

void FFeelRecipeEditorState::OpenCapture(const FFeelPlayCapture& Capture)
{
	UFeelRecipe* CapturedRecipe = Capture.Recipe.Get();
	if (!CapturedRecipe || !GEditor)
	{
		return;
	}

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(CapturedRecipe);
	for (const TWeakPtr<FFeelRecipeEditorState>& WeakState : FeelRecipeEditorStatePrivate::GetLiveStates())
	{
		const TSharedPtr<FFeelRecipeEditorState> State = WeakState.Pin();
		if (State.IsValid() && State->GetRecipe() == CapturedRecipe)
		{
			State->LoadCapture(Capture);
			State->TogglePlay();
		}
	}
}

void FFeelRecipeEditorState::LoadCapture(const FFeelPlayCapture& Capture)
{
	Stop();
	ActiveCapture = Capture;
	OnCaptureChanged.Broadcast();
	SetTime(0.0f);
}

void FFeelRecipeEditorState::ClearCapture()
{
	if (ActiveCapture.IsSet())
	{
		Stop();
		ActiveCapture.Reset();
		OnCaptureChanged.Broadcast();
	}
}

FText FFeelRecipeEditorState::GetReleaseDecision(bool bFullRelease, bool* bOutSilenced) const
{
	bool bSilencedStorage = false;
	bool& bSilenced = bOutSilenced ? *bOutSilenced : bSilencedStorage;
	bSilenced = true;
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	const UFeelRecipe* ReleaseRecipe = CurrentRecipe ? (bFullRelease ? CurrentRecipe->FullReleaseRecipe.Get() : CurrentRecipe->EarlyReleaseRecipe.Get()) : nullptr;
	if (!ReleaseRecipe)
	{
		return FText::GetEmpty();
	}
	if (ReleaseRecipe == CurrentRecipe)
	{
		return LOCTEXT("ReleaseDecisionSelf", "Skipped: this is the recipe itself");
	}
	if (!FFeelPlaybackClock::HasSustain(*CurrentRecipe))
	{
		return LOCTEXT("ReleaseDecisionNoSustain", "Never plays: no sustain region");
	}

	const bool bHasParameter = CurrentRecipe->FindParameter(CurrentRecipe->ReleaseParameter) != nullptr;
	if (bFullRelease && !bHasParameter)
	{
		return LOCTEXT("ReleaseDecisionNoParameter", "Never plays: no Release Parameter");
	}

	const FText Parameter = FText::FromName(CurrentRecipe->ReleaseParameter);
	const FFeelEvalParams Params = GetOutputParams();
	if (!Params.bReleased)
	{
		bSilenced = false;
		if (bFullRelease)
		{
			return FText::Format(LOCTEXT("ReleaseDecisionWaitsFull", "Plays if {0} reaches Release At"), Parameter);
		}
		return bHasParameter
			? FText::Format(LOCTEXT("ReleaseDecisionWaitsEarly", "Plays if released before {0} reaches Release At"), Parameter)
			: LOCTEXT("ReleaseDecisionWaitsAny", "Plays on release");
	}

	if (Params.bReleaseReached == bFullRelease)
	{
		bSilenced = false;
		if (bFullRelease)
		{
			return FText::Format(LOCTEXT("ReleaseDecisionPlaysFull", "Plays: {0} reached Release At"), Parameter);
		}
		return bHasParameter
			? FText::Format(LOCTEXT("ReleaseDecisionPlaysEarly", "Plays: released before {0} reached Release At"), Parameter)
			: LOCTEXT("ReleaseDecisionPlaysAny", "Plays: released");
	}
	return bFullRelease
		? FText::Format(LOCTEXT("ReleaseDecisionSkippedFull", "Skipped: released before {0} reached Release At"), Parameter)
		: FText::Format(LOCTEXT("ReleaseDecisionSkippedEarly", "Skipped: {0} reached Release At"), Parameter);
}

FText FFeelRecipeEditorState::GetTrackDecision(int32 TrackIndex, bool* bOutSilenced) const
{
	bool bSilencedStorage = false;
	bool& bSilenced = bOutSilenced ? *bOutSilenced : bSilencedStorage;
	bSilenced = true;
	const UFeelRecipe* CurrentRecipe = GetRecipe();
	if (!CurrentRecipe || !CurrentRecipe->Tracks.IsValidIndex(TrackIndex) || !CurrentRecipe->Tracks[TrackIndex].Step)
	{
		return FText::GetEmpty();
	}

	const FFeelTrack& Track = CurrentRecipe->Tracks[TrackIndex];
	const FFeelEvalParams Params = GetPreviewParams();

	if (!Track.bEnabled)
	{
		return LOCTEXT("DecisionMuted", "Muted");
	}
#if WITH_EDITORONLY_DATA
	if (!Track.bSolo && CurrentRecipe->HasSoloTracks())
	{
		return LOCTEXT("DecisionNotSoloed", "Silent: another track is soloed");
	}
#endif

	// Which condition failed, checked one at a time.
	const FFeelConditions& Conditions = Track.Conditions;
	if (!FFeelEvaluator::PassesConditions(Track, TrackIndex, Params))
	{
		if (Conditions.Platforms.Num() > 0 && !Conditions.Platforms.Contains(FName(FPlatformProperties::IniPlatformName())))
		{
			return LOCTEXT("DecisionPlatform", "Skipped: not on this platform");
		}
		if (Conditions.bLocalPlayerOnly && !Params.bTargetIsLocalPlayer)
		{
			return LOCTEXT("DecisionLocalPlayer", "Skipped: target is not a local player's");
		}
		if (Conditions.MaxDistance > 0.0f && Params.TargetDistance > Conditions.MaxDistance)
		{
			return FText::Format(LOCTEXT("DecisionDistance", "Skipped: target {0} cm away"), FText::AsNumber(FMath::RoundToInt32(Params.TargetDistance)));
		}
		return FText::Format(LOCTEXT("DecisionChance", "Skipped this play (chance {0}%)"), FText::AsNumber(FMath::RoundToInt32(Conditions.Chance * 100.0f)));
	}

	if (Track.AppliesTo == EFeelTrackTarget::Instigator && !Params.bHasInstigator)
	{
		return LOCTEXT("DecisionNoInstigator", "Skipped: the play had no instigator");
	}

	float ComfortScale = 1.0f;
	const UFeelStep* ComfortStep = FFeelEvaluator::ResolveStep(Track, Params.Comfort, ComfortScale);
	if (!ComfortStep)
	{
		return LOCTEXT("DecisionComfortRemoved", "Removed by comfort settings");
	}
	if (ComfortStep == Track.SubstituteStep)
	{
		bSilenced = false;
		return LOCTEXT("DecisionSubstituted", "Substitute plays (comfort)");
	}

	const float TrackScale = Params.TrackScales.IsValidIndex(TrackIndex) ? Params.TrackScales[TrackIndex] : 1.0f;
	if (TrackScale <= 0.0f)
	{
		return LOCTEXT("DecisionFlashSuppressed", "Flash suppressed by the flash limiter");
	}
	bSilenced = false;
	if (TrackScale < 0.999f)
	{
		return FText::Format(LOCTEXT("DecisionFlashSoftened", "Flash softened by the flash limiter (x{0})"), FText::AsNumber(TrackScale));
	}

	if (Cast<UFeelStep_RandomChoice>(Track.Step))
	{
		float ChoiceScale = 1.0f;
		const UFeelStep* Picked = FFeelEvaluator::ResolveTrackStep(*CurrentRecipe, TrackIndex, Params, ChoiceScale);
		bSilenced = Picked == nullptr;
		return Picked
			? FText::Format(LOCTEXT("DecisionPicked", "Picked: {0}"), Picked->GetClass()->GetDisplayNameText())
			: LOCTEXT("DecisionPickedNothing", "Picked nothing");
	}

	if (ComfortScale < 0.999f)
	{
		return FText::Format(LOCTEXT("DecisionComfortScaled", "Comfort x{0}"), FText::AsNumber(ComfortScale));
	}
	return FText::GetEmpty();
}

// FEELKIT_PRO_BEGIN
bool FFeelRecipeEditorState::CreateTrackFromSound(int32 SoundTrackIndex, bool bForceFeedback)
{
	if (IsReadOnly())
	{
		return false;
	}

	UFeelRecipe* CurrentRecipe = GetRecipe();
	const UFeelStep_PlaySound* SoundStep = CurrentRecipe && CurrentRecipe->Tracks.IsValidIndex(SoundTrackIndex) ? Cast<UFeelStep_PlaySound>(CurrentRecipe->Tracks[SoundTrackIndex].Step) : nullptr;
	const FFeelAudioEnvelope* Envelope = SoundStep ? FFeelAudioAnalysis::GetEnvelope(SoundStep->Sound) : nullptr;
	if (!Envelope || Envelope->Duration <= SoundStep->SoundStartTime)
	{
		return false;
	}

	const FScopedTransaction Transaction(bForceFeedback
		? LOCTEXT("CreateForceFeedbackFromSound", "Create Force Feedback Track From Sound")
		: LOCTEXT("CreateShakeFromSound", "Create Shake Track From Sound"));
	CurrentRecipe->Modify();

	UFeelStep* NewStep = bForceFeedback
		? static_cast<UFeelStep*>(NewObject<UFeelStep_ForceFeedbackCurve>(CurrentRecipe, NAME_None, RF_Transactional))
		: static_cast<UFeelStep*>(NewObject<UFeelStep_ProceduralShake>(CurrentRecipe, NAME_None, RF_Transactional));

	FFeelTrack NewTrack;
	NewTrack.Step = NewStep;
	NewTrack.Channel = NewStep->GetDefaultChannel();
	NewTrack.StartTime = CurrentRecipe->Tracks[SoundTrackIndex].StartTime;
	NewTrack.Duration = FMath::Max(Envelope->Duration - SoundStep->SoundStartTime, 0.05f);

	// The sound's loudness over time, as keys on normalized track time.
	FRichCurve* Curve = NewTrack.IntensityCurve.GetRichCurve();
	Curve->Reset();
	constexpr int32 NumKeys = 32;
	for (int32 KeyIndex = 0; KeyIndex < NumKeys; ++KeyIndex)
	{
		const float Alpha = static_cast<float>(KeyIndex) / static_cast<float>(NumKeys - 1);
		const float SoundTime = SoundStep->SoundStartTime + Alpha * NewTrack.Duration;
		Curve->SetKeyInterpMode(Curve->AddKey(Alpha, Envelope->SampleLoudness(SoundTime, NewTrack.Duration / NumKeys)), RCIM_Linear);
	}

	const int32 NewIndex = SoundTrackIndex + 1;
	CurrentRecipe->Tracks.Insert(NewTrack, NewIndex);
	OnTracksChanged.Broadcast();
	SelectedTrack = INDEX_NONE;
	SetSelectedTrack(NewIndex);
	return true;
}
// FEELKIT_PRO_END

void FFeelRecipeEditorState::ResetPreviewParameters()
{
	PreviewParameterValues.Reset();
	EvaluateOutput();
}

#undef LOCTEXT_NAMESPACE
