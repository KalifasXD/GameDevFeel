// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_Meta.generated.h"

class UFeelRecipe;

/**
 * Plays another recipe inside this track. The inner recipe starts when the track starts and plays while the
 * track lasts, so give the track at least the inner recipe's length. It uses the same target, context and parameters,
 * and its tracks keep their own channels and comfort. Previews exactly like the inner recipe.
 */
UCLASS(meta = (DisplayName = "Play Recipe"))
class FEELCORE_API UFeelStep_Recipe : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Recipe to play inside this track. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
	TObjectPtr<UFeelRecipe> Recipe;

	/** Multiplies the inner recipe's intensity, on top of this track's intensity curve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe", meta = (ClampMin = "0"))
	float IntensityScale = 1.0f;

	/** Deepest nesting of recipes inside recipes. Deeper tracks are skipped, which also stops a recipe from playing itself forever. */
	static constexpr int32 MaxNestingDepth = 4;

	virtual FGameplayTag GetDefaultChannel_Implementation() const override;
	virtual bool UsesConstantIntensityByDefault() const override { return true; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};

/** One option of a random choice. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelRandomChoiceOption
{
	GENERATED_BODY()

	/** Step played when this option is chosen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Option")
	TObjectPtr<UFeelStep> Step;

	/** Relative chance of this option. An option with weight 2 is picked twice as often as one with weight 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Option", meta = (ClampMin = "0"))
	float Weight = 1.0f;
};

/**
 * Plays one of several steps, picked at random for each play. The pick is fixed for the whole play, so
 * scrubbing shows the same option, and every new play picks again.
 */
UCLASS(meta = (DisplayName = "Random Choice"))
class FEELCORE_API UFeelStep_RandomChoice : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Steps to choose from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
	TArray<FFeelRandomChoiceOption> Options;

	/** The option picked for a roll in [0, 1), by weight. Null when there are no usable options. */
	UFeelStep* ChooseOption(float Roll) const;

	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif
};
