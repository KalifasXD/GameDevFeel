// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "FeelTrackLifecycle.h"
#include "GameFramework/Actor.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelParameterTests
{
	const FName DamageName(TEXT("Damage"));

	/** Recipe with a Damage parameter (0 to 100, default 50) and one constant flash from 0 to 1 s. */
	UFeelRecipe* MakeDamageRecipe()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelRecipeParameter& Damage = Recipe->Parameters.AddDefaulted_GetRef();
		Damage.Name = DamageName;
		Damage.MinValue = 0.0f;
		Damage.MaxValue = 100.0f;
		Damage.DefaultValue = 50.0f;

		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 1.0f;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Recipe;
	}

	float FlashAt(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params)
	{
		FFeelOutputAccumulator Accumulator;
		FFeelEvaluator::Evaluate(Recipe, Time, Params, Accumulator);
		return Accumulator.Output.FlashAlpha;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterMappingTest, "FeelKit.Parameters.IntensityMapping", FEEL_TEST_FLAGS)
bool FFeelParameterMappingTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeDamageRecipe());
	FFeelEvalParams Params;
	TestEqual(TEXT("Without mappings the parameter does not change the track"), FlashAt(*Recipe, 0.5f, Params), 1.0f, 0.0001f);

	FFeelParameterMapping& Mapping = Recipe->Tracks[0].ParameterMappings.AddDefaulted_GetRef();
	Mapping.Parameter = DamageName;
	TestEqual(TEXT("Default mapping is linear: default 50 of 0..100 gives 0.5"), FlashAt(*Recipe, 0.5f, Params), 0.5f, 0.001f);

	TMap<FName, float> Values;
	Params.ParameterValues = &Values;
	Values.Add(DamageName, 100.0f);
	TestEqual(TEXT("Max value gives full intensity"), FlashAt(*Recipe, 0.5f, Params), 1.0f, 0.001f);
	Values.Add(DamageName, 25.0f);
	TestEqual(TEXT("A play value is used"), FlashAt(*Recipe, 0.5f, Params), 0.25f, 0.001f);
	Values.Add(DamageName, 400.0f);
	TestEqual(TEXT("Values above max are clamped"), FlashAt(*Recipe, 0.5f, Params), 1.0f, 0.001f);
	Values.Add(DamageName, -10.0f);
	TestEqual(TEXT("Values below min are clamped"), FlashAt(*Recipe, 0.5f, Params), 0.0f, 0.001f);

	// A custom curve: light hits keep 30 percent.
	FRichCurve* Curve = Mapping.Curve.GetRichCurve();
	Curve->Reset();
	Curve->SetKeyInterpMode(Curve->AddKey(0.0f, 0.3f), RCIM_Linear);
	Curve->SetKeyInterpMode(Curve->AddKey(1.0f, 1.0f), RCIM_Linear);
	Values.Add(DamageName, 0.0f);
	TestEqual(TEXT("The mapping curve shapes the multiplier"), FlashAt(*Recipe, 0.5f, Params), 0.3f, 0.001f);

	// Mappings combine with the intensity curve, call intensity and recipe default intensity.
	Values.Add(DamageName, 100.0f);
	Params.Intensity = 0.5f;
	Recipe->DefaultIntensity = 0.8f;
	TestEqual(TEXT("Call x recipe default x curve x mapping"), FlashAt(*Recipe, 0.5f, Params), 0.4f, 0.001f);
	Params.Intensity = 1.0f;
	Recipe->DefaultIntensity = 1.0f;

	// Mapping to a parameter the recipe does not declare is ignored.
	Mapping.Parameter = TEXT("Speed");
	TestEqual(TEXT("Undeclared parameters are ignored"), FlashAt(*Recipe, 0.5f, Params), 1.0f, 0.001f);

	TestNotNull(TEXT("FindParameter finds declared parameters"), Recipe->FindParameter(DamageName));
	TestTrue(TEXT("GetParameterNames lists them"), Recipe->GetParameterNames().Contains(DamageName));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterRandomRangesTest, "FeelKit.Parameters.RandomRanges", FEEL_TEST_FLAGS)
bool FFeelParameterRandomRangesTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeDamageRecipe());
	FFeelTrack& Track = Recipe->Tracks[0];
	FFeelEvalParams Params;

	// Random intensity: within range, stable for one play, different across plays.
	Track.RandomIntensity = FFloatInterval(0.5f, 1.0f);
	float Lowest = 2.0f;
	float Highest = -1.0f;
	bool bStable = true;
	for (int32 Seed = 0; Seed < 300; ++Seed)
	{
		Params.InstanceSeed = Seed;
		const float First = FlashAt(*Recipe, 0.2f, Params);
		bStable &= FMath::IsNearlyEqual(First, FlashAt(*Recipe, 0.8f, Params));
		Lowest = FMath::Min(Lowest, First);
		Highest = FMath::Max(Highest, First);
	}
	TestTrue(TEXT("Random intensity stays within the range"), Lowest >= 0.5f - 0.0001f && Highest <= 1.0f + 0.0001f);
	TestTrue(TEXT("Random intensity varies across plays"), Highest - Lowest > 0.3f);
	TestTrue(TEXT("Random intensity is the same during one play"), bStable);
	Track.RandomIntensity = FFloatInterval(1.0f, 1.0f);

	// Random duration: a fixed scale of 2 doubles the track and the recipe.
	Track.RandomDurationScale = FFloatInterval(2.0f, 2.0f);
	Params.InstanceSeed = 7;
	TestEqual(TEXT("Track length for this play is scaled"), FFeelEvaluator::GetTrackDuration(*Recipe, 0, Params), 2.0f, 0.0001f);
	TestEqual(TEXT("Recipe length for this play is scaled"), FFeelEvaluator::GetRecipeDuration(*Recipe, Params), 2.0f, 0.0001f);
	TestEqual(TEXT("Nominal recipe length is unchanged"), Recipe->GetDuration(), 1.0f, 0.0001f);
	TestTrue(TEXT("Track is still active past its nominal end"), FFeelEvaluator::IsTrackActiveAt(*Recipe, 0, 1.5f, Params));
	TestEqual(TEXT("Alpha follows the scaled length"), FFeelEvaluator::GetTrackAlpha(*Recipe, 0, 1.0f, Params), 0.5f, 0.0001f);

	// A range: lengths vary per play but stay within it.
	Track.RandomDurationScale = FFloatInterval(0.5f, 1.5f);
	float ShortestLength = 10.0f;
	float LongestLength = 0.0f;
	for (int32 Seed = 0; Seed < 300; ++Seed)
	{
		Params.InstanceSeed = Seed;
		const float Length = FFeelEvaluator::GetTrackDuration(*Recipe, 0, Params);
		ShortestLength = FMath::Min(ShortestLength, Length);
		LongestLength = FMath::Max(LongestLength, Length);
	}
	TestTrue(TEXT("Random length stays within the range"), ShortestLength >= 0.5f - 0.0001f && LongestLength <= 1.5f + 0.0001f);
	TestTrue(TEXT("Random length varies across plays"), LongestLength - ShortestLength > 0.5f);

	// Intensity and duration rolls are independent of the chance roll.
	int32 DifferentBuckets = 0;
	for (int32 Seed = 0; Seed < 200; ++Seed)
	{
		const bool bChanceLow = FFeelEvaluator::GetChanceRoll(Track, 0, Seed) < 0.5f;
		Params.InstanceSeed = Seed;
		const bool bLengthLow = FFeelEvaluator::GetTrackDuration(*Recipe, 0, Params) < 1.0f;
		DifferentBuckets += bChanceLow != bLengthLow ? 1 : 0;
	}
	TestTrue(TEXT("Random length and chance use different rolls"), DifferentBuckets > 50);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterLifecycleTest, "FeelKit.Parameters.LifecycleUsesPlayLength", FEEL_TEST_FLAGS)
bool FFeelParameterLifecycleTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeDamageRecipe());
	Recipe->Tracks[0].RandomDurationScale = FFloatInterval(2.0f, 2.0f);

	FFeelEvalParams Params;
	FFeelTrackLifecycle Lifecycle;
	Lifecycle.Reset(Recipe->Tracks.Num());
	auto MakeContext = [](int32, float) { return FFeelContext(); };

	Lifecycle.Update(*Recipe, 0.1f, Params, MakeContext);
	TestEqual(TEXT("Track starts"), Lifecycle.GetNumRunning(), 1);
	Lifecycle.Update(*Recipe, 1.5f, Params, MakeContext);
	TestEqual(TEXT("Track keeps running past its nominal end"), Lifecycle.GetNumRunning(), 1);
	Lifecycle.Update(*Recipe, 2.1f, Params, MakeContext);
	TestEqual(TEXT("Track stops at its scaled end"), Lifecycle.GetNumRunning(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterInstigatorFilterTest, "FeelKit.Parameters.InstigatorTrackFilter", FEEL_TEST_FLAGS)
bool FFeelParameterInstigatorFilterTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeDamageRecipe());
	FFeelTrack AttackerTrack = Recipe->Tracks[0];
	AttackerTrack.AppliesTo = EFeelTrackTarget::Instigator;
	Recipe->Tracks.Add(AttackerTrack);

	FFeelOutputAccumulator Accumulator;
	FFeelEvalParams Params;
	TestTrue(TEXT("The recipe has instigator tracks"), FFeelEvaluator::HasInstigatorTracks(*Recipe));
	TestEqual(TEXT("Unfiltered evaluation plays both tracks"), FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Accumulator), 2);

	Params.TargetFilter = EFeelTrackTarget::PlayTarget;
	TestEqual(TEXT("Target filter plays only target tracks"), FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Accumulator), 1);
	Params.TargetFilter = EFeelTrackTarget::Instigator;
	TestEqual(TEXT("Instigator filter plays only instigator tracks"), FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Accumulator), 1);

	Params.TargetFilter.Reset();
	Params.bHasInstigator = false;
	TestEqual(TEXT("Without an instigator, instigator tracks are skipped"), FFeelEvaluator::Evaluate(*Recipe, 0.5f, Params, Accumulator), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterRuntimeContextTest, "FeelKit.Runtime.PlayContext", FEEL_TEST_FLAGS)
bool FFeelParameterRuntimeContextTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	const double PreviousDeltaTime = FApp::GetDeltaTime();
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	auto SpawnActorWithRoot = [World]()
	{
		AActor* Actor = World->SpawnActor<AActor>();
		USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		return Actor;
	};
	auto Step = [World](float Seconds)
	{
		FApp::SetDeltaTime(Seconds);
		World->GetSubsystem<UFeelSubsystem>()->Tick(Seconds);
	};

	UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
	AActor* Victim = SpawnActorWithRoot();
	AActor* Attacker = SpawnActorWithRoot();

	// Victim punch 0.25, attacker punch 0.5 scaled by Damage (0 to 100).
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelRecipeParameter& Damage = Recipe->Parameters.AddDefaulted_GetRef();
	Damage.Name = DamageName;
	Damage.MaxValue = 100.0f;
	Damage.DefaultValue = 100.0f;

	auto AddPunch = [&Recipe](float Amount, EFeelTrackTarget AppliesTo) -> FFeelTrack&
	{
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe.Get());
		Punch->Amount = FVector(Amount);
		Punch->Bounces = 0;
		Track.Step = Punch;
		Track.Duration = 0.4f;
		Track.AppliesTo = AppliesTo;
		Track.IntensityCurve.GetRichCurve()->Reset();
		return Track;
	};
	AddPunch(0.25f, EFeelTrackTarget::PlayTarget);
	AddPunch(0.5f, EFeelTrackTarget::Instigator).ParameterMappings.AddDefaulted_GetRef().Parameter = DamageName;

	FFeelPlayContext Context;
	Context.Instigator = Attacker;
	Context.Parameters.Add(DamageName, 50.0f);
	TestTrue(TEXT("Play with context starts"), Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Victim), 1.0f, Context).IsValid());

	Step(0.2f);
	TestTrue(TEXT("The target track plays on the target"), Victim->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.25), 0.01));
	TestTrue(TEXT("The instigator track plays on the instigator, scaled by Damage 50 of 100"), Attacker->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.25), 0.01));

	Step(0.3f);
	TestTrue(TEXT("Target scale restored"), Victim->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));
	TestTrue(TEXT("Instigator scale restored"), Attacker->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));

	// Without an instigator, the instigator track is skipped and nothing else breaks.
	Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Victim));
	Step(0.2f);
	TestTrue(TEXT("Target still plays without an instigator"), Victim->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.25), 0.01));
	TestTrue(TEXT("Nothing plays on the would-be instigator"), Attacker->GetRootComponent()->GetRelativeScale3D().Equals(FVector(1.0), 0.0001));
	Step(0.3f);

	FApp::SetDeltaTime(PreviousDeltaTime);
	World->RemoveFromRoot();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelParameterValidationTest, "FeelKit.Validation.Parameters", FEEL_TEST_FLAGS)
