// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "AssetRegistry/AssetData.h"
#include "CoreMinimal.h"

/** Where a recipe comes from. */
enum class EFeelRecipeSource : uint8
{
	/** Recipes of the FeelKit library and of this project. */
	Both,
	/** Only the recipes that ship with FeelKit. */
	Library,
	/** Only the recipes of this project. */
	Project,
};

/**
 * What the recipe browser is filtering by. Empty sets mean "no filter in this group". Feelings and genres describe what a
 * recipe is, so several of them match any of them; channels describe what a recipe does, so several of them must all be
 * present.
 */
struct FFeelRecipeFilterState
{
	EFeelRecipeSource Source = EFeelRecipeSource::Both;

	/** Feeling tag names, for example Feel.Feeling.Impact. */
	TSet<FName> Feelings;

	/** Genre tag names, for example Feel.Genre.Shooter. */
	TSet<FName> Genres;

	/** Channel group roots, for example Feel.Camera. A recipe must have a channel in **every** chosen group. */
	TSet<FName> ChannelGroups;

	/** Matched against name, description and parameter names, case-insensitively. */
	FString Search;
};

/** One recipe as the browser lists it, read from the asset registry without loading the recipe. */
struct FFeelRecipeEntry
{
	FAssetData Asset;
	FName Feeling;
	TArray<FName> Genres;
	TArray<FName> Channels;
	FString Description;
	TArray<FString> Parameters;
	float Length = 0.0f;
	int32 TrackCount = 0;
	bool bSustained = false;
	bool bLibrary = false;

	FString GetName() const { return Asset.AssetName.ToString(); }
};

namespace FeelRecipeFilter
{
	/** The channel groups the browser offers, in display order. */
	extern const TArray<FName> ChannelGroups;

	/** Reads one recipe's registry tags. */
	FFeelRecipeEntry MakeEntry(const FAssetData& Asset);

	/** Every recipe in the project and the library, read from the asset registry. */
	TArray<FFeelRecipeEntry> GatherAll();

	/** The entries matching the filter, sorted by feeling then name. */
	TArray<FFeelRecipeEntry> Apply(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State);

	/** How many recipes each feeling has with every filter except the feeling selection applied. */
	TMap<FName, int32> CountByFeeling(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State);
}
