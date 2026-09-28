// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelTrack.h"
#include "UObject/GCObject.h"

class UFeelRecipe;

/**
 * In-process clipboard for recipe tracks.
 * Copies hold their own duplicates of the steps, so the source recipe can change or close before pasting.
 */
class FFeelTrackClipboard : public FGCObject
{
public:
	static FFeelTrackClipboard& Get();
	static void Startup();
	static void Shutdown();

	/** Copies the given tracks of Recipe in order. Invalid indices are ignored. */
	void Copy(const UFeelRecipe& Recipe, TConstArrayView<int32> TrackIndices);

	bool HasTracks() const { return Tracks.Num() > 0; }
	int32 Num() const { return Tracks.Num(); }
	void Clear() { Tracks.Reset(); }

	/**
	 * Inserts copies into Recipe at InsertIndex, shifted so the earliest copied track starts at StartTime.
	 * Returns the index of the first pasted track, or INDEX_NONE when there is nothing to paste. Call Modify on the recipe first.
	 */
	int32 PasteInto(UFeelRecipe& Recipe, int32 InsertIndex, float StartTime) const;

	//~ Begin FGCObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override { return TEXT("FFeelTrackClipboard"); }
	//~ End FGCObject interface

private:
	static FFeelTrack DuplicateTrack(const FFeelTrack& Source, UObject* Outer, bool bTransactional);

	TArray<FFeelTrack> Tracks;
};
