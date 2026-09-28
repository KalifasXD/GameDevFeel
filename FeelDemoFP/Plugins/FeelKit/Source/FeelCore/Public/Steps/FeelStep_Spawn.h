// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_Spawn.generated.h"

class UMaterialInterface;

/** Where a spawned effect appears. */
UENUM(BlueprintType)
enum class EFeelSpawnLocation : uint8
{
	/** The play context's Location when the play passed one, otherwise the target. */
	PlayLocationOrTarget UMETA(DisplayName = "Play Location or Target"),
	/** Always the target. */
	Target,
};

/**
 * Shows a number or short text that pops up at the target and floats away, such as a score, an amount or a label.
 * The text can come from a recipe parameter. Drawn over the game view of the target's player.
 */
UCLASS(meta = (DisplayName = "Number Pop"))
class FEELCORE_API UFeelStep_NumberPop : public UFeelStep
{
	GENERATED_BODY()

public:
	/**
	 * Parameter whose value is shown. Leave empty to show Text instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FName ValueParameter;

	/** Digits after the decimal point when showing a parameter value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text", meta = (ClampMin = "0", ClampMax = "4"))
	int32 Decimals = 0;

	/** Text shown when no value parameter is set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FText Text;

	/** Added before the text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FString Prefix;

	/** Added after the text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	FString Suffix;

	/** Text color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor Color = FLinearColor::White;

	/** Font size at intensity 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (ClampMin = "6", ClampMax = "200"))
	float FontSize = 28.0f;

	/** Grow the text with the track's intensity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	bool bScaleWithIntensity = true;

	/** Where the text appears. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	EFeelSpawnLocation Location = EFeelSpawnLocation::PlayLocationOrTarget;

	/** World offset from that location, in centimeters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
	FVector WorldOffset = FVector(0.0, 0.0, 60.0);

	/** How far the text floats up on screen over its lifetime, in slate units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0"))
	float RiseDistance = 60.0f;

	/** Random sideways spread, in slate units, so repeated pops do not stack. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0"))
	float Spread = 24.0f;

	/** Size at the moment the text appears, relative to its settled size. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.1", ClampMax = "4"))
	float PopScale = 1.6f;

	/** Seconds the text stays when the track has no length. Tracks with a length use it instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion", meta = (ClampMin = "0.1", Units = "Seconds"))
	float DefaultLifetime = 1.0f;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool SupportsPreview_Implementation() const override { return false; }
	virtual bool RequiresDuration() const override { return false; }
};

/** Spawns a decal at the target or the play location, oriented by the play context's Normal or the surface found there. */
UCLASS(meta = (DisplayName = "Spawn Decal"))
class FEELCORE_API UFeelStep_SpawnDecal : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Decal material (Deferred Decal material domain). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	TObjectPtr<UMaterialInterface> DecalMaterial;

	/** Decal size: X is projection depth, Y and Z the width and height. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	FVector DecalSize = FVector(16.0, 48.0, 48.0);

	/** Where the decal appears. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	EFeelSpawnLocation Location = EFeelSpawnLocation::PlayLocationOrTarget;

	/** Seconds before the decal starts fading. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (ClampMin = "0", Units = "Seconds"))
	float Lifetime = 5.0f;

	/** Fade-out length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (ClampMin = "0", Units = "Seconds"))
	float FadeOutTime = 1.0f;

	/** Turn each decal by a random angle around the normal, so repeated decals do not line up. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bRandomRotation = true;

	/**
	 * Looks for a surface from the spawn point along the direction the decal projects (down, unless the play gives a
	 * Normal), and places the decal on it with that surface's orientation. Useful because actor locations are usually
	 * above the ground: a character's location is the middle of its capsule. Off places the decal exactly at the spawn
	 * point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bFindSurface = true;

	/** How far to look for a surface, beyond the edge of the target (for a character: below its feet). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal", meta = (ClampMin = "0", Units = "Centimeters", EditCondition = "bFindSurface"))
	float SurfaceSearchDistance = 500.0f;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool RequiresDuration() const override { return false; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
