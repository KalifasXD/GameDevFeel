// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelParameters.h"
#include "FeelTypes.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FeelBlueprintLibrary.generated.h"

class APlayerController;
class UFeelComfortSubsystem;
class UFeelRecipe;

/** Blueprint and C++ entry points for playing recipes. */
UCLASS()
class FEELCORE_API UFeelBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Plays a recipe.
	 * @param WorldContextObject	Any object in the world to play in.
	 * @param Recipe				The recipe to play.
	 * @param Target				What the recipe plays on. Camera and screen effects go to the target's local player.
	 * @param Intensity				Multiplier for every track of this play.
	 * @return						Handle to stop or query this play. Invalid if the recipe could not start (cooldown, max concurrent, or feel.Enabled 0).
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static FFeelHandle PlayFeel(const UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity = 1.0f);

	// FEELKIT_PRO_BEGIN
	/**
	 * Plays a recipe like Play Feel, with extra information for this play. Use it when the recipe declares parameters,
	 * has tracks that apply to a second actor, or uses context tags.
	 * - Parameters: values for the recipe's parameters; tracks with parameter mappings scale their intensity by them.
	 * - Instigator: a second actor; tracks set to Applies To = Instigator play their actor effects on it.
	 * - Direction, Location, Normal, Context Tags: passed to steps and Blueprint events, and used by Feel Maps.
	 * With an empty context this behaves exactly like Play Feel.
	 * @param WorldContextObject	Any object in the world to play in.
	 * @param Recipe				The recipe to play.
	 * @param Target				What the recipe plays on.
	 * @param Context				Parameter values, second actor and details for this play. Every field is optional.
	 * @param Intensity				Multiplier for every track of this play.
	 * @return						Handle to stop or query this play. Invalid if the recipe could not start.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Context"))
	static FFeelHandle PlayFeelWithContext(const UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity = 1.0f);

	/**
	 * Sends a named event and plays the recipe a Feel Map assigns to it. The map row is chosen by the event tag and the
	 * context tags: the most specific match wins. Maps on a Feel Trigger component of the target actor are checked before
	 * the project's maps (Project Settings > Plugins > FeelKit > Feel Maps). Use it to keep gameplay code free of recipe choices:
	 * gameplay sends what happened, the maps decide how it feels.
	 * @param WorldContextObject	Any object in the world to play in.
	 * @param Event					Event tag to look up.
	 * @param Target				What the recipe plays on.
	 * @param Context				Optional parameter values, second actor, details and context tags.
	 * @param Intensity				Multiplier for every track of this play, multiplied by the row's intensity scale.
	 * @return						Handle of the play. Invalid when no row matches or the recipe could not start.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "Context"))
	static FFeelHandle SendFeelEvent(const UObject* WorldContextObject, FGameplayTag Event, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity = 1.0f);

	/**
	 * Ends the sustain loop of a playing recipe so it plays its remaining tracks and finishes. Recipes without a sustain
	 * region are not affected.
	 * @param WorldContextObject	Any object in the world the recipe plays in.
	 * @param Handle				Handle returned when the recipe started.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static void ReleaseFeel(const UObject* WorldContextObject, FFeelHandle Handle);

	/**
	 * Changes a parameter of a recipe that is playing. Tracks mapped to the parameter follow the new value from the next
	 * frame. Useful for sustained recipes whose strength follows a changing value.
	 * @param WorldContextObject	Any object in the world the recipe plays in.
	 * @param Handle				Handle returned when the recipe started.
	 * @param ParameterName			Name of a parameter the recipe declares. The dropdown lists the parameters of the recipe
	 *								played by the node the Handle comes from, or every recipe parameter in the project.
	 * @param Value					New value. Clamped to the parameter's range.
	 * @return						False when the recipe is no longer playing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static bool SetFeelParameter(const UObject* WorldContextObject, FFeelHandle Handle, UPARAM(meta = (FeelParameterFromHandle = "Handle")) FName ParameterName, float Value);

	/**
	 * Adds to an accumulator (Project Settings > Plugins > FeelKit > Accumulators). The value is clamped to the accumulator's max and
	 * decays over time. Recipe parameters that read the accumulator follow it.
	 * @param WorldContextObject	Any object in the world.
	 * @param AccumulatorName		Name of the accumulator.
	 * @param Amount				Amount to add. Negative amounts subtract.
	 * @param Actor					Actor the value belongs to. Leave empty for the global value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Accumulators", meta = (WorldContext = "WorldContextObject"))
	static void AddToFeelAccumulator(const UObject* WorldContextObject, UPARAM(meta = (GetOptions = "GetFeelAccumulatorOptions")) FName AccumulatorName, float Amount, AActor* Actor = nullptr);

	/**
	 * Sets an accumulator to a value, clamped to its max.
	 * @param WorldContextObject	Any object in the world.
	 * @param AccumulatorName		Name of the accumulator.
	 * @param Value					New value.
	 * @param Actor					Actor the value belongs to. Leave empty for the global value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Accumulators", meta = (WorldContext = "WorldContextObject"))
	static void SetFeelAccumulator(const UObject* WorldContextObject, UPARAM(meta = (GetOptions = "GetFeelAccumulatorOptions")) FName AccumulatorName, float Value, AActor* Actor = nullptr);

	/**
	 * Current value of an accumulator.
	 * @param WorldContextObject	Any object in the world.
	 * @param AccumulatorName		Name of the accumulator.
	 * @param Actor					Actor the value belongs to. Leave empty for the global value.
	 * @return						The value, or 0 when it was never set or has decayed.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel|Accumulators", meta = (WorldContext = "WorldContextObject"))
	static float GetFeelAccumulator(const UObject* WorldContextObject, UPARAM(meta = (GetOptions = "GetFeelAccumulatorOptions")) FName AccumulatorName, AActor* Actor = nullptr);

	/** Names of the accumulators in Project Settings > Plugins > FeelKit, for the accumulator name dropdowns on Blueprint nodes. */
	UFUNCTION()
	static TArray<FName> GetFeelAccumulatorOptions();
	// FEELKIT_PRO_END

	/**
	 * Stops one playing recipe.
	 * @param WorldContextObject	Any object in the world the recipe plays in.
	 * @param Handle				Handle returned by Play Feel.
	 * @param bBlendOut				Fade out over the project's blend-out time instead of stopping at once.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static void StopFeel(const UObject* WorldContextObject, FFeelHandle Handle, bool bBlendOut = true);

	/**
	 * Stops playing recipes at once.
	 * @param WorldContextObject	Any object in the world to stop recipes in.
	 * @param Target				Only stop recipes playing on this actor. Leave empty to stop all.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static void StopAllFeel(const UObject* WorldContextObject, AActor* Target = nullptr);

	/**
	 * Whether a recipe is still playing, including while it blends out.
	 * @param WorldContextObject	Any object in the world the recipe plays in.
	 * @param Handle				Handle returned by Play Feel.
	 * @return						True while playing.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel", meta = (WorldContext = "WorldContextObject"))
	static bool IsFeelPlaying(const UObject* WorldContextObject, FFeelHandle Handle);

	/**
	 * Whether a handle came from a successful Play Feel. The recipe may have finished since.
	 * @param Handle	Handle to check.
	 * @return			True for handles of recipes that started.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel", meta = (DisplayName = "Is Valid (Feel Handle)"))
	static bool IsValidFeelHandle(FFeelHandle Handle);

	/**
	 * Target an actor. Actor effects such as Scale Punch use its root component; when the root takes part in collision (a character's capsule), they use its visible child components instead, so collision and movement stay unchanged.
	 * @param Actor	The actor to play on.
	 * @return		The target.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	static FFeelTarget MakeFeelTargetFromActor(AActor* Actor);

	/**
	 * Target a scene component, such as a character's mesh.
	 * @param Component	The component to play on.
	 * @return			The target.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	static FFeelTarget MakeFeelTargetFromComponent(USceneComponent* Component);

	/**
	 * Target a world location. Camera and screen effects go to the first local player.
	 * @param Location	The location to play at.
	 * @return			The target.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	static FFeelTarget MakeFeelTargetAtLocation(FVector Location);

	/**
	 * Target a local player's camera.
	 * @param PlayerIndex	Index of the local player.
	 * @return				The target.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	static FFeelTarget MakeFeelTargetFromLocalPlayerCamera(int32 PlayerIndex = 0);

	/**
	 * Target a UMG widget. Widget steps (punch, shake, flash) apply to it; camera, screen and haptic effects go to its owning player.
	 * @param Widget	The widget to play on.
	 * @return			The target.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	static FFeelTarget MakeFeelTargetFromWidget(UWidget* Widget);

	/**
	 * Comfort settings of a local player: read and change scales, apply presets, save.
	 * @param PlayerController	A local player controller.
	 * @return					The player's comfort subsystem, or none for controllers without a local player.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel|Comfort")
	static UFeelComfortSubsystem* GetFeelComfort(const APlayerController* PlayerController);

	/**
	 * Turns all FeelKit feedback off or on, for comparing a game with and without it. Off stops every playing recipe at
	 * once (slowed time returns to normal) and nothing new plays until it is turned on again. Only FeelKit is affected:
	 * the game's own camera shakes, sounds and effects keep working. Same switch as the console variable feel.Enabled;
	 * applies to the whole game on this machine and is not saved.
	 * @param bEnabled	True to turn FeelKit on, false to turn it off.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Switch")
	static void SetFeelEnabled(bool bEnabled);

	/**
	 * Turns FeelKit off when it is on, and on when it is off (see Set Feel Enabled).
	 * @return	True when FeelKit is on after the switch.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Switch")
	static bool ToggleFeel();

	/**
	 * Whether FeelKit feedback is on (see Set Feel Enabled).
	 * @return	True when recipes play.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel|Switch")
	static bool IsFeelEnabled();
};
