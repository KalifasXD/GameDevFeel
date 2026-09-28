// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Steps/FeelStep_ShapedMotion.h"
#include "FeelStep_HitFlash.generated.h"

class UMaterialInterface;

/**
 * Flashes the target's meshes with a color, drawn on top of them through the overlay material slot. Works on any mesh,
 * whatever its own materials are, so characters flash without preparing their materials. The flash material is a
 * translucent overlay material with a FlashColor vector parameter and a FlashAmount scalar parameter (0 to 1).
 */
UCLASS(meta = (DisplayName = "Hit Flash"))
class FEELCORE_API UFeelStep_HitFlash : public UFeelStep_ShapedMotion
{
	GENERATED_BODY()

public:
	UFeelStep_HitFlash();

	/** Overlay material drawn over the meshes. It reads FlashColor (vector) and FlashAmount (scalar, 0 to 1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	TObjectPtr<UMaterialInterface> FlashMaterial = nullptr;

	/** Flash color at the peak. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flash")
	FLinearColor Color = FLinearColor::White;

	virtual void Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
