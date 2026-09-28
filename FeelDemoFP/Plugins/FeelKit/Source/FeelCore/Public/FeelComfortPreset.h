// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FeelComfortTypes.h"
#include "FeelComfortPreset.generated.h"

/** A custom comfort preset players can apply. */
UCLASS(BlueprintType)
class FEELCORE_API UFeelComfortPreset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Name shown to players, for example in a settings menu. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comfort")
	FText DisplayName;

	/** Scales this preset applies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comfort")
	FFeelComfortScales Scales;
};
