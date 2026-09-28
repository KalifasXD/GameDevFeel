// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelRecipe.h"
#include "FeelTags.h"
#include "Steps/FeelStep_ForceFeedbackCurve.h"
#include "Steps/FeelStep_Meta.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/AssetRegistryTagsContext.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelLibraryMetadataTests
{
	FFeelTrack& AddTrack(UFeelRecipe& Recipe, UFeelStep* Step, float Start, float Duration)
	{
		FFeelTrack& Track = Recipe.Tracks.AddDefaulted_GetRef();
		Track.Step = Step;
		Track.Channel = Step ? Step->GetDefaultChannel() : FGameplayTag();
		Track.StartTime = Start;
		Track.Duration = Duration;
		return Track;
	}

	FString FindTag(const UObject& Object, FName Name)
	{
		FAssetRegistryTagsContextData Data(&Object, EAssetRegistryTagsCaller::Uncategorized);
		Object.GetAssetRegistryTags(Data);
		const UObject::FAssetRegistryTag* Found = Data.Tags.Find(Name);
		return Found ? Found->Value : TEXT("<missing>");
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryMetadataTest, "FeelKit.Library.Metadata", FEEL_TEST_FLAGS)
bool FFeelLibraryMetadataTest::RunTest(const FString& Parameters)
{
	using namespace FeelLibraryMetadataTests;

	TStrongObjectPtr<UFeelRecipe> Inner(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddTrack(*Inner, NewObject<UFeelStep_PlaySound>(Inner.Get()), 0.0f, 0.3f);

	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	AddTrack(*Recipe, NewObject<UFeelStep_ProceduralShake>(Recipe.Get()), 0.0f, 0.5f);

	// A muted track does not count as a channel of the recipe.
	FFeelTrack& Muted = AddTrack(*Recipe, NewObject<UFeelStep_ScreenFlash>(Recipe.Get()), 0.0f, 0.2f);
	Muted.Channel = FeelTags::Screen_Distortion;
	Muted.bEnabled = false;

	UFeelStep_Recipe* Nested = NewObject<UFeelStep_Recipe>(Recipe.Get());
	Nested->Recipe = Inner.Get();
	AddTrack(*Recipe, Nested, 0.2f, 0.4f);

	UFeelStep_RandomChoice* Choice = NewObject<UFeelStep_RandomChoice>(Recipe.Get());
	Choice->Options.AddDefaulted_GetRef().Step = NewObject<UFeelStep_ScreenFlash>(Choice);
	Choice->Options.AddDefaulted_GetRef().Step = NewObject<UFeelStep_ForceFeedbackCurve>(Choice);
	AddTrack(*Recipe, Choice, 0.5f, 1.0f);

	TSet<FGameplayTag> Channels;
	Recipe->GatherChannels(Channels);
	TestTrue(TEXT("Track channels are gathered"), Channels.Contains(FeelTags::Camera_Shake));
	TestTrue(TEXT("Channels of a nested recipe are gathered"), Channels.Contains(FeelTags::Audio));
	TestTrue(TEXT("Channels of random choice options are gathered"), Channels.Contains(FeelTags::Screen_Flash) && Channels.Contains(FeelTags::Haptics));
	TestFalse(TEXT("Muted tracks are ignored"), Channels.Contains(FeelTags::Screen_Distortion));

	// A recipe that plays itself must not recurse forever.
	UFeelStep_Recipe* Self = NewObject<UFeelStep_Recipe>(Inner.Get());
	Self->Recipe = Inner.Get();
	AddTrack(*Inner, Self, 0.0f, 0.1f);
	TSet<FGameplayTag> InnerChannels;
	Inner->GatherChannels(InnerChannels);
	TestTrue(TEXT("A self-nesting recipe still reports its channels"), InnerChannels.Contains(FeelTags::Audio));

	Recipe->bSustain = true;
	TestEqual(TEXT("Track count tag"), FindTag(*Recipe, UFeelRecipe::TrackCountTagName), FString(TEXT("4")));
	TestEqual(TEXT("Length tag"), FindTag(*Recipe, UFeelRecipe::LengthTagName), FString(TEXT("1.50")));
	TestEqual(TEXT("Sustained tag"), FindTag(*Recipe, UFeelRecipe::SustainedTagName), FString(TEXT("True")));
	const FString ChannelsTag = FindTag(*Recipe, UFeelRecipe::ChannelsTagName);
	TestTrue(TEXT("Channels tag lists the channels"), ChannelsTag.Contains(TEXT("Feel.Camera.Shake")) && ChannelsTag.Contains(TEXT("Feel.Haptics")) && !ChannelsTag.Contains(TEXT("Feel.Screen.Distortion")));

#if WITH_EDITORONLY_DATA
	Recipe->Feeling = FeelTags::Feeling_Impact;
	Recipe->Genres.AddTag(FeelTags::Genre_Action);
	Recipe->Genres.AddTag(FeelTags::Genre_Shooter);
	Recipe->Description = FText::FromString(TEXT("A sharp hit."));
	TestEqual(TEXT("Feeling tag"), FindTag(*Recipe, UFeelRecipe::FeelingTagName), FString(TEXT("Feel.Feeling.Impact")));
	const FString GenresTag = FindTag(*Recipe, UFeelRecipe::GenresTagName);
	TestTrue(TEXT("Genres tag lists every genre"), GenresTag.Contains(TEXT("Feel.Genre.Action")) && GenresTag.Contains(TEXT("Feel.Genre.Shooter")));
	TestEqual(TEXT("Description tag"), FindTag(*Recipe, UFeelRecipe::DescriptionTagName), FString(TEXT("A sharp hit.")));
#endif

	TestEqual(TEXT("Schema version 2 marks library metadata"), UFeelRecipe::CurrentSchemaVersion, 2);
	return true;
}

#endif
