// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelFrameOutput.h"
#include "FeelSettings.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"

class UFeelRecipe;

/** Limits for additive camera arbitration. */
struct FEELCORE_API FFeelCameraCaps
{
	float MaxLocationOffset = 30.0f;
	float MaxRotationOffset = 8.0f;
	float MaxFieldOfViewOffset = 15.0f;
};

/** Combine the outputs of overlapping recipe instances. */
namespace FeelArbiters
{
	/** Combines camera location, rotation and field of view for one player. Other fields are left untouched. */
	FEELCORE_API void ResolveCamera(TConstArrayView<const FFeelFrameOutput*> Inputs, EFeelCameraArbitration Mode, const FFeelCameraCaps& Caps, FFeelFrameOutput& OutResult);

	/** The strongest flash wins, and per post-process parameter the strongest weight wins. Other fields are left untouched. */
	FEELCORE_API void ResolveScreen(TConstArrayView<const FFeelFrameOutput*> Inputs, FFeelFrameOutput& OutResult);

	/** Per controller motor, the strongest value wins. Other fields are left untouched. */
	FEELCORE_API void ResolveForceFeedback(TConstArrayView<const FFeelFrameOutput*> Inputs, FFeelFrameOutput& OutResult);
}

/**
 * One owner per time target (world settings for global time, or an actor).
 * Each frame, the highest-priority request wins. When no request remains, the dilation the target had before
 * FeelKit took control is restored. Destroyed targets are dropped without being touched.
 */
class FEELCORE_API FFeelTimeArbiter
{
public:
	using FReadDilation = TFunctionRef<float(UObject*)>;
	using FWriteDilation = TFunctionRef<void(UObject*, float)>;

	/** Adds a request for the current frame. Inactive requests are ignored. */
	void Submit(UObject* Target, const FFeelTimeRequest& Request);

	/** Applies this frame's winners, restores targets that are no longer requested, then clears the requests. */
	void Apply(FReadDilation ReadDilation, FWriteDilation WriteDilation);

	/** Restores every owned target and forgets all state. */
	void RestoreAll(FWriteDilation WriteDilation);

	/** True while FeelKit controls at least one clock. */
	bool IsOwningAny() const { return Owned.Num() > 0; }

	/** True while FeelKit controls this target's clock. */
	bool IsOwning(const UObject* Target) const { return Owned.Contains(FObjectKey(Target)); }

private:
	struct FOwnedTarget
	{
		TWeakObjectPtr<UObject> Target;
		float PreviousDilation = 1.0f;
	};

	struct FPendingRequest
	{
		TWeakObjectPtr<UObject> Target;
		FFeelTimeRequest Request;
	};

	TMap<FObjectKey, FOwnedTarget> Owned;
	TMap<FObjectKey, FPendingRequest> Pending;
};

/** Enforces Cooldown and MaxConcurrent per recipe and target. */
class FEELCORE_API FFeelPlaybackGate
{
public:
	/** True when the recipe may start on this target. ActiveInstances counts instances of this recipe already playing on it. */
	bool CanPlay(const UFeelRecipe& Recipe, const FObjectKey& TargetKey, double NowSeconds, int32 ActiveInstances) const;

	/** Records a successful start for cooldown tracking. */
	void NotifyPlayed(const UFeelRecipe& Recipe, const FObjectKey& TargetKey, double NowSeconds);

	void Reset() { LastPlayTimes.Reset(); }

private:
	TMap<TPair<FObjectKey, FObjectKey>, double> LastPlayTimes;
};
