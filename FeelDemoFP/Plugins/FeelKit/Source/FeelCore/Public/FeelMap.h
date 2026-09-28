// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FeelMap.generated.h"

class FDataValidationContext;
class UFeelRecipe;

/** One row of a Feel Map: which recipe plays for an event, optionally only in some circumstances. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelMapEntry
{
	GENERATED_BODY()

	/** Event this row answers. Also answers child events: a row for A.B answers A.B.C when no row is more specific. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entry")
	FGameplayTag Event;

	/** Tags the play context must all contain for this row to apply. Empty applies in every circumstance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entry")
	FGameplayTagContainer RequiredTags;

	/** Recipe to play. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entry")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Multiplies the intensity of the event when this row plays. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entry", meta = (ClampMin = "0"))
	float IntensityScale = 1.0f;

	/** Breaks ties between rows that match equally specifically. Higher wins. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entry")
	int32 Priority = 0;
};

/**
 * Decides which recipe plays for a named event. Gameplay sends an event tag with a play context; the map picks the row
 * that matches most specifically, so variants for different circumstances need no branching in Blueprint.
 * Matching: the row's event must match the sent event (same tag or a parent), and the context must contain all of the
 * row's required tags. Among matches, a more specific event wins, then more required tags, then higher priority, then
 * the earlier row.
 */
UCLASS(BlueprintType)
class FEELCORE_API UFeelMap : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Rows of this map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Feel Map", meta = (TitleProperty = "Event"))
	TArray<FFeelMapEntry> Entries;

	/**
	 * Best row for an event across several maps. Maps earlier in the list win ties with later maps.
	 * Returns null when nothing matches.
	 */
	static const FFeelMapEntry* FindBestEntry(TConstArrayView<const UFeelMap*> Maps, const FGameplayTag& Event, const FGameplayTagContainer& ContextTags);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
