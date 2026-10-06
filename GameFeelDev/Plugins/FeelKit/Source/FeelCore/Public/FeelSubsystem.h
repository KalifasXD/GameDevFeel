// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelActorDelivery.h"
#include "FeelArbiters.h"
#include "FeelAudioDelivery.h"
#include "FeelWidgetDelivery.h"
#include "FeelFrameOutput.h"
#include "FeelParameters.h"
#include "FeelPlaybackClock.h"
#include "FeelTrackLifecycle.h"
#include "FeelTypes.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"
#include "FeelSubsystem.generated.h"

class APlayerController;
class FFeelSceneViewExtension;
// FEELKIT_PRO_BEGIN
class UFeelMap;
// FEELKIT_PRO_END
class UFeelRecipe;
class UWidget;
class SFeelNumberPopLayer;
struct FFeelComfortContext;
struct FFeelContext;
struct FFeelEvalParams;

/** One playing recipe. */
USTRUCT()
struct FFeelInstance
{
	GENERATED_BODY()

	/** Kept alive while playing. */
	UPROPERTY()
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	int32 Id = 0;
	EFeelTargetType TargetType = EFeelTargetType::None;
	TWeakObjectPtr<AActor> TargetActor;
	TWeakObjectPtr<USceneComponent> TargetComponent;
	TWeakObjectPtr<UWidget> TargetWidget;
	FVector TargetLocation = FVector::ZeroVector;
	int32 LocalPlayerIndex = 0;
	FObjectKey TargetKey;

	float Intensity = 1.0f;
	int32 Seed = 0;

	/** Measured at play start for track conditions: distance to the nearest local camera (-1 unknown) and whether the target is a local player's. */
	float TargetDistance = -1.0f;
	bool bTargetIsLocalPlayer = false;

	/** Information passed with the play. The instigator is held separately as a weak pointer. */
	FFeelPlayContext PlayContext;
	TWeakObjectPtr<AActor> Instigator;

	/** Parameter values of this play, including automatic Distance and accumulator values. */
	TMap<FName, float> ParameterValues;

	/** Parameters the game passed (at play or through Set Feel Parameter). Accumulators never overwrite these. */
	TSet<FName> ExplicitParameters;

	/** The play direction in world space and in the target player's view space at play start. */
	FVector WorldDirection = FVector::ZeroVector;
	FVector ViewDirection = FVector::ZeroVector;
	FVector ViewDirectionFromLocation = FVector::ZeroVector;

	/** Recipe time, including sustain looping and release. */
	FFeelPlaybackClock Clock;

	/** Whether the recipe's Release Parameter reaching Release At released this play (rather than the game). */
	bool bReleaseReached = false;

	/** Per-track strength decided at run time (flash limiter). Same length as the recipe's tracks. */
	TArray<float> TrackScales;

	/** The same for the tracks of nested recipes (Play Recipe tracks and the release recipe), by FFeelEvaluator::MakeTrackScaleKey. */
	TMap<uint32, float> NestedTrackScales;

	/** For moment capture: whether the play started with an instigator, and the comfort of its last frame. */
	bool bHadInstigator = false;
	bool bHasComfortSnapshot = false;
	FFeelComfortScales ComfortSnapshot;

	/** Unscaled seconds since the play started. */
	double Elapsed = 0.0;
	float Duration = 0.0f;
	bool bStopping = false;
	double StopElapsed = 0.0;
	bool bFinished = false;

	/** Which tracks have started and stopped (OnStart / OnStop). */
	FFeelTrackLifecycle Lifecycle;

	/** Output of the tracks that apply to the play target, before arbitration. */
	FFeelFrameOutput Output;

	/** Output of the tracks that apply to the instigator, before arbitration. */
	FFeelFrameOutput InstigatorOutput;

	/** Rebuilds the target description (objects may be null if destroyed). */
	FEELCORE_API FFeelTarget MakeTarget() const;
};

/** A floating text shown by the Number Pop step. */
struct FEELCORE_API FFeelNumberPop
{
	FText Text;
	FVector WorldLocation = FVector::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float FontSize = 28.0f;
	float Lifetime = 1.0f;
	float RiseDistance = 60.0f;
	FVector2D ScreenOffset = FVector2D::ZeroVector;
	float PopScale = 1.6f;
	TWeakObjectPtr<APlayerController> PlayerController;

	/** Subsystem time when the pop appeared. Set by AddNumberPop. */
	double StartTime = 0.0;
};

// FEELKIT_PRO_BEGIN
/** Broadcast when a recipe starts playing. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnFeelStarted, FFeelHandle /*Handle*/, UFeelRecipe* /*Recipe*/);

