// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/EngineBaseTypes.h"
#include "FeelComfortTypes.h"
#include "FeelSettings.generated.h"

/** How camera contributions of overlapping recipe instances combine. */
UENUM(BlueprintType)
enum class EFeelCameraArbitration : uint8
{
	/** Per quantity (location, rotation, field of view), the strongest contribution wins. */
	StrongestWins,
	/** Contributions add up, then are clamped to the caps. */
	AdditiveCapped,
};

// FEELKIT_PRO_BEGIN
class UFeelMap;
// FEELKIT_PRO_END

/**
 * A named value that builds up when the game adds to it and falls back toward zero over time. Recipe parameters can read
 * it, so repeated plays grow stronger while they come quickly and calm down when they stop.
 */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelAccumulatorDefinition
{
	GENERATED_BODY()

	/** Name used to add to the value and to read it from recipe parameters. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Accumulator")
	FName Name;

	/** Highest value. Additions above it are clamped. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Accumulator", meta = (ClampMin = "0"))
	float MaxValue = 1.0f;

	/** How fast the value falls toward zero, in units per second. 0 keeps it until the game changes it. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Accumulator", meta = (ClampMin = "0"))
	float DecayPerSecond = 1.0f;

	/** Seconds after the last addition before the value starts to fall. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Accumulator", meta = (ClampMin = "0", Units = "Seconds"))
	float DecayDelay = 0.0f;
};

/** Project-wide FeelKit settings (Project Settings > Plugins > FeelKit). */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "FeelKit"))
class FEELCORE_API UFeelSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFeelSettings();

	/** How overlapping camera effects combine. */
	UPROPERTY(Config, EditAnywhere, Category = "Camera")
	EFeelCameraArbitration CameraArbitration = EFeelCameraArbitration::StrongestWins;

	/** Largest combined camera location offset in additive mode. */
	UPROPERTY(Config, EditAnywhere, Category = "Camera", meta = (ClampMin = "0", Units = "Centimeters", EditCondition = "CameraArbitration == EFeelCameraArbitration::AdditiveCapped"))
	float MaxCameraLocationOffset = 30.0f;

	/** Largest combined camera rotation offset per axis in additive mode. */
	UPROPERTY(Config, EditAnywhere, Category = "Camera", meta = (ClampMin = "0", Units = "Degrees", EditCondition = "CameraArbitration == EFeelCameraArbitration::AdditiveCapped"))
	float MaxCameraRotationOffset = 8.0f;

	/** Largest combined field of view change in additive mode. */
	UPROPERTY(Config, EditAnywhere, Category = "Camera", meta = (ClampMin = "0", Units = "Degrees", EditCondition = "CameraArbitration == EFeelCameraArbitration::AdditiveCapped"))
	float MaxFieldOfViewOffset = 15.0f;

	/** Fade-out length when a recipe is stopped with blend out. */
	UPROPERTY(Config, EditAnywhere, Category = "Playback", meta = (ClampMin = "0", Units = "Seconds"))
	float BlendOutTime = 0.2f;

	/**
	 * In networked games (client, listen server), world time is shared by everyone, so by default a Global Hitstop or
	 * global Slow-mo slows only the actors of the play (its target) on this machine instead of the whole world.
	 * Turn on to change world time anyway: on a listen server that slows every connected player, and on a client the
	 * server's time can override it. Single-player games always use world time.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Playback")
	bool bAllowGlobalTimeDilationInMultiplayer = false;

	/** Whether global time requests become per-actor requests in this net mode. */
	bool ShouldLocalizeGlobalTimeDilation(ENetMode NetMode) const
	{
		return NetMode != NM_Standalone && !bAllowGlobalTimeDilationInMultiplayer;
	}

	/** Named values that build up with repeated additions and decay over time. Recipe parameters can read them. */
	UPROPERTY(Config, EditAnywhere, Category = "Playback", meta = (TitleProperty = "Name"))
	TArray<FFeelAccumulatorDefinition> Accumulators;

	// FEELKIT_PRO_BEGIN
	/**
	 * Feel Maps used by Send Feel Event for every target. A Feel Trigger component on the target can add maps that are
	 * checked first.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Events")
	TArray<TSoftObjectPtr<UFeelMap>> FeelMaps;
	// FEELKIT_PRO_END

	/** Comfort scales new players start with. Also the Default preset. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort")
	FFeelComfortScales DefaultComfortScales;

	/** Which comfort group scales each channel. Child tags are included; the most specific tag wins. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort")
	TArray<FFeelChannelComfortMapping> ChannelComfortGroups;

	/** Scales applied by the Reduced Motion preset. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Presets")
	FFeelComfortScales ReducedMotionPreset;

	/** Scales applied by the Reduced Flashing preset. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Presets")
	FFeelComfortScales ReducedFlashingPreset;

	/** Scales applied by the No Haptics preset. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Presets")
	FFeelComfortScales NoHapticsPreset;

	/**
	 * Menu opened by Show Feel Comfort Menu when the node is given no menu class. To restyle the menu, copy
	 * WBP_FeelComfortMenu into your project (Copy to Project) and select the copy here.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort", meta = (MetaClass = "/Script/FeelCore.FeelComfortMenu"))
	FSoftClassPath ComfortMenuClass;

	/**
	 * Applies each player's Camera Shake comfort scale (and Master) to camera shakes the game plays through the engine,
	 * not only to FeelKit recipes, so one comfort setting reduces every shake.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort")
	bool bApplyComfortToEngineCameraShakes = true;

	/**
	 * Applies each player's Haptics comfort scale (and Master) to all controller vibration through the player controller's
	 * force feedback scale, not only to FeelKit recipes.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort")
	bool bApplyComfortToEngineForceFeedback = true;

	/** Save automatically whenever a player's comfort settings change. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Storage")
	bool bAutoSaveComfort = true;

	/** Save slot prefix for the default storage. The local player index is appended. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Storage")
	FString ComfortSaveSlotPrefix = TEXT("FeelComfort");

	/** Storage for comfort settings, implementing Feel Comfort Storage. Empty uses one save game slot per local player. */
	UPROPERTY(Config, EditAnywhere, Category = "Comfort|Storage", meta = (MustImplement = "/Script/FeelCore.FeelComfortStorage"))
	TSoftClassPtr<UObject> ComfortStorageClass;

	/** Scales of a built-in preset. */
	FFeelComfortScales GetPresetScales(EFeelBuiltInComfortPreset Preset) const;

	/** Accumulator definition with this name, or null. */
	const FFeelAccumulatorDefinition* FindAccumulator(FName AccumulatorName) const;

	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
