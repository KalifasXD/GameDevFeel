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
	 * released (Release Feel, the end of an anim notify state, or the Release Parameter below), then plays the rest of the
	 * recipe and the release recipe below, if any. Stop Feel ends it at once.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain")
	bool bSustain = false;

	/** Start of the looping region, in seconds from the recipe start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (ClampMin = "0", Units = "Seconds", EditCondition = "bSustain"))
	float SustainStart = 0.0f;

	/** End of the looping region, in seconds from the recipe start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (ClampMin = "0", Units = "Seconds", EditCondition = "bSustain"))
	float SustainEnd = 1.0f;

	/**
	 * When the play is released, jump straight to Sustain End and play the ending at once, instead of finishing the
	 * current loop first. Use it when the ending answers the release, such as a charged attack letting go. Tracks that
	 * would have ended before Sustain End end at the jump; tracks that run past Sustain End carry on. Always on when
	 * On Full Release or On Early Release is set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (EditCondition = "bSustain"))
	bool bJumpToEndOnRelease = false;

	/**
	 * Parameter that releases the play on its own when it reaches Release At, as Release Feel would. It is read every
	 * frame, so values from Set Feel Parameter and from accumulators count. Leave empty to release only from the game:
	 * Release Feel, the end of an anim notify state, a Feel Input binding or a gameplay cue.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (EditCondition = "bSustain", GetOptions = "GetParameterNames"))
	FName ReleaseParameter;

	/** Point in the Release Parameter's range where the play releases itself: 0 is the parameter's min, 1 its max. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (EditCondition = "bSustain", ClampMin = "0", ClampMax = "1"))
	float ReleaseAt = 1.0f;

	/**
	 * Recipe that plays from Sustain End when the Release Parameter releases the play, such as the burst of a fully charged
	 * attack. All its tracks play on this play's target, with this play's intensity and parameter values. Shown on the
	 * timeline after Sustain End.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (DisplayName = "On Full Release", EditCondition = "bSustain"))
	TObjectPtr<UFeelRecipe> FullReleaseRecipe;

	/**
	 * Recipe that plays from Sustain End when the game releases the play before the Release Parameter reaches Release At
	 * (Release Feel, the end of an anim notify state), such as a charge that fizzles out. Without a Release Parameter, every
	 * release plays it.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sustain", meta = (DisplayName = "On Early Release", EditCondition = "bSustain"))
	TObjectPtr<UFeelRecipe> EarlyReleaseRecipe;

	/** Shortest usable sustain region. */
	static constexpr float MinSustainLength = 0.01f;

	/** Version of the recipe data format, written on save. 0 for recipes saved before versioning. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "Recipe")
	int32 SchemaVersion = 0;

	/**
	 * Current recipe data format. 1: parameters, play context and random ranges. 2: library metadata. 3: release by
	 * parameter, jump to end on release and the release recipes.
	 */
	static constexpr int32 CurrentSchemaVersion = 3;

	/**
	 * Nominal length of the recipe: the latest track end time, without random duration variation. With release recipes,
	 * the longer of the two counts from Sustain End.
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

	/** Latest track end time of this recipe's own tracks, without release recipes or random duration variation. */
	float GetTracksLength() const;

	/** Whether the recipe has a usable sustain region and at least one release recipe. */
	bool HasReleaseRecipes() const;

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

	/** The library recipe this one was copied from, with Copy to Project, the Recipe Browser or Recipe from Template. */
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
