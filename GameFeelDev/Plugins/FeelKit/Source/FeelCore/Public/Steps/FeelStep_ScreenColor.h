// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_ScreenColor.generated.h"

class UMaterialInterface;

/** Drains color from the scene and brings it back. */
UCLASS(meta = (DisplayName = "Desaturate"))
class FEELCORE_API UFeelStep_Desaturate : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_Desaturate();

	/** How much color is removed at the peak: 1 is grayscale. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float Amount = 0.8f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};

/** Tints the whole scene toward a color and back. */
UCLASS(meta = (DisplayName = "Color Tint"))
class FEELCORE_API UFeelStep_ColorTint : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_ColorTint();

	/** Color the scene is multiplied toward. White leaves it unchanged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FLinearColor TintColor = FLinearColor(1.0f, 0.4f, 0.35f);

	/** How far the scene moves toward the tint at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color", meta = (ClampMin = "0", ClampMax = "1"))
	float Strength = 1.0f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
};

/** Fades the view to a color, holds, and fades back, drawn under flashes. */
UCLASS(meta = (DisplayName = "Screen Fade"))
class FEELCORE_API UFeelStep_ScreenFade : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Color faded to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade")
	FLinearColor FadeColor = FLinearColor::Black;

	/** Opacity while held. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxOpacity = 1.0f;

	/** Portion of the track spent fading in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ClampMin = "0", ClampMax = "1"))
	float FadeInFraction = 0.3f;

	/** Portion of the track spent fading out. 0 stays faded until the track ends. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade", meta = (ClampMin = "0", ClampMax = "1"))
	float FadeOutFraction = 0.3f;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }
};

/**
 * Blends a post-process material over the scene and back, for effects the engine has no setting for, such as radial blur,
 * scanlines or heat haze. The material must use the Post Process material domain.
 */
UCLASS(meta = (DisplayName = "Post Process Material Pulse"))
class FEELCORE_API UFeelStep_PostProcessMaterialPulse : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_PostProcessMaterialPulse();

	/** Post-process material to blend in. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	TObjectPtr<UMaterialInterface> Material;

	/** Blend weight at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (ClampMin = "0", ClampMax = "1"))
	float MaxWeight = 1.0f;

	/**
	 * Scalar parameter of the material that receives the current weight (0 to 1), so the material can fade its own effect,
	 * for example as the Alpha of a Lerp from the scene (Scene Texture: PostProcessInput0) to the effect. Unreal shows a
	 * post-process material at full strength whenever it is active, so a material without this parameter appears at full
	 * strength for the whole track. Set to None to blend the material directly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FName WeightParameter = TEXT("Weight");

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
