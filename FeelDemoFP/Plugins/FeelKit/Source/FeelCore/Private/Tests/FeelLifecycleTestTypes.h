// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelLifecycleTestTypes.generated.h"

/** Records lifecycle calls for automation tests. */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UFeelTestRecorderStep : public UFeelStep
{
	GENERATED_BODY()

public:
	int32 StartCount = 0;
	int32 StopCount = 0;
	int32 InterruptedStopCount = 0;
	float LastStartIntensity = -1.0f;
	int32 LastTrackIndex = INDEX_NONE;

	virtual void OnStart_Implementation(const FFeelContext& Context) override
	{
		++StartCount;
		LastStartIntensity = Context.Intensity;
		LastTrackIndex = Context.TrackIndex;
	}

	virtual void OnStop_Implementation(const FFeelContext& Context, bool bInterrupted) override
	{
		++StopCount;
		InterruptedStopCount += bInterrupted ? 1 : 0;
	}
};

/** Receives Blueprint Event step calls in automation tests. */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UFeelTestEventReceiver : public UObject
{
	GENERATED_BODY()

public:
	int32 NoInputCalls = 0;
	float LastFloat = -1.0f;
	double LastDouble = -1.0;

	UFUNCTION()
	void FeelTestNoInputs() { ++NoInputCalls; }

	UFUNCTION()
	void FeelTestFloat(float Intensity) { LastFloat = Intensity; }

	UFUNCTION()
	void FeelTestDouble(double Intensity) { LastDouble = Intensity; }
};
