// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelRecipe.h"
#include "FeelRecipeJson.h"
#include "FeelTags.h"
#include "Steps/FeelStep_Meta.h"
#include "Steps/FeelStep_ProceduralShake.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeJsonTest, "FeelKit.Editor.RecipeJson", FEEL_TEST_FLAGS)
bool FFeelRecipeJsonTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelRecipe> Source(NewObject<UFeelRecipe>(GetTransientPackage()));
	Source->Cooldown = 0.25f;
	Source->SchemaVersion = UFeelRecipe::CurrentSchemaVersion;
	Source->bSustain = true;
	Source->SustainStart = 0.1f;
	Source->SustainEnd = 0.4f;
	{
		FFeelRecipeParameter& Parameter = Source->Parameters.AddDefaulted_GetRef();
		Parameter.Name = TEXT("Strength");
		Parameter.DefaultValue = 0.5f;
		Parameter.MaxValue = 2.0f;
	}
	{
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Source.Get());
		Flash->Color = FLinearColor(0.2f, 0.4f, 0.6f);
		Flash->MaxOpacity = 0.35f;
		FFeelTrack& Track = Source->Tracks.AddDefaulted_GetRef();
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.StartTime = 0.05f;
		Track.Duration = 0.3f;
		Track.Conditions.Chance = 0.75f;
		Track.IntensityCurve.GetRichCurve()->Reset();
		Track.IntensityCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
		Track.IntensityCurve.GetRichCurve()->AddKey(1.0f, 0.0f);
		Track.ParameterMappings.AddDefaulted_GetRef().Parameter = TEXT("Strength");
	}
	{
		// A step with nested instanced steps.
		UFeelStep_RandomChoice* Choice = NewObject<UFeelStep_RandomChoice>(Source.Get());
		FFeelRandomChoiceOption& First = Choice->Options.AddDefaulted_GetRef();
		UFeelStep_ProceduralShake* Shake = NewObject<UFeelStep_ProceduralShake>(Choice);
		Shake->Frequency = 17.0f;
		First.Step = Shake;
		First.Weight = 3.0f;
		FFeelRandomChoiceOption& Second = Choice->Options.AddDefaulted_GetRef();
		Second.Step = NewObject<UFeelStep_ScreenFlash>(Choice);
		FFeelTrack& Track = Source->Tracks.AddDefaulted_GetRef();
		Track.Step = Choice;
		Track.Duration = 0.5f;
	}

