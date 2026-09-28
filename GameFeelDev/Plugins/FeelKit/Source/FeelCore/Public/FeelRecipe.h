// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FeelTrack.h"
#include "FeelRecipe.generated.h"

class FDataValidationContext;

/** A reusable piece of game feel: timed tracks of steps. */
UCLASS(BlueprintType)
class FEELCORE_API UFeelRecipe : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Tracks of this recipe. Edited in the recipe timeline. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
	TArray<FFeelTrack> Tracks;

	/** Seconds before this recipe can play again on the same target. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0", Units = "Seconds"))
	float Cooldown = 0.0f;

	/** Maximum simultaneous instances per target. 0 means unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0"))
	int32 MaxConcurrent = 0;

	/** Multiplier applied to every play of this recipe. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe", meta = (ClampMin = "0"))
	float DefaultIntensity = 1.0f;

	/**
	 * Named numbers the game can pass each time the recipe plays. Tracks can scale their intensity by them through
	 * parameter mappings, and the recipe editor shows a slider for each to preview the range. A parameter named Distance
	 * is filled in automatically when the game does not pass it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters", meta = (TitleProperty = "Name"))
	TArray<FFeelRecipeParameter> Parameters;

	/**
	 * Keeps the recipe playing while a condition lasts: time loops between Sustain Start and Sustain End until the play is
	 * released (Release Feel, the end of an anim notify state, or Stop Feel), then plays the rest of the recipe.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain")
	bool bSustain = false;

	/** Start of the looping region, in seconds from the recipe start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (ClampMin = "0", Units = "Seconds", EditCondition = "bSustain"))
	float SustainStart = 0.0f;

	/** End of the looping region, in seconds from the recipe start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (ClampMin = "0", Units = "Seconds", EditCondition = "bSustain"))
	float SustainEnd = 1.0f;

	/** Shortest usable sustain region. */
	static constexpr float MinSustainLength = 0.01f;

	/** Version of the recipe data format, written on save. 0 for recipes saved before versioning. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Recipe")
	int32 SchemaVersion = 0;

	/** Current recipe data format. 1: parameters, play context and random ranges. 2: library metadata. */
	static constexpr int32 CurrentSchemaVersion = 2;

	/**
	 * Nominal length of the recipe: the latest track end time, without random duration variation.
	 * @return	Length in seconds.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel")
	float GetDuration() const;

	/** Declared parameter with this name, or null. */
	const FFeelRecipeParameter* FindParameter(FName ParameterName) const;

	/** Names of the declared parameters, for the parameter pickers of track mappings. */
	UFUNCTION()
	TArray<FName> GetParameterNames() const;

	/** Names of the accumulators in the project settings, for the accumulator picker of parameters. */
	UFUNCTION()
	TArray<FName> GetAccumulatorNames() const;

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

	/** Channels this recipe produces: the channels of its enabled tracks, including the tracks of nested recipes and the options of random choices. */
	void GatherChannels(TSet<FGameplayTag>& OutChannels) const;

private:
	void GatherChannels(TSet<FGameplayTag>& OutChannels, int32 Depth) const;

public:

	/** Asset registry tag listing the declared parameter names, comma separated. */
	static const FName ParametersTagName;

	/** Asset registry tags the recipe browser and the Content Browser tooltip read without loading the recipe. */
	static const FName FeelingTagName;
	static const FName GenresTagName;
	static const FName DescriptionTagName;
	static const FName ChannelsTagName;
	static const FName TrackCountTagName;
	static const FName LengthTagName;
	static const FName SustainedTagName;

#if WITH_EDITORONLY_DATA
	/**
	 * What this recipe makes the player feel. It groups the recipe browser and the recipe library, for example
	 * Feel.Feeling.Impact. Projects can add their own tags under Feel.Feeling. Not included in cooked builds.
	 */
	UPROPERTY(EditAnywhere, Category = "Library", meta = (Categories = "Feel.Feeling"))
	FGameplayTag Feeling;

	/** Kinds of game this recipe suits, for example Feel.Genre.Shooter. Used by the recipe browser filters. */
	UPROPERTY(EditAnywhere, Category = "Library", meta = (Categories = "Feel.Genre"))
	FGameplayTagContainer Genres;

	/** One or two sentences describing the recipe, shown in the recipe browser and the Content Browser tooltip. */
	UPROPERTY(EditAnywhere, Category = "Library", meta = (MultiLine = true))
	FText Description;

	/** The library recipe this one was copied from, when it was created with Recipe from Template. */
	UPROPERTY(VisibleAnywhere, Category = "Library")
	FSoftObjectPath BasedOn;

	/** Mesh shown in the recipe editor preview. Static or skeletal; empty shows a cube. Not included in cooked builds. */
	UPROPERTY(EditAnywhere, Category = "Preview", meta = (AllowedClasses = "/Script/Engine.StaticMesh,/Script/Engine.SkeletalMesh"))
	TSoftObjectPtr<UObject> PreviewMesh;

	bool HasSoloTracks() const;
#endif

#if WITH_EDITOR
	/** Readable names and tooltips for the recipe's asset registry tags, used by the Content Browser tooltip and columns. */
	virtual void GetAssetRegistryTagMetadata(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const override;

	/** Missing steps and assets, zero-length tracks, curve problems and essential tracks without a fallback. */
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
