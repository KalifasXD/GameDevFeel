// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelRecipe.h"
#include "FeelRecipeFilter.h"
#include "FeelTags.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelRecipeFilterTests
{
	/** An entry as the browser would read it from the asset registry. */
	FFeelRecipeEntry MakeEntry(const TCHAR* PackagePath, const TCHAR* AssetName, const FGameplayTag& Feeling,
		TArray<FName> Genres, TArray<FName> Channels, const FString& Description, TArray<FString> ParameterNames)
	{
		FFeelRecipeEntry Entry;
		Entry.Asset = FAssetData(FName(PackagePath), FName(*FString::Printf(TEXT("%s/%s"), PackagePath, AssetName)), FName(AssetName), UFeelRecipe::StaticClass()->GetClassPathName());
		Entry.Feeling = Feeling.IsValid() ? Feeling.GetTagName() : NAME_None;
		Entry.Genres = MoveTemp(Genres);
		Entry.Channels = MoveTemp(Channels);
		Entry.Description = Description;
		Entry.Parameters = MoveTemp(ParameterNames);
		Entry.bLibrary = FString(PackagePath).StartsWith(TEXT("/FeelKit/Library/"));
		return Entry;
	}

	TArray<FString> NamesOf(const TArray<FFeelRecipeEntry>& Entries)
	{
		TArray<FString> Names;
		for (const FFeelRecipeEntry& Entry : Entries)
		{
			Names.Add(Entry.GetName());
		}
		return Names;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeFilterTest, "FeelKit.Editor.RecipeFilter", FEEL_TEST_FLAGS)
bool FFeelRecipeFilterTest::RunTest(const FString& Parameters)
{
	using namespace FeelRecipeFilterTests;

	const TArray<FFeelRecipeEntry> All =
	{
		MakeEntry(TEXT("/FeelKit/Library/Impact"), TEXT("FR_Impact_HeavyHit"), FeelTags::Feeling_Impact,
			{ FGameplayTag(FeelTags::Genre_Action).GetTagName(), FGameplayTag(FeelTags::Genre_Shooter).GetTagName() },
			{ FGameplayTag(FeelTags::Camera_Shake).GetTagName(), FGameplayTag(FeelTags::Haptics).GetTagName() },
			TEXT("A heavy blow that lands."), { TEXT("Damage") }),
		MakeEntry(TEXT("/FeelKit/Library/Impact"), TEXT("FR_Impact_LightHit"), FeelTags::Feeling_Impact,
			{ FGameplayTag(FeelTags::Genre_Action).GetTagName() },
			{ FGameplayTag(FeelTags::Camera_Shake).GetTagName() },
			TEXT("A quick tap."), {}),
		MakeEntry(TEXT("/FeelKit/Library/Reward"), TEXT("FR_Reward_Pickup"), FeelTags::Feeling_Reward,
			{ FGameplayTag(FeelTags::Genre_Platformer).GetTagName() },
			{ FGameplayTag(FeelTags::Audio).GetTagName(), FGameplayTag(FeelTags::UI).GetTagName() },
			TEXT("Collecting something good."), { TEXT("Combo") }),
		MakeEntry(TEXT("/Game/MyRecipes"), TEXT("R_MyOwnHit"), FeelTags::Feeling_Impact,
			{ FGameplayTag(FeelTags::Genre_Shooter).GetTagName() },
			{ FGameplayTag(FeelTags::Screen_Flash).GetTagName() },
			TEXT("Project recipe."), { TEXT("Strength") }),
		MakeEntry(TEXT("/Game/MyRecipes"), TEXT("R_Untagged"), FGameplayTag(), {}, {}, TEXT(""), {}),
	};

	FFeelRecipeFilterState State;
	TestEqual(TEXT("No filter lists everything"), FeelRecipeFilter::Apply(All, State).Num(), 5);

	// Sorting: by feeling, then by name; recipes without a feeling come last.
	const TArray<FString> Sorted = NamesOf(FeelRecipeFilter::Apply(All, State));
	TestEqual(TEXT("Impact recipes come first, alphabetically"), Sorted[0], FString(TEXT("FR_Impact_HeavyHit")));
	TestEqual(TEXT("Then the rest of the Impact recipes"), Sorted[1], FString(TEXT("FR_Impact_LightHit")));
	TestEqual(TEXT("Recipes without a feeling come last"), Sorted.Last(), FString(TEXT("R_Untagged")));

	State.Source = EFeelRecipeSource::Library;
	TestEqual(TEXT("Library only"), FeelRecipeFilter::Apply(All, State).Num(), 3);
	State.Source = EFeelRecipeSource::Project;
	TestEqual(TEXT("Project only"), FeelRecipeFilter::Apply(All, State).Num(), 2);

	State = FFeelRecipeFilterState();
	State.Feelings.Add(FGameplayTag(FeelTags::Feeling_Impact).GetTagName());
	TestEqual(TEXT("One feeling"), FeelRecipeFilter::Apply(All, State).Num(), 3);
	State.Feelings.Add(FGameplayTag(FeelTags::Feeling_Reward).GetTagName());
	TestEqual(TEXT("Two feelings match either"), FeelRecipeFilter::Apply(All, State).Num(), 4);

	State.Genres.Add(FGameplayTag(FeelTags::Genre_Shooter).GetTagName());
	TestEqual(TEXT("Feeling and genre must both match"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Impact_HeavyHit"), TEXT("R_MyOwnHit") }));

	State = FFeelRecipeFilterState();
	State.ChannelGroups.Add(FName(TEXT("Feel.Camera")));
	TestEqual(TEXT("A channel group matches its channels"), FeelRecipeFilter::Apply(All, State).Num(), 2);
	State.ChannelGroups.Add(FName(TEXT("Feel.Haptics")));
	TestEqual(TEXT("Two channels mean the recipe must do both"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Impact_HeavyHit") }));
	State.ChannelGroups.Add(FName(TEXT("Feel.UI")));
	TestEqual(TEXT("A recipe missing one of the three is left out"), FeelRecipeFilter::Apply(All, State).Num(), 0);
	State.ChannelGroups.Reset();
	State.ChannelGroups.Add(FName(TEXT("Feel.UI")));
	TestEqual(TEXT("A group that is a whole channel matches too"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Reward_Pickup") }));
	State.ChannelGroups.Reset();
	State.ChannelGroups.Add(FName(TEXT("Feel.Haptics")));
	TestEqual(TEXT("Haptics matches the recipe that rumbles"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Impact_HeavyHit") }));

	State = FFeelRecipeFilterState();
	State.Search = TEXT("heavy");
	TestEqual(TEXT("Search matches the name, ignoring case"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Impact_HeavyHit") }));
	State.Search = TEXT("collecting");
	TestEqual(TEXT("Search matches the description"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Reward_Pickup") }));
	State.Search = TEXT("Damage");
	TestEqual(TEXT("Search matches a parameter name"), NamesOf(FeelRecipeFilter::Apply(All, State)), TArray<FString>({ TEXT("FR_Impact_HeavyHit") }));
	State.Search = TEXT("nothing here");
	TestEqual(TEXT("Search with no match lists nothing"), FeelRecipeFilter::Apply(All, State).Num(), 0);

	// Counts ignore the feeling selection but respect every other filter.
	State = FFeelRecipeFilterState();
	State.Feelings.Add(FGameplayTag(FeelTags::Feeling_Reward).GetTagName());
	TMap<FName, int32> Counts = FeelRecipeFilter::CountByFeeling(All, State);
	TestEqual(TEXT("Impact is still counted while Reward is selected"), Counts.FindRef(FGameplayTag(FeelTags::Feeling_Impact).GetTagName()), 3);
	TestEqual(TEXT("Reward count"), Counts.FindRef(FGameplayTag(FeelTags::Feeling_Reward).GetTagName()), 1);
	TestEqual(TEXT("Recipes without a feeling are counted too"), Counts.FindRef(NAME_None), 1);

	State.Source = EFeelRecipeSource::Library;
	Counts = FeelRecipeFilter::CountByFeeling(All, State);
	TestEqual(TEXT("Counts respect the source filter"), Counts.FindRef(FGameplayTag(FeelTags::Feeling_Impact).GetTagName()), 2);
	TestFalse(TEXT("Untagged project recipes are not counted in the library"), Counts.Contains(NAME_None));

	return true;
}

#endif