#if WITH_EDITORONLY_DATA
	Source->Feeling = FeelTags::Feeling_Impact;
	Source->Genres.AddTag(FeelTags::Genre_Action);
	Source->Description = FText::FromString(TEXT("A sharp hit."));
	Source->BasedOn = FSoftObjectPath(TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit"));
#endif

	const FString Json = FFeelRecipeJson::Export(*Source);
	TestTrue(TEXT("The file names its format"), Json.Contains(FFeelRecipeJson::FormatName));
	TestFalse(TEXT("Where a copy came from is not exported"), Json.Contains(TEXT("FR_Impact_HeavyHit")));

	TStrongObjectPtr<UFeelRecipe> Target(NewObject<UFeelRecipe>(GetTransientPackage()));
	Target->Tracks.AddDefaulted();
	FText Error;
	if (!TestTrue(TEXT("Import succeeds"), FFeelRecipeJson::Import(*Target, Json, Error)))
	{
		AddError(Error.ToString());
		return false;
	}

	TestEqual(TEXT("Round trip gives identical JSON"), FFeelRecipeJson::Export(*Target).Replace(*Target->GetName(), TEXT("")), Json.Replace(*Source->GetName(), TEXT("")));
	if (TestEqual(TEXT("Existing tracks are replaced"), Target->Tracks.Num(), 2))
	{
		const UFeelStep_ScreenFlash* Flash = Cast<UFeelStep_ScreenFlash>(Target->Tracks[0].Step);
		TestTrue(TEXT("Step class and values restored"), Flash && FMath::IsNearlyEqual(Flash->MaxOpacity, 0.35f));
		TestTrue(TEXT("Steps are new objects inside the imported recipe"), Flash && Flash != Source->Tracks[0].Step && Flash->GetOuter() == Target.Get());
		TestEqual(TEXT("Curve keys restored"), Target->Tracks[0].IntensityCurve.GetRichCurveConst()->GetNumKeys(), 2);
		TestEqual(TEXT("Conditions restored"), Target->Tracks[0].Conditions.Chance, 0.75f);

		const UFeelStep_RandomChoice* Choice = Cast<UFeelStep_RandomChoice>(Target->Tracks[1].Step);
		if (TestTrue(TEXT("Nested options restored"), Choice && Choice->Options.Num() == 2))
		{
			const UFeelStep_ProceduralShake* Shake = Cast<UFeelStep_ProceduralShake>(Choice->Options[0].Step);
			TestTrue(TEXT("Nested step values restored"), Shake && FMath::IsNearlyEqual(Shake->Frequency, 17.0f));
			TestTrue(TEXT("Nested steps are owned by the imported recipe"), Shake && Shake != Cast<UFeelStep_RandomChoice>(Source->Tracks[1].Step)->Options[0].Step && Shake->IsIn(Target.Get()));
		}
	}
#if WITH_EDITORONLY_DATA
	TestTrue(TEXT("Feeling restored"), Target->Feeling == FeelTags::Feeling_Impact);
	TestTrue(TEXT("Genres restored"), Target->Genres.HasTagExact(FeelTags::Genre_Action));
	TestEqual(TEXT("Description restored"), Target->Description.ToString(), FString(TEXT("A sharp hit.")));
	TestTrue(TEXT("The target keeps its own Based On"), Target->BasedOn.IsNull());

	// A file written before library metadata existed still imports, leaving the fields empty.
	TStrongObjectPtr<UFeelRecipe> Older(NewObject<UFeelRecipe>(GetTransientPackage()));
	const FString OlderJson = TEXT("{\"format\":\"FeelKitRecipe\",\"schemaVersion\":1,\"recipe\":{\"cooldown\":0.5}}");
	if (TestTrue(TEXT("A schema 1 file imports"), FFeelRecipeJson::Import(*Older, OlderJson, Error)))
	{
		TestEqual(TEXT("Its values are read"), Older->Cooldown, 0.5f);
		TestFalse(TEXT("It has no feeling"), Older->Feeling.IsValid());
		TestEqual(TEXT("Importing raises its schema version"), Older->SchemaVersion, UFeelRecipe::CurrentSchemaVersion);
	}
#endif

	TestTrue(TEXT("Sustain restored"), Target->bSustain && FMath::IsNearlyEqual(Target->SustainEnd, 0.4f));
	TestEqual(TEXT("Parameters restored"), Target->Parameters.Num(), 1);

	// A reference to an asset this project does not have is reported, and the rest of the recipe still imports.
	{
		TStrongObjectPtr<UFeelRecipe> WithMissing(NewObject<UFeelRecipe>(GetTransientPackage()));
		const FString MissingJson = TEXT("{\"format\":\"FeelKitRecipe\",\"schemaVersion\":2,\"recipe\":{\"cooldown\":0.25,\"tracks\":[{\"step\":{\"_ClassName\":\"/Script/FeelCore.FeelStep_PlaySound\",\"sound\":\"/FeelKit/Samples/Sounds/S_FK_DoesNotExist.S_FK_DoesNotExist\"},\"duration\":0.4}]}}");
		TArray<FString> MissingAssets;
		if (TestTrue(TEXT("A recipe with a missing asset still imports"), FFeelRecipeJson::Import(*WithMissing, MissingJson, Error, &MissingAssets)))
		{
			TestEqual(TEXT("The track is there"), WithMissing->Tracks.Num(), 1);
			TestEqual(TEXT("The missing reference is reported"), MissingAssets.Num(), 1);
			TestTrue(TEXT("It names the asset"), MissingAssets.Num() == 1 && MissingAssets[0].Contains(TEXT("S_FK_DoesNotExist")));
		}
	}

	// Bad input leaves the recipe untouched.
	const FString Before = FFeelRecipeJson::Export(*Target);
	TestFalse(TEXT("Not JSON is rejected"), FFeelRecipeJson::Import(*Target, TEXT("{ not json"), Error));
	TestFalse(TEXT("Other JSON is rejected"), FFeelRecipeJson::Import(*Target, TEXT("{\"format\":\"Something\",\"recipe\":{}}"), Error));
	const FString Newer = Json.Replace(*FString::Printf(TEXT("\"schemaVersion\": %d"), UFeelRecipe::CurrentSchemaVersion), TEXT("\"schemaVersion\": 999"));
	TestFalse(TEXT("A newer format is rejected"), FFeelRecipeJson::Import(*Target, Newer, Error));
	TestTrue(TEXT("The newer format error says to update"), Error.ToString().Contains(TEXT("newer")));
	TestEqual(TEXT("Rejected imports change nothing"), FFeelRecipeJson::Export(*Target), Before);
	return true;
}

#endif
