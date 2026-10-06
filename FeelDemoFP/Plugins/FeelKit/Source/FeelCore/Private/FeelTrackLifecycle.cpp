// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTrackLifecycle.h"

#include "FeelEvaluator.h"
#include "FeelRecipe.h"
#include "FeelTrack.h"
#include "Steps/FeelStep_Meta.h"

void FFeelTrackLifecycle::Reset(int32 NumTracks, float InLastTime)
{
	const int32 Count = FMath::Max(NumTracks, 0);
	States.Reset();
	States.SetNumZeroed(Count);
	StartIntensities.Reset();
	StartIntensities.SetNumZeroed(Count);
	RunningSteps.Reset();
	RunningSteps.SetNum(Count);
	Children.Reset();
	ReleaseChild.Reset();
	ReleaseChildRecipe.Reset();
	LastTime = InLastTime;
}

const UFeelRecipe* FFeelTrackLifecycle::GetStartedReleaseRecipe() const
{
	return ReleaseChild.IsValid() ? ReleaseChildRecipe.Get() : nullptr;
}

TConstArrayView<int32> FFeelTrackLifecycle::GetReleaseStartedThisUpdate() const
{
	return ReleaseChild.IsValid() ? ReleaseChild->GetStartedThisUpdate() : TConstArrayView<int32>();
}

int32 FFeelTrackLifecycle::MakeNestedTrackKey(int32 ParentKey, int32 TrackIndex, int32 InnerTrackIndex)
{
	// Release recipe tracks are numbered below INDEX_NONE, and what they play stays on that side.
	const int32 Key = (TrackIndex + 1) * 10000 + InnerTrackIndex;
	return ParentKey < INDEX_NONE ? -Key : Key;
}

void FFeelTrackLifecycle::UpdateChild(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params, FMakeContext MakeContext)
{
	const TSharedPtr<FFeelTrackLifecycle>* Child = Children.Find(TrackIndex);
	const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(RunningSteps[TrackIndex].Get());
	FFeelEvalParams NestedParams;
	if (!Child || !RecipeStep || !FFeelEvaluator::MakeNestedParams(Recipe, TrackIndex, Time, Params, *RecipeStep, NestedParams))
	{
		return;
	}

	const UFeelRecipe& InnerRecipe = *RecipeStep->Recipe;
	const float StartTime = Recipe.Tracks[TrackIndex].StartTime;
	auto MakeInnerContext = [&MakeContext, &InnerRecipe, &NestedParams, TrackIndex](int32 InnerTrackIndex, float Intensity)
	{
		// The outer track decides the target; the inner track index is folded in so per-track bookkeeping stays unique.
		FFeelContext Context = MakeContext(TrackIndex, Intensity);
		Context.Recipe = const_cast<UFeelRecipe*>(&InnerRecipe);
		Context.TrackIndex = MakeNestedTrackKey(Context.TrackIndex, TrackIndex, InnerTrackIndex);
		Context.TrackDuration = FFeelEvaluator::GetTrackDuration(InnerRecipe, InnerTrackIndex, NestedParams);
		return Context;
	};
	(*Child)->Update(InnerRecipe, Time - StartTime, NestedParams, MakeInnerContext);
}

