# Writing your own steps in Blueprint and C++ {#ch20}

The 37 built-in steps cover most feedback, and the Blueprint Event step calls any Blueprint code at the start and end of a track. When a game needs an effect of its own that behaves like the built-in ones, with its own settings in the track's Details and its own entry in the **+ Track** menu, it can add a step class. A step written in Blueprint can act when its track starts and stops; a step written in C++ can also produce continuous output, frame by frame, like the built-in camera and screen steps.

Custom steps work in both editions.

## How a step works {#ch20_model}

A step is a small object that lives inside a track. FeelKit calls it in three ways:

| Call | When | Use it for |
|---|---|---|
| **Evaluate** (C++ only) | Every frame while the track plays, in the game and in the preview. | Continuous output: camera offsets, screen flashes, scale, time dilation, post process, material parameters, controller vibration. |
| **On Start** | Once, when the track begins. | Side effects: spawning something, playing a sound, sending an event. |
| **On Stop** | Once, when the track ends or its play is stopped. **Interrupted** is true when the play was stopped early. | Undoing what On Start did, or a closing side effect. |

**Evaluate computes; FeelKit applies.** Evaluate never touches the camera, the screen or an actor. It hands its contribution to an output sink, and FeelKit combines the contributions of every track and every play, applies the player's comfort and the rules for overlapping effects ([Ref: ch02_channels]), and delivers the result. The same code feeds the editor preview, so a custom step previews exactly like a built-in one.

**Evaluate must be pure.** Its output may depend only on its context: the time since the track started, the track's length and progress, the intensity, the seed, and the play's directions. It must not remember anything between frames. That is what lets the preview scrub a track forward and backward and show the same frame each time, and what makes a replayed play match the game ([Ref: ch10_replay]).

## A step in Blueprint {#ch20_blueprint}

1. In the Content Browser, choose **Add** > **Blueprint Class**, open **All Classes**, and pick **Feel Step** as the parent class.
2. Add variables for the step's settings and tick **Instance Editable** on each: they appear in the track's **Details**, per track.
3. Override the functions the step needs, from **Functions** > **Override** in the **My Blueprint** panel:

| Function | Purpose |
|---|---|
| **On Start** | Runs when the track begins. **Context** gives the world, the target, the instigator, the play context with its parameters, the local player controller, the recipe, the track's intensity and length, and the target's location. |
| **On Stop** | Runs when the track ends; **Interrupted** says whether the play was stopped early. |
| **Get Default Channel** | Returns the channel a new track with this step gets, such as `Feel.Audio`. The channel decides the comfort group ([Ref: ch08_groups]). |
| **Supports Preview** | Returns whether the recipe editor's preview runs the step. Return false for a step whose side effects only make sense in the game, such as spawning gameplay actors: the preview then skips it and marks the track **No preview**. |

The step appears in the **+ Track** menu under its Blueprint's name, and saving a recipe checks it like any other.

A Blueprint step cannot override Evaluate, so it cannot move the camera or fade the screen frame by frame. For that, write the step in C++, or combine a Blueprint step with built-in tracks in the same recipe.

## A step in C++ {#ch20_cpp}

A C++ step subclasses `UFeelStep` and overrides `Evaluate`. The module that holds it needs `FeelCore` and `GameplayTags` in its dependencies:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
	"Core", "CoreUObject", "Engine", "FeelCore", "GameplayTags" });
```

This step moves the camera up and down a set number of times over the track, scaled by the track's intensity. Replace `MYGAME_API` with your module's API macro.

```cpp
// MyFeelStep_CameraBob.h
#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "MyFeelStep_CameraBob.generated.h"

/** Moves the camera up and down a set number of times over the track. */
UCLASS(meta = (DisplayName = "Camera Bob"))
class MYGAME_API UMyFeelStep_CameraBob : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Height of each bob. */
	UPROPERTY(EditAnywhere, Category = "Bob", meta = (Units = "Centimeters"))
	float Height = 4.0f;

	/** Number of up and down movements over the length of the track. */
	UPROPERTY(EditAnywhere, Category = "Bob", meta = (ClampMin = "1"))
	int32 Bobs = 2;

	virtual void Evaluate(const FFeelStepEvalContext& Context,
		IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors,
		TArray<FText>& OutWarnings) const override;
