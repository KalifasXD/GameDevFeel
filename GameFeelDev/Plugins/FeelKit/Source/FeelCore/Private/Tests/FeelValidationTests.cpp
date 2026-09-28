// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "FeelRecipe.h"
#include "FeelTags.h"
#include "Misc/DataValidation.h"
#include "Sound/SoundWave.h"
#include "Steps/FeelStep_BlueprintEvent.h"
#include "Steps/FeelStep_ForceFeedbackCurve.h"
#include "Steps/FeelStep_MaterialPulse.h"
#include "Steps/FeelStep_PlaySound.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelValidationTests
{
	struct FValidationOutcome
	{
		EDataValidationResult Result = EDataValidationResult::NotValidated;
		uint32 Errors = 0;
		uint32 Warnings = 0;
	};

	FValidationOutcome Validate(const UFeelRecipe* Recipe)
	{
		FDataValidationContext Context;
		FValidationOutcome Outcome;
		Outcome.Result = Recipe->IsDataValid(Context);
		Outcome.Errors = Context.GetNumErrors();
		Outcome.Warnings = Context.GetNumWarnings();
		return Outcome;
	}

	/** A recipe with one flash track that has no problems. */
	UFeelRecipe* MakeValidRecipe()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 0.5f;
		return Recipe;
	}

	/** Replaces the first track's step and channel. */
	void SetStep(UFeelRecipe* Recipe, UFeelStep* Step, const FGameplayTag& Channel)
	{
		Recipe->Tracks[0].Step = Step;
		Recipe->Tracks[0].Channel = Channel;
	}

	void Expect(FAutomationTestBase& Test, const TCHAR* What, const UFeelRecipe* Recipe, uint32 ExpectedErrors, uint32 ExpectedWarnings)
	{
		const FValidationOutcome Outcome = Validate(Recipe);
		Test.TestEqual(FString::Printf(TEXT("%s: errors"), What), Outcome.Errors, ExpectedErrors);
		Test.TestEqual(FString::Printf(TEXT("%s: warnings"), What), Outcome.Warnings, ExpectedWarnings);
		const EDataValidationResult ExpectedResult = ExpectedErrors > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
		Test.TestTrue(FString::Printf(TEXT("%s: result"), What), Outcome.Result == ExpectedResult);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeValidationTest, "FeelKit.Validation.RecipeRules", FEEL_TEST_FLAGS)
bool FFeelRecipeValidationTest::RunTest(const FString& Parameters)
{
	using namespace FeelValidationTests;

	Expect(*this, TEXT("Valid recipe"), MakeValidRecipe(), 0, 0);
	Expect(*this, TEXT("Empty recipe"), NewObject<UFeelRecipe>(GetTransientPackage()), 0, 1);

	UFeelRecipe* Recipe = MakeValidRecipe();
	Recipe->Tracks[0].Step = nullptr;
	Expect(*this, TEXT("Missing step"), Recipe, 1, 0);

	Recipe = MakeValidRecipe();
	Recipe->Tracks[0].Duration = 0.0f;
	Expect(*this, TEXT("Zero length on a step that needs a length"), Recipe, 1, 0);

	Recipe = MakeValidRecipe();
	SetStep(Recipe, NewObject<UFeelStep_PlaySound>(Recipe), FeelTags::Audio);
	Recipe->Tracks[0].Duration = 0.0f;
	Expect(*this, TEXT("Instant Play Sound without a sound"), Recipe, 1, 0);
	Cast<UFeelStep_PlaySound>(Recipe->Tracks[0].Step)->Sound = NewObject<USoundWave>(GetTransientPackage());
	Expect(*this, TEXT("Instant Play Sound with a sound"), Recipe, 0, 0);

	Recipe = MakeValidRecipe();
	Recipe->Tracks[0].IntensityCurve.GetRichCurve()->AddKey(1.5f, 0.5f);
	Expect(*this, TEXT("Curve key outside 0 to 1"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	{
		FRichCurve* Curve = Recipe->Tracks[0].IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->AddKey(0.0f, 1.0f);
		Curve->AddKey(1.0f, -0.5f);
	}
	Expect(*this, TEXT("Curve below 0"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	{
		FRichCurve* Curve = Recipe->Tracks[0].IntensityCurve.GetRichCurve();
		Curve->Reset();
		Curve->AddKey(0.0f, 0.0f);
	}
	Expect(*this, TEXT("Silent curve"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	Recipe->Tracks[0].bEssential = true;
	Expect(*this, TEXT("Essential without substitute or floor"), Recipe, 0, 1);
	Recipe->Tracks[0].EssentialFloor = 0.3f;
	Expect(*this, TEXT("Essential with a floor"), Recipe, 0, 0);

	Recipe = MakeValidRecipe();
	Recipe->Tracks[0].SubstituteStep = NewObject<UFeelStep_ScreenFlash>(Recipe);
	Expect(*this, TEXT("Substitute on a non-essential track"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	Recipe->Tracks[0].Channel = FGameplayTag();
	Expect(*this, TEXT("No channel"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	UFeelStep_MaterialPulse* Pulse = NewObject<UFeelStep_MaterialPulse>(Recipe);
	Pulse->Parameters[0].ParameterName = NAME_None;
	SetStep(Recipe, Pulse, FeelTags::Actor_Material);
	Expect(*this, TEXT("Material parameter without a name"), Recipe, 1, 0);

	Recipe = MakeValidRecipe();
	UFeelStep_ForceFeedbackCurve* Rumble = NewObject<UFeelStep_ForceFeedbackCurve>(Recipe);
	Rumble->LeftLarge = 0.0f;
	Rumble->LeftSmall = 0.0f;
	Rumble->RightLarge = 0.0f;
	Rumble->RightSmall = 0.0f;
	SetStep(Recipe, Rumble, FeelTags::Haptics);
	Expect(*this, TEXT("Force feedback with every motor at 0"), Recipe, 0, 1);

	Recipe = MakeValidRecipe();
	SetStep(Recipe, NewObject<UFeelStep_BlueprintEvent>(Recipe), FeelTags::Meta_Event);
	Recipe->Tracks[0].Duration = 0.0f;
	Expect(*this, TEXT("Instant Blueprint Event without event names"), Recipe, 0, 1);

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