bool FFeelParameterValidationTest::RunTest(const FString& Parameters)
{
	using namespace FeelParameterTests;

	struct FCounts
	{
		int32 Errors = 0;
		int32 Warnings = 0;
	};
	auto Validate = [](const UFeelRecipe& Recipe)
	{
		FDataValidationContext Context;
		Recipe.IsDataValid(Context);
		FCounts Counts;
		Counts.Errors = Context.GetNumErrors();
		Counts.Warnings = Context.GetNumWarnings();
		return Counts;
	};

	TStrongObjectPtr<UFeelRecipe> Recipe(MakeDamageRecipe());
	FCounts Counts = Validate(*Recipe);
	TestEqual(TEXT("A valid parameter recipe has no errors"), Counts.Errors, 0);
	TestEqual(TEXT("A valid parameter recipe has no warnings"), Counts.Warnings, 0);

	Recipe->Tracks[0].ParameterMappings.AddDefaulted_GetRef().Parameter = TEXT("Speed");
	TestEqual(TEXT("Mapping an undeclared parameter is an error"), Validate(*Recipe).Errors, 1);
	Recipe->Tracks[0].ParameterMappings.Reset();

	const FFeelRecipeParameter Duplicate = Recipe->Parameters[0];
	Recipe->Parameters.Add(Duplicate);
	TestEqual(TEXT("Duplicate parameter names are an error"), Validate(*Recipe).Errors, 1);
	Recipe->Parameters.SetNum(1);

	Recipe->Parameters[0].MaxValue = 0.0f;
	TestEqual(TEXT("Max not above min is an error"), Validate(*Recipe).Errors, 1);
	Recipe->Parameters[0].MaxValue = 100.0f;

	Recipe->Parameters[0].DefaultValue = 150.0f;
	TestEqual(TEXT("Default outside the range is a warning"), Validate(*Recipe).Warnings, 1);
	Recipe->Parameters[0].DefaultValue = 50.0f;

	Recipe->Tracks[0].RandomIntensity = FFloatInterval(1.0f, 0.5f);
	TestEqual(TEXT("Random range with Max below Min is an error"), Validate(*Recipe).Errors, 1);
	Recipe->Tracks[0].RandomIntensity = FFloatInterval(1.0f, 1.0f);

	Recipe->Tracks[0].RandomDurationScale = FFloatInterval(0.0f, 1.0f);
	TestEqual(TEXT("Random duration reaching 0 is a warning"), Validate(*Recipe).Warnings, 1);

	return true;
}
#endif

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