#endif
};
```

```cpp
// MyFeelStep_CameraBob.cpp
#include "MyFeelStep_CameraBob.h"
#include "FeelOutputSink.h"
#include "FeelTags.h"

void UMyFeelStep_CameraBob::Evaluate(const FFeelStepEvalContext& Context,
	IFeelOutputSink& Sink) const
{
	// Depends only on the context, so the editor can scrub the track in any direction.
	const float Wave = FMath::Sin(Context.Alpha * UE_TWO_PI * Bobs);
	const FVector Offset(0.0, 0.0, Height * Wave * Context.Intensity);
	Sink.AddCameraOffset(Offset, FRotator::ZeroRotator);
}

FGameplayTag UMyFeelStep_CameraBob::GetDefaultChannel_Implementation() const
{
	return FeelTags::Camera_Motion;
}

#if WITH_EDITOR
void UMyFeelStep_CameraBob::ValidateStep(TArray<FText>& OutErrors,
	TArray<FText>& OutWarnings) const
{
	if (Height <= 0.0f)
	{
		OutWarnings.Add(NSLOCTEXT("MyGame", "BobHeight",
			"Camera Bob has no height, so it does nothing."));
	}
}
#endif
```

This code was compiled against FeelKit on Unreal Engine 5.6 with no warnings. After a rebuild, **Camera Bob** is in the **+ Track** menu, previews in the recipe editor, and plays in the game with the player's Camera Motion comfort applied.

**The context.** `FFeelStepEvalContext` holds `LocalTime` (seconds since the track started), `Duration`, `Alpha` (0 at the start, 1 at the end), `Intensity` (the play's intensity times the track's intensity curve and the comfort scale), `Seed`, and the play's `Direction`, `ViewDirection` and `ViewDirectionFromLocation`. Derive any randomness from `Seed`, never from a random number generator, so that each frame can be recomputed.

**The sink.** `IFeelOutputSink` receives the contribution. Its functions:

| Function | Contribution |
|---|---|
| `AddCameraOffset` | Camera location offset (X forward, Y right, Z up, in cm) and rotation (degrees). |
| `AddFieldOfViewOffset` | Field of view change in degrees. |
| `AddScreenFlash`, `AddScreenFade`, `AddScreenTint` | Full-screen color with an alpha or weight from 0 to 1. |
| `AddPostProcess` | A post-process parameter (vignette, chromatic aberration, saturation) blended in with a weight. |
| `AddPostProcessMaterial` | A post-process material with a weight, and the scalar parameter that receives it. |
| `AddTimeDilation` | A time dilation for the world or the target, with a priority. |
| `AddTargetScale`, `AddTargetTransform` | Relative scale, location and rotation of the target. |
| `AddMaterialParameter`, `AddOverlayFlash`, `AddLight` | Material parameters, an overlay flash and light changes on the target. |
| `AddForceFeedback` | Strength of each controller motor, from 0 to 1. |
| `AddSoundClassAdjust` | Volume, pitch and low-pass filter of a sound class. |
| `AddWidgetTransform`, `AddWidgetColor` | Render transform and tint of a widget target. |

**Other overrides.**

| Override | Default | When to change it |
|---|---|---|
| `OnStart_Implementation`, `OnStop_Implementation` | Nothing | Side effects, as in Blueprint. |
| `SupportsPreview_Implementation` | true | Return false when the preview cannot show the step. |
| `UsesConstantIntensityByDefault` | false | Return true for steps that shape their own envelope, such as a spring; new tracks then start with a flat intensity curve instead of a fade. |
| `RequiresDuration` | true | Return false for steps that work as instant tracks, such as a sound; saving then accepts a length of 0. |
| `ValidateStep` | Nothing | Report problems with the step's settings when a recipe is saved ([Ref: ch05_validation]). |

## Channels and comfort {#ch20_channels}

A track's channel decides which comfort group scales it, and which effects it competes with. Give a custom step the channel of the built-in steps it resembles: `Feel.Camera.Motion` for a camera move, `Feel.Screen.Flash` for a flash, and so on. A step on a channel that no comfort group maps plays at the player's **Master** scale only ([Ref: ch08_groups]). A game can add its own channel tags under `Feel` and map them to a group in the project settings.
