// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "FeelFrameOutput.h"
#include "UObject/WeakObjectPtr.h"
#include "FeelCameraModifier.generated.h"

class UCameraShakeBase;
struct FFeelComfortScales;

/**
 * Applies FeelKit camera output to a local player's view, with the player's motion comfort.
 * When enabled in the project settings, also applies the player's camera shake comfort to shakes the
 * game plays through the engine.
 * Added automatically for local players; works with any camera that uses the standard camera manager.
 */
UCLASS()
class FEELCORE_API UFeelCameraModifier : public UCameraModifier
{
	GENERATED_BODY()

public:
	using UCameraModifier::ModifyCamera;

	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

	/** Adds the modifier to a local player controller's camera manager if it does not have one yet. */
	static void EnsureOn(class APlayerController* PlayerController);

private:
	/** Scales engine camera shakes by the player's comfort, remembering each shake's own scale. */
	void ScaleEngineCameraShakes(const FFeelComfortScales& Scales);

	/** Field of view offset applied last frame, for the change rate limit. */
	float PreviousFieldOfViewOffset = 0.0f;

	/** Instances of post-process materials that fade through a weight parameter. */
	FFeelPostProcessMaterialInstances PostProcessMaterialInstances;

	/** Original scale of each engine shake this modifier has scaled. */
	TMap<TWeakObjectPtr<UCameraShakeBase>, float> EngineShakeBaseScales;
};
