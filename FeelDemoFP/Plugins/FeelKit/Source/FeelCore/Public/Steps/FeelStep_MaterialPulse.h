// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_MaterialPulse.generated.h"

/** How pulsed parameters reach the target's materials. */
UENUM(BlueprintType)
enum class EFeelMaterialPulseRoute : uint8
{
	/** Dynamic material instances on the target's meshes. Works with any material parameter. */
	MaterialInstance UMETA(DisplayName = "Material Instance"),
	/** Custom primitive data. No material instances and better batching; the material must read the parameter as custom primitive data. */
	CustomPrimitiveData UMETA(DisplayName = "Custom Primitive Data"),
};

/** One material parameter to pulse. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelMaterialPulseParameter
{
	GENERATED_BODY()

	/** Name of the material parameter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	FName ParameterName = TEXT("Color");

	/** Blend a color parameter toward Color instead of offsetting a scalar parameter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	bool bIsColor = true;

	/** Added to the scalar parameter's original value at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (EditCondition = "!bIsColor", EditConditionHides))
	float ScalarAmount = 1.0f;

	/** Color reached at the peak, blended from the parameter's original color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material", meta = (EditCondition = "bIsColor", EditConditionHides))
	FLinearColor Color = FLinearColor(1.0f, 0.15f, 0.1f);
};

/** Pulses material parameters on the target, such as a hit flash or an emissive glow, then restores them. */
UCLASS(meta = (DisplayName = "Material Parameter Pulse"))
class FEELCORE_API UFeelStep_MaterialPulse : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_MaterialPulse();

	/** Parameters pulsed together. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	TArray<FFeelMaterialPulseParameter> Parameters;

	/** How the parameters reach the materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	EFeelMaterialPulseRoute Route = EFeelMaterialPulseRoute::MaterialInstance;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