void FFeelTrackLifecycle::Update(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params, FMakeContext MakeContext)
{
	StartedThisUpdate.Reset();

	const int32 NumTracks = Recipe.Tracks.Num();
	if (States.Num() != NumTracks)
	{
		// The recipe was edited while playing: keep what is known, treat new tracks as not started.
		States.SetNumZeroed(NumTracks);
		StartIntensities.SetNumZeroed(NumTracks);
		RunningSteps.SetNum(NumTracks);
	}

	for (int32 TrackIndex = 0; TrackIndex < NumTracks; ++TrackIndex)
	{
		const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
		ETrackState& State = States[TrackIndex];
		if (State == ETrackState::Finished)
		{
			continue;
		}

		// Lengths include this play's random duration.
		const float EndTime = FFeelEvaluator::GetTrackEndTime(Recipe, TrackIndex, Params);

		if (State == ETrackState::NotStarted)
		{
			if (Track.StartTime > Time)
			{
				continue;
			}

			const bool bInstant = EndTime <= Track.StartTime;
			const bool bReachedSinceLastUpdate = Track.StartTime > LastTime;
			const bool bStillActive = !bInstant && Time <= EndTime;
			if (!bReachedSinceLastUpdate && !bStillActive)
			{
				// Already over before playback (re)started here.
				State = ETrackState::Finished;
				continue;
			}

			float ComfortScale = 1.0f;
			UFeelStep* Step = FFeelEvaluator::ShouldEvaluateTrack(Recipe, TrackIndex, Params)
				? FFeelEvaluator::ResolveTrackStep(Recipe, TrackIndex, Params, ComfortScale)
				: nullptr;
			if (!Step)
			{
				State = ETrackState::Finished;
				continue;
			}

			const float StartSampleTime = FMath::Clamp(Time, Track.StartTime, EndTime);
			const float Intensity = FFeelEvaluator::ComputeTrackIntensity(Recipe, TrackIndex, StartSampleTime, Params) * ComfortScale;
			StartIntensities[TrackIndex] = Intensity;
			RunningSteps[TrackIndex] = Step;

			Step->OnStart(MakeContext(TrackIndex, Intensity));
			State = ETrackState::Running;
			StartedThisUpdate.Add(TrackIndex);

			const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(Step);
			if (RecipeStep && RecipeStep->Recipe)
			{
				TSharedPtr<FFeelTrackLifecycle> Child = MakeShared<FFeelTrackLifecycle>();
				Child->Reset(RecipeStep->Recipe->Tracks.Num());
				Children.Add(TrackIndex, Child);
			}
		}

		if (State == ETrackState::Running && Children.Contains(TrackIndex))
		{
			UpdateChild(Recipe, TrackIndex, FMath::Min(Time, EndTime), Params, MakeContext);
		}

		if (State == ETrackState::Running && (EndTime <= Track.StartTime || Time > EndTime))
		{
			StopTrack(TrackIndex, false, MakeContext);
		}
	}

	UpdateRelease(Recipe, Time, Params, MakeContext);
	LastTime = Time;
}

void FFeelTrackLifecycle::UpdateRelease(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params, FMakeContext MakeContext)
{
	const UFeelRecipe* ReleaseRecipe = FFeelEvaluator::GetReleaseRecipe(Recipe, Params);
	if (ReleaseChild.IsValid() && ReleaseChildRecipe.Get() != ReleaseRecipe)
	{
		// The recipe was edited while playing.
		StopRelease(true, MakeContext);
	}

	FFeelEvalParams ReleaseParams;
	if (!ReleaseRecipe || Time < Recipe.SustainEnd || !FFeelEvaluator::MakeReleaseParams(Recipe, Params, ReleaseParams))
	{
		return;
	}

	if (!ReleaseChild.IsValid())
	{
		// Playback restarted past Sustain End (scrubbing) skips what the release recipe had already played by then.
		ReleaseChild = MakeShared<FFeelTrackLifecycle>();
		ReleaseChild->Reset(ReleaseRecipe->Tracks.Num(), LastTime >= Recipe.SustainEnd ? LastTime - Recipe.SustainEnd : -1.0f);
		ReleaseChildRecipe = ReleaseRecipe;
	}

	auto MakeReleaseContext = [&MakeContext, ReleaseRecipe, &ReleaseParams](int32 ReleaseTrackIndex, float Intensity)
	{
		// Release recipes play on the play's own target.
		FFeelContext Context = MakeContext(INDEX_NONE, Intensity);
		Context.Recipe = const_cast<UFeelRecipe*>(ReleaseRecipe);
		Context.TrackIndex = MakeReleaseTrackKey(ReleaseTrackIndex);
		Context.TrackDuration = FFeelEvaluator::GetTrackDuration(*ReleaseRecipe, ReleaseTrackIndex, ReleaseParams);
		return Context;
	};
	ReleaseChild->Update(*ReleaseRecipe, Time - Recipe.SustainEnd, ReleaseParams, MakeReleaseContext);
}

void FFeelTrackLifecycle::StopRelease(bool bInterrupted, FMakeContext MakeContext)
{
	const TSharedPtr<FFeelTrackLifecycle> Child = MoveTemp(ReleaseChild);
	const UFeelRecipe* ReleaseRecipe = ReleaseChildRecipe.Get();
	ReleaseChild.Reset();
	ReleaseChildRecipe.Reset();
	if (!Child.IsValid())
	{
		return;
	}
	Child->StopAll(ReleaseRecipe, bInterrupted, [&MakeContext, ReleaseRecipe](int32 ReleaseTrackIndex, float Intensity)
	{
		FFeelContext Context = MakeContext(INDEX_NONE, Intensity);
		Context.Recipe = const_cast<UFeelRecipe*>(ReleaseRecipe);
		Context.TrackIndex = MakeReleaseTrackKey(ReleaseTrackIndex);
		return Context;
	});
}

