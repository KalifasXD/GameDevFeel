// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FeelComfortTypes.generated.h"

/** Comfort groups players can scale separately. */
UENUM(BlueprintType)
enum class EFeelComfortGroup : uint8
{
	/** Not part of a group; only Master applies. */
	None,
	/** Camera shakes. */
	CameraShake UMETA(DisplayName = "Camera Shake"),
	/** Camera punches, FOV kicks, roll and zoom. */
	CameraMotion UMETA(DisplayName = "Camera Motion"),
	/** Full-screen flashes. */
	Flashes,
	/** Hitstops and slow motion. */
	HitstopAndSlowMo UMETA(DisplayName = "Hitstop and Slow-mo"),
	/** Vignette, chromatic aberration and other distortion. */
	ScreenDistortion UMETA(DisplayName = "Screen Distortion"),
	/** Controller vibration. */
	Haptics,
};

/** Built-in comfort presets. Their values are set in Project Settings > Plugins > FeelKit. */
UENUM(BlueprintType)
enum class EFeelBuiltInComfortPreset : uint8
{
	/** The project's default scales. */
	Default,
	/** Less camera shake, camera motion and distortion. */
	ReducedMotion UMETA(DisplayName = "Reduced Motion"),
	/** Weaker flashes. */
	ReducedFlashing UMETA(DisplayName = "Reduced Flashing"),
	/** No controller vibration. */
	NoHaptics UMETA(DisplayName = "No Haptics"),
};

/** What the flash limiter does with flashes above the allowed rate. */
UENUM(BlueprintType)
enum class EFeelFlashLimitMode : uint8
{
	/** Extra flashes play at reduced strength. */
	Soften,
	/** Extra flashes do not play. */
	Suppress,
};

/** A player's comfort scales, each from 0 (off) to 1 (full strength), and related comfort options. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelComfortScales
{
	GENERATED_BODY()

	/** Scales every effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float Master = 1.0f;

	/** Scales camera shakes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float CameraShake = 1.0f;

	/** Scales camera punches, FOV kicks, roll and zoom. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float CameraMotion = 1.0f;

	/** Scales full-screen flashes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float Flashes = 1.0f;

	/** Scales hitstops and slow motion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (DisplayName = "Hitstop and Slow-mo", ClampMin = "0", ClampMax = "1"))
	float HitstopAndSlowMo = 1.0f;

	/** Scales vignette, chromatic aberration and other distortion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float ScreenDistortion = 1.0f;

	/** Scales controller vibration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort", meta = (ClampMin = "0", ClampMax = "1"))
	float Haptics = 1.0f;

	/**
	 * Limits how many flashes can start in a short time. Counts FeelKit tracks on channels mapped to the Flashes
	 * group. A mitigation helper, not a photosensitivity certification: test your game with an analysis tool.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Limiter")
	bool bLimitFlashes = true;

	/** Flashes allowed per second before the limiter acts. Values below 1 allow one flash per 1 / value seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Limiter", meta = (ClampMin = "0.1", ClampMax = "30", EditCondition = "bLimitFlashes"))
	float MaxFlashesPerSecond = 3.0f;

	/** What happens to flashes above the allowed rate. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Limiter", meta = (EditCondition = "bLimitFlashes"))
	EFeelFlashLimitMode FlashLimitMode = EFeelFlashLimitMode::Soften;

	/** Strength of softened flashes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash Limiter", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bLimitFlashes && FlashLimitMode == EFeelFlashLimitMode::Soften"))
	float SoftenedFlashScale = 0.3f;

	/** Allows FeelKit to roll the camera. Off removes camera roll from every recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion")
	bool bAllowCameraRoll = true;

	/** Fastest field of view change FeelKit may cause, in degrees per second. 0 is unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0", Units = "Degrees"))
	float MaxFieldOfViewChangePerSecond = 0.0f;

	/** Scale of one group, not including Master. Returns 1 for None. */
	float GetGroupScale(EFeelComfortGroup Group) const;

	/** Sets one group's scale, clamped to 0 to 1. Does nothing for None. */
	void SetGroupScale(EFeelComfortGroup Group, float Scale);

	/** Clamps every scale to 0 to 1. */
	void ClampScales();

	bool operator==(const FFeelComfortScales& Other) const;
	bool operator!=(const FFeelComfortScales& Other) const { return !(*this == Other); }
};

/** Assigns a channel tag, and all tags below it, to a comfort group. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelChannelComfortMapping
{
	GENERATED_BODY()

	FFeelChannelComfortMapping() = default;
	FFeelChannelComfortMapping(const FGameplayTag& InChannel, EFeelComfortGroup InGroup)
		: Channel(InChannel)
		, Group(InGroup)
	{
	}

	/** Channel tag, for example Feel.Camera.Shake. Child tags are included. */
	UPROPERTY(EditAnywhere, Category = "Comfort", meta = (Categories = "Feel"))
	FGameplayTag Channel;

	/** Comfort group that scales this channel. */
	UPROPERTY(EditAnywhere, Category = "Comfort")
	EFeelComfortGroup Group = EFeelComfortGroup::None;
};

namespace FeelComfort
{
	/** Group of a channel. When several mappings match, the most specific tag wins. */
	FEELCORE_API EFeelComfortGroup FindGroup(const FGameplayTag& Channel, TConstArrayView<FFeelChannelComfortMapping> Mappings);

	/**
	 * Camera Shake and Camera Motion. For these groups the player's setting always wins: an essential floor never
	 * raises them, so turning shake or camera motion off means no motion at all (Xbox Accessibility camera comfort).
	 */
	FEELCORE_API bool IsMotionGroup(EFeelComfortGroup Group);

	/**
	 * Motion comfort on camera output: removes roll when not allowed, and limits how fast the field of
	 * view offset changes. InOutPreviousFieldOfView carries the last applied offset between frames.
	 */
	FEELCORE_API void ApplyMotionComfort(const FFeelComfortScales& Scales, FRotator& InOutRotationOffset, float& InOutFieldOfViewOffset, float& InOutPreviousFieldOfView, float DeltaSeconds);
}

/**
 * Flash limiter state for one player: remembers when recent flashes started and decides how strong
 * a new one may be. Shared by the runtime and the editor preview.
 */
struct FEELCORE_API FFeelFlashLimiter
{
	/** Registers a flash starting at Now (seconds) and returns its strength multiplier: 1, the softened scale, or 0. */
	float RegisterFlash(double Now, const FFeelComfortScales& Scales);

	void Reset() { RecentFlashes.Reset(); }

private:
	TArray<double, TInlineAllocator<8>> RecentFlashes;
};

/** Comfort applied during one evaluation. Without scales, evaluation is neutral. */
struct FEELCORE_API FFeelComfortContext
{
	const FFeelComfortScales* Scales = nullptr;
	TConstArrayView<FFeelChannelComfortMapping> Mappings;

	/** Channel group scale x Master. 1 when no scales are set. */
	float GetScale(const FGameplayTag& Channel) const;

	/** Comfort group of a channel under these mappings. */
	EFeelComfortGroup GetGroup(const FGameplayTag& Channel) const;
};
