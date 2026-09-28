// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeFilter.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "Modules/ModuleManager.h"

namespace FeelRecipeFilter
{
	const TArray<FName> ChannelGroups =
	{
		FName(TEXT("Feel.Camera")),
		FName(TEXT("Feel.Screen")),
		FName(TEXT("Feel.Actor")),
		FName(TEXT("Feel.Time")),
		FName(TEXT("Feel.Audio")),
		FName(TEXT("Feel.Haptics")),
		FName(TEXT("Feel.UI")),
		FName(TEXT("Feel.Spawn")),
	};

	namespace Private
	{
		void SplitNames(const FString& Value, TArray<FName>& OutNames)
		{
			TArray<FString> Parts;
			Value.ParseIntoArray(Parts, TEXT(","));
			for (const FString& Part : Parts)
			{
				const FString Trimmed = Part.TrimStartAndEnd();
				if (!Trimmed.IsEmpty())
				{
					OutNames.Add(FName(*Trimmed));
				}
			}
		}

		FString GetTag(const FAssetData& Asset, FName TagName)
		{
			FString Value;
			Asset.GetTagValue(TagName, Value);
			return Value;
		}
	}

	FFeelRecipeEntry MakeEntry(const FAssetData& Asset)
	{
		using namespace Private;

		FFeelRecipeEntry Entry;
		Entry.Asset = Asset;
		Entry.Feeling = FName(*GetTag(Asset, UFeelRecipe::FeelingTagName));
		SplitNames(GetTag(Asset, UFeelRecipe::GenresTagName), Entry.Genres);
		SplitNames(GetTag(Asset, UFeelRecipe::ChannelsTagName), Entry.Channels);
		Entry.Description = GetTag(Asset, UFeelRecipe::DescriptionTagName);

		TArray<FName> ParameterNames;
		SplitNames(GetTag(Asset, UFeelRecipe::ParametersTagName), ParameterNames);
		for (const FName& Name : ParameterNames)
		{
			Entry.Parameters.Add(Name.ToString());
		}

		Entry.Length = FCString::Atof(*GetTag(Asset, UFeelRecipe::LengthTagName));
		Entry.TrackCount = FCString::Atoi(*GetTag(Asset, UFeelRecipe::TrackCountTagName));
		Entry.bSustained = GetTag(Asset, UFeelRecipe::SustainedTagName) == TEXT("True");
		Entry.bLibrary = FeelLibrary::IsLibraryAsset(Asset);

		// A feeling written as None means the recipe has none.
		if (Entry.Feeling == NAME_None || Entry.Feeling.ToString() == TEXT("None"))
		{
			Entry.Feeling = NAME_None;
		}
		return Entry;
	}

	TArray<FFeelRecipeEntry> GatherAll()
	{
		FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FAssetData> Assets;
		AssetRegistry.Get().GetAssetsByClass(UFeelRecipe::StaticClass()->GetClassPathName(), Assets, true);

		TArray<FFeelRecipeEntry> Entries;
		Entries.Reserve(Assets.Num());
		for (const FAssetData& Asset : Assets)
		{
			Entries.Add(MakeEntry(Asset));
		}
		return Entries;
	}

	/** Whether one entry passes every group of the filter, optionally ignoring the feeling group (for the counts). */
	static bool Passes(const FFeelRecipeEntry& Entry, const FFeelRecipeFilterState& State, bool bIgnoreFeelings)
	{
		if (State.Source == EFeelRecipeSource::Library && !Entry.bLibrary)
		{
			return false;
		}
		if (State.Source == EFeelRecipeSource::Project && Entry.bLibrary)
		{
			return false;
		}

		if (!bIgnoreFeelings && State.Feelings.Num() > 0 && !State.Feelings.Contains(Entry.Feeling))
		{
			return false;
		}

		if (State.Genres.Num() > 0 && !Entry.Genres.ContainsByPredicate([&State](const FName& Genre) { return State.Genres.Contains(Genre); }))
		{
			return false;
		}

		// Channels are things a recipe does, so picking two means "does both": a recipe that shakes and rumbles.
		for (const FName& Group : State.ChannelGroups)
		{
			const FString GroupName = Group.ToString();
			const bool bHasGroup = Entry.Channels.ContainsByPredicate([&GroupName](const FName& Channel)
			{
				const FString ChannelName = Channel.ToString();
				return ChannelName == GroupName || ChannelName.StartsWith(GroupName + TEXT("."));
			});
			if (!bHasGroup)
			{
				return false;
			}
		}

		if (!State.Search.IsEmpty())
		{
			const FString Search = State.Search;
			const bool bMatches = Entry.GetName().Contains(Search)
				|| Entry.Description.Contains(Search)
				|| Entry.Parameters.ContainsByPredicate([&Search](const FString& Parameter) { return Parameter.Contains(Search); });
			if (!bMatches)
			{
				return false;
			}
		}

		return true;
	}

	TArray<FFeelRecipeEntry> Apply(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State)
	{
		TArray<FFeelRecipeEntry> Result;
		for (const FFeelRecipeEntry& Entry : All)
		{
			if (Passes(Entry, State, false))
			{
				Result.Add(Entry);
			}
		}

		Result.Sort([](const FFeelRecipeEntry& A, const FFeelRecipeEntry& B)
		{
			if (A.Feeling != B.Feeling)
			{
				// Recipes without a feeling come last.
				if (A.Feeling.IsNone() != B.Feeling.IsNone())
				{
					return B.Feeling.IsNone();
				}
				return A.Feeling.ToString() < B.Feeling.ToString();
			}
			return A.GetName() < B.GetName();
		});
		return Result;
	}

	TMap<FName, int32> CountByFeeling(const TArray<FFeelRecipeEntry>& All, const FFeelRecipeFilterState& State)
	{
		TMap<FName, int32> Counts;
		for (const FFeelRecipeEntry& Entry : All)
		{
			if (Passes(Entry, State, true))
			{
				++Counts.FindOrAdd(Entry.Feeling);
			}
		}
		return Counts;
	}
}