void FFeelTrackLifecycle::Rewind(const UFeelRecipe& Recipe, float ToTime, FMakeContext MakeContext)
{
	for (int32 TrackIndex = 0; TrackIndex < States.Num() && TrackIndex < Recipe.Tracks.Num(); ++TrackIndex)
	{
		if (Recipe.Tracks[TrackIndex].StartTime < ToTime)
		{
			continue;
		}

		if (States[TrackIndex] == ETrackState::Running)
		{
			StopTrack(TrackIndex, false, MakeContext);
		}
		States[TrackIndex] = ETrackState::NotStarted;
	}

	// Tracks starting exactly at ToTime are reached again.
	LastTime = ToTime - UE_KINDA_SMALL_NUMBER;
}

void FFeelTrackLifecycle::WrapSustain(const UFeelRecipe& Recipe, const FFeelEvalParams& Params, FMakeContext MakeContext)
{
	Update(Recipe, Recipe.SustainEnd - UE_KINDA_SMALL_NUMBER, Params, MakeContext);
	Rewind(Recipe, Recipe.SustainStart, MakeContext);
}

void FFeelTrackLifecycle::JumpForward(const UFeelRecipe& Recipe, float ToTime)
{
	// Tracks starting exactly at ToTime are still reached; tracks over before it count as already passed.
	LastTime = FMath::Max(LastTime, ToTime - UE_KINDA_SMALL_NUMBER);

	for (const TPair<int32, TSharedPtr<FFeelTrackLifecycle>>& Child : Children)
	{
		const UFeelStep_Recipe* RecipeStep = RunningSteps.IsValidIndex(Child.Key) ? Cast<UFeelStep_Recipe>(RunningSteps[Child.Key].Get()) : nullptr;
		if (Child.Value.IsValid() && RecipeStep && RecipeStep->Recipe && Recipe.Tracks.IsValidIndex(Child.Key))
		{
			Child.Value->JumpForward(*RecipeStep->Recipe, ToTime - Recipe.Tracks[Child.Key].StartTime);
		}
	}
}

void FFeelTrackLifecycle::StopAll(const UFeelRecipe* Recipe, bool bInterrupted, FMakeContext MakeContext)
{
	for (int32 TrackIndex = 0; TrackIndex < States.Num(); ++TrackIndex)
	{
		if (States[TrackIndex] == ETrackState::Running)
		{
			StopTrack(TrackIndex, bInterrupted, MakeContext);
		}
		States[TrackIndex] = ETrackState::Finished;
	}
	StopRelease(bInterrupted, MakeContext);
}

void FFeelTrackLifecycle::StopTrack(int32 TrackIndex, bool bInterrupted, FMakeContext MakeContext)
{
	// Inner recipe tracks stop first, with contexts of their own.
	TSharedPtr<FFeelTrackLifecycle> Child;
	if (Children.RemoveAndCopyValue(TrackIndex, Child) && Child.IsValid())
	{
		const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(RunningSteps[TrackIndex].Get());
		const UFeelRecipe* InnerRecipe = RecipeStep ? RecipeStep->Recipe.Get() : nullptr;
		Child->StopAll(InnerRecipe, bInterrupted, [&MakeContext, InnerRecipe, TrackIndex](int32 InnerTrackIndex, float Intensity)
		{
			FFeelContext Context = MakeContext(TrackIndex, Intensity);
			Context.Recipe = const_cast<UFeelRecipe*>(InnerRecipe);
			Context.TrackIndex = MakeNestedTrackKey(Context.TrackIndex, TrackIndex, InnerTrackIndex);
			return Context;
		});
	}

	if (UFeelStep* RunningStep = RunningSteps[TrackIndex].Get())
	{
		RunningStep->OnStop(MakeContext(TrackIndex, StartIntensities[TrackIndex]), bInterrupted);
	}
	RunningSteps[TrackIndex].Reset();
	States[TrackIndex] = ETrackState::Finished;
}

int32 FFeelTrackLifecycle::GetNumRunning() const
{
	int32 Count = 0;
	for (ETrackState State : States)
	{
		Count += State == ETrackState::Running ? 1 : 0;
	}
	return Count + (ReleaseChild.IsValid() ? ReleaseChild->GetNumRunning() : 0);
}

UFeelStep* FFeelTrackLifecycle::GetRunningStep(int32 TrackIndex) const
{
	return RunningSteps.IsValidIndex(TrackIndex) && States[TrackIndex] == ETrackState::Running ? RunningSteps[TrackIndex].Get() : nullptr;
}
