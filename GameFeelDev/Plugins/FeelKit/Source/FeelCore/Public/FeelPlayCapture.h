// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortTypes.h"
#include "Misc/DateTime.h"
#include "UObject/WeakObjectPtr.h"

class UFeelRecipe;

/**
 * Everything needed to replay one play exactly in the recipe editor: evaluation is a pure function of these values, so the
 * editor shows the same frames the game showed. Recorded when a play ends, in development builds only.
 */
struct FEELCORE_API FFeelPlayCapture
{
	TWeakObjectPtr<UFeelRecipe> Recipe;
	FString RecipeName;
	FString TargetName;
	FString WorldName;
	FDateTime EndedAt;

	int32 Seed = 0;
	float Intensity = 1.0f;
	TMap<FName, float> ParameterValues;
	float TargetDistance = -1.0f;
	bool bTargetIsLocalPlayer = true;
	bool bHadInstigator = false;
	FVector ViewDirection = FVector::ZeroVector;
	FVector ViewDirectionFromLocation = FVector::ZeroVector;

	/** The comfort that applied to the play. */
	bool bHasComfort = false;
	FFeelComfortScales ComfortScales;

	/** Per-track strength decided at run time, such as flashes softened or suppressed by the flash limiter. */
	TArray<float> TrackScales;

	/** The same for the tracks of nested recipes (Play Recipe tracks and the release recipe). */
	TMap<uint32, float> NestedTrackScales;

	/** Seconds the play lasted and how it ended. */
	float PlayedSeconds = 0.0f;
	bool bReleased = false;
	bool bInterrupted = false;

	/** Whether the recipe's Release Parameter released the play, which picks the release recipe in a replay. */
	bool bReleaseReached = false;
};

/** Recent plays of every world, newest last, kept across play sessions so they can be opened after stopping play. */
class FEELCORE_API FFeelPlayCaptureStore
{
public:
	static FFeelPlayCaptureStore& Get();

	/** Most plays kept. Older plays are dropped. */
	static constexpr int32 MaxCaptures = 64;

	void Add(FFeelPlayCapture&& Capture);
	void Clear();
	const TArray<FFeelPlayCapture>& GetCaptures() const { return Captures; }

	/** Broadcast after a capture is added or the store is cleared. */
	FSimpleMulticastDelegate OnChanged;

private:
	TArray<FFeelPlayCapture> Captures;
};
