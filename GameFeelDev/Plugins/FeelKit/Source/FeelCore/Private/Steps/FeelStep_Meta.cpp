// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_Meta.h"

#include "FeelRecipe.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_Meta)

FGameplayTag UFeelStep_Recipe::GetDefaultChannel_Implementation() const
{
	return FeelTags::Meta_Recipe;
}

#if WITH_EDITOR
void UFeelStep_Recipe::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!Recipe)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "RecipeStepMissingRecipe", "Play Recipe has no recipe."));
	}
	else if (Recipe == GetTypedOuter<UFeelRecipe>())
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "RecipeStepSelf", "Play Recipe plays the recipe it is in."));
	}
}
#endif

UFeelStep* UFeelStep_RandomChoice::ChooseOption(float Roll) const
{
	float TotalWeight = 0.0f;
	for (const FFeelRandomChoiceOption& Option : Options)
	{
		TotalWeight += Option.Step ? FMath::Max(Option.Weight, 0.0f) : 0.0f;
	}
	if (TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Remaining = FMath::Clamp(Roll, 0.0f, 0.999999f) * TotalWeight;
	UFeelStep* LastUsable = nullptr;
	for (const FFeelRandomChoiceOption& Option : Options)
	{
		const float Weight = Option.Step ? FMath::Max(Option.Weight, 0.0f) : 0.0f;
		if (Weight <= 0.0f)
		{
			continue;
		}
		LastUsable = Option.Step;
		if (Remaining < Weight)
		{
			return Option.Step;
		}
		Remaining -= Weight;
	}
	return LastUsable;
}

FGameplayTag UFeelStep_RandomChoice::GetDefaultChannel_Implementation() const
{
	for (const FFeelRandomChoiceOption& Option : Options)
	{
		if (Option.Step)
		{
			return Option.Step->GetDefaultChannel();
		}
	}
	return FeelTags::Meta_Recipe;
}

#if WITH_EDITOR
void UFeelStep_RandomChoice::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	const bool bAnyUsable = Options.ContainsByPredicate([](const FFeelRandomChoiceOption& Option) { return Option.Step && Option.Weight > 0.0f; });
	if (!bAnyUsable)
	{
		OutWarnings.Add(NSLOCTEXT("FeelKit", "RandomChoiceEmpty", "Random Choice has no option with a step and a weight above 0, so it plays nothing."));
	}
	for (const FFeelRandomChoiceOption& Option : Options)
	{
		if (Option.Step)
		{
			Option.Step->ValidateStep(OutErrors, OutWarnings);
		}
	}
}
#endif