/** Broadcast when a recipe stops: bInterrupted is false when it played to the end. */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFeelFinished, FFeelHandle /*Handle*/, UFeelRecipe* /*Recipe*/, bool /*bInterrupted*/);
// FEELKIT_PRO_END

/**
 * Owns and plays all recipe instances of a world.
 * Ticks only while something is playing or needs restoring, times instances in real time,
 * and delivers the arbitrated output to cameras, the screen, clocks, target components and controllers.
 */
UCLASS()
class FEELCORE_API UFeelSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Subsystem of the world of WorldContextObject, or null. */
	static UFeelSubsystem* Get(const UObject* WorldContextObject);

	/**
	 * Starts a recipe. Returns an invalid handle when it cannot start (null recipe, cooldown, max concurrent, feel.Enabled 0,
	 * dedicated server). Context carries optional parameter values, a second actor and details for this play.
	 */
	FFeelHandle PlayFeel(UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity = 1.0f, const FFeelPlayContext& Context = FFeelPlayContext());

	// FEELKIT_PRO_BEGIN
	/**
	 * Plays the recipe a Feel Map assigns to an event, chosen by the event tag and the context tags. Maps on a Feel Trigger
	 * component of the target actor are checked before the project's maps. Returns an invalid handle when no row matches.
	 */
	FFeelHandle SendFeelEvent(const FGameplayTag& Event, const FFeelTarget& Target, float Intensity = 1.0f, const FFeelPlayContext& Context = FFeelPlayContext());
	// FEELKIT_PRO_END

	/** Stops one instance, optionally fading it out over the project's blend-out time. */
	void StopFeel(FFeelHandle Handle, bool bBlendOut);

	// FEELKIT_PRO_BEGIN
	/** Ends the sustain loop of a sustained recipe so it plays to the end. Recipes without sustain are unaffected. */
	void ReleaseFeel(FFeelHandle Handle);
	// FEELKIT_PRO_END

	/** Stops every instance, or only those playing on Target when it is set. */
	void StopAllFeel(const AActor* Target = nullptr);

	// FEELKIT_PRO_BEGIN
	/** Changes a parameter of a playing recipe. Takes effect on the next frame. Returns false when the play has ended. */
	bool SetFeelParameter(FFeelHandle Handle, FName ParameterName, float Value);
	// FEELKIT_PRO_END

	/** True while the instance of Handle is playing (including while blending out). */
	bool IsPlaying(FFeelHandle Handle) const;

	/** Number of instances that have not finished. */
	int32 GetNumActiveInstances() const;

	/** Playing instances, for debugging tools. */
	const TArray<FFeelInstance>& GetInstances() const { return Instances; }

	// FEELKIT_PRO_BEGIN
	/** Adds to an accumulator for an actor, or the global value when Actor is null. Clamped to the accumulator's max. */
	void AddToAccumulator(FName AccumulatorName, float Amount, const AActor* Actor = nullptr);

	/** Sets an accumulator for an actor, or the global value when Actor is null. */
	void SetAccumulator(FName AccumulatorName, float Value, const AActor* Actor = nullptr);

	/** Current accumulator value for an actor, or the global value when Actor is null. 0 when never set. */
	float GetAccumulator(FName AccumulatorName, const AActor* Actor = nullptr) const;
	// FEELKIT_PRO_END

	/** Calls Visit for every accumulator value that is set: name, actor (null for global) and value. For debugging tools. */
	void ForEachAccumulator(TFunctionRef<void(FName, const AActor*, float)> Visit) const;

	/** Arbitrated camera and force feedback output for a local player. Returns false when no recipe affects that player. */
	bool GetCameraOutput(const APlayerController* PlayerController, FFeelFrameOutput& OutOutput) const;

	/** Arbitrated screen output (flash, post-process) for this world's views. */
	const FFeelFrameOutput& GetScreenOutput() const { return ScreenOutput; }

	/** Shows a floating text over the game view (Number Pop step). */
	void AddNumberPop(const FFeelNumberPop& Pop);

	/** Floating texts currently shown, and the clock their start times use. */
	const TArray<FFeelNumberPop>& GetNumberPops() const { return NumberPops; }
	double GetNumberPopTime() const { return NumberPopTime; }

	// FEELKIT_PRO_BEGIN
	/** Native notifications for starts and finishes. */
	FOnFeelStarted OnFeelStarted;
	FOnFeelFinished OnFeelFinished;
	// FEELKIT_PRO_END

	/**
	 * Keeps recipes playing while the game is paused, for example while a settings menu previews effects. Every call
	 * must be matched by one call to RemovePausedPlaybackRequest.
	 */
	void AddPausedPlaybackRequest() { ++PausedPlaybackRequests; }
	void RemovePausedPlaybackRequest() { PausedPlaybackRequests = FMath::Max(0, PausedPlaybackRequests - 1); }

	//~ Begin UTickableWorldSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return PausedPlaybackRequests > 0; }
	virtual TStatId GetStatId() const override;
	//~ End UTickableWorldSubsystem interface

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** Requests to keep playing while the game is paused. */
	int32 PausedPlaybackRequests = 0;

	/** A player's dynamic force feedback, one engine handle per motor (left large, left small, right large, right small). */
	struct FForceFeedbackPlayer
	{
		TWeakObjectPtr<APlayerController> PlayerController;
		uint64 MotorHandles[4] = { 0, 0, 0, 0 };
		bool bRequested = false;
	};

	/** One accumulator value, per actor or global. */
	struct FAccumulatorValue
	{
		FName Name;
		TWeakObjectPtr<const AActor> Actor;
		bool bGlobal = true;
		float Value = 0.0f;
		double LastAddTime = 0.0;
	};

	// FEELKIT_PRO_BEGIN
	/** "showdebug feel" lists playing recipes, accumulators and comfort on the HUD. */
	void HandleShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
	FDelegateHandle ShowDebugHandle;
	// FEELKIT_PRO_END

	void AdvanceInstance(FFeelInstance& Instance, float RealDeltaSeconds, bool bEnabled);
	void FinishInstance(FFeelInstance& Instance, bool bInterrupted, bool bNotifySteps = true);
	void ResolveAndDeliver();
	void DeliverForceFeedback(const TMap<FObjectKey, TWeakObjectPtr<APlayerController>>& PlayerControllers);
	void StopForceFeedback(FForceFeedbackPlayer& Player);
	void EnsureCameraModifiers();
	void RestoreEverything();
	void UpdateAccumulators(float RealDeltaSeconds);
	void RefreshAccumulatorParameters(FFeelInstance& Instance) const;

	// FEELKIT_PRO_BEGIN
	/** Events already reported as matching no Feel Map entry, so the warning appears once per event. */
	TSet<FGameplayTag> WarnedUnmatchedEvents;
	// FEELKIT_PRO_END
	FAccumulatorValue* FindAccumulatorValue(FName AccumulatorName, const AActor* Actor);
	const FAccumulatorValue* FindAccumulatorValue(FName AccumulatorName, const AActor* Actor) const;
	// FEELKIT_PRO_BEGIN
	void CollectFeelMaps(const FFeelTarget& Target, TArray<const UFeelMap*>& OutMaps);
	// FEELKIT_PRO_END
	FFeelInstance* FindInstance(FFeelHandle Handle);
	bool IsTargetAlive(const FFeelInstance& Instance) const;
	FFeelContext MakeContext(const FFeelInstance& Instance, int32 TrackIndex, float TrackIntensity) const;
	FFeelComfortContext MakeComfortContext(const FFeelInstance& Instance) const;
	FFeelEvalParams MakeEvalParams(const FFeelInstance& Instance, float BlendWeight) const;
	FObjectKey MakeTargetKey(const FFeelTarget& Target) const;

	UPROPERTY(Transient)
	TArray<FFeelInstance> Instances;

	/** Plays started while instances are being advanced (for example from a step callback). Added after the update. */
	UPROPERTY(Transient)
	TArray<FFeelInstance> PendingInstances;
	bool bAdvancingInstances = false;

	// FEELKIT_PRO_BEGIN
	/** Project Feel Maps, loaded on first use. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFeelMap>> ProjectFeelMaps;
	bool bProjectFeelMapsLoaded = false;
	// FEELKIT_PRO_END

	TArray<FAccumulatorValue> AccumulatorValues;

	/** Unscaled seconds the accumulators have been updated for, used for decay delays. */
	double AccumulatorTime = 0.0;

	/** Flash limiter per local player. */
	TMap<FObjectKey, FFeelFlashLimiter> FlashLimiters;

	TArray<FFeelNumberPop> NumberPops;
	double NumberPopTime = 0.0;
	TSharedPtr<SFeelNumberPopLayer> NumberPopLayer;

	/** Per local player: arbitrated camera and force feedback output. */
	TMap<FObjectKey, FFeelFrameOutput> CameraOutputs;
	FFeelFrameOutput ScreenOutput;
	FFeelTimeArbiter TimeArbiter;
	FFeelPlaybackGate PlaybackGate;
	FFeelActorDelivery ActorDelivery;
	FFeelWidgetDelivery WidgetDelivery;
	FFeelAudioDelivery AudioDelivery;
	TMap<FObjectKey, FForceFeedbackPlayer> ForceFeedbackPlayers;
	TSharedPtr<FFeelSceneViewExtension, ESPMode::ThreadSafe> ViewExtension;
	int32 NextInstanceId = 1;
};
