// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTrackClipboard.h"

#include "Curves/CurveFloat.h"
#include "FeelRecipe.h"
#include "FeelStep.h"
#include "UObject/Package.h"

namespace FeelTrackClipboardPrivate
{
	TUniquePtr<FFeelTrackClipboard> Instance;
}

FFeelTrackClipboard& FFeelTrackClipboard::Get()
{
	Startup();
	return *FeelTrackClipboardPrivate::Instance;
}

void FFeelTrackClipboard::Startup()
{
	if (!FeelTrackClipboardPrivate::Instance.IsValid())
	{
		FeelTrackClipboardPrivate::Instance = MakeUnique<FFeelTrackClipboard>();
	}
}

void FFeelTrackClipboard::Shutdown()
{
	FeelTrackClipboardPrivate::Instance.Reset();
}

void FFeelTrackClipboard::Copy(const UFeelRecipe& Recipe, TConstArrayView<int32> TrackIndices)
{
	Tracks.Reset();
	for (int32 TrackIndex : TrackIndices)
	{
		if (Recipe.Tracks.IsValidIndex(TrackIndex))
		{
			Tracks.Add(DuplicateTrack(Recipe.Tracks[TrackIndex], GetTransientPackage(), false));
		}
	}
}

int32 FFeelTrackClipboard::PasteInto(UFeelRecipe& Recipe, int32 InsertIndex, float StartTime) const
{
	if (Tracks.Num() == 0)
	{
		return INDEX_NONE;
	}

	float EarliestStart = TNumericLimits<float>::Max();
	for (const FFeelTrack& Track : Tracks)
	{
		EarliestStart = FMath::Min(EarliestStart, Track.StartTime);
	}

	const int32 FirstIndex = FMath::Clamp(InsertIndex, 0, Recipe.Tracks.Num());
	for (int32 CopyIndex = 0; CopyIndex < Tracks.Num(); ++CopyIndex)
	{
		FFeelTrack Pasted = DuplicateTrack(Tracks[CopyIndex], &Recipe, true);
		Pasted.StartTime = FMath::Max(0.0f, Pasted.StartTime - EarliestStart + StartTime);
#if WITH_EDITORONLY_DATA
		Pasted.bSolo = false;
#endif
		Recipe.Tracks.Insert(MoveTemp(Pasted), FirstIndex + CopyIndex);
	}
	return FirstIndex;
}

void FFeelTrackClipboard::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (FFeelTrack& Track : Tracks)
	{
		Collector.AddReferencedObject(Track.Step);
		Collector.AddReferencedObject(Track.SubstituteStep);
		Collector.AddReferencedObject(Track.IntensityCurve.ExternalCurve);
	}
}

FFeelTrack FFeelTrackClipboard::DuplicateTrack(const FFeelTrack& Source, UObject* Outer, bool bTransactional)
{
	FFeelTrack Copy = Source;

	auto DuplicateStep = [Outer, bTransactional](UFeelStep* Step) -> UFeelStep*
	{
		if (!Step)
		{
			return nullptr;
		}

		UFeelStep* Duplicate = DuplicateObject<UFeelStep>(Step, Outer);
		if (bTransactional)
		{
			Duplicate->SetFlags(RF_Transactional);
		}
		else
		{
			Duplicate->ClearFlags(RF_Transactional);
		}
		return Duplicate;
	};

	Copy.Step = DuplicateStep(Source.Step);
	Copy.SubstituteStep = DuplicateStep(Source.SubstituteStep);
	return Copy;
}
