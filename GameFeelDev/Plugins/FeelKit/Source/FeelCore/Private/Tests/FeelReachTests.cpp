// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelRecipe.h"
#include "FeelReplicationComponent.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "GameFramework/Actor.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelReachTests
{
	class FScopedWorld
	{
	public:
		FScopedWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* Subsystem() const { return World->GetSubsystem<UFeelSubsystem>(); }

		UWorld* World = nullptr;
	};

	UFeelRecipe* MakeFlashRecipe()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 1.0f;
		return Recipe;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelNetModesTest, "FeelKit.Network.ModesAndPacking", FEEL_TEST_FLAGS)
bool FFeelNetModesTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Dedicated server never plays"), FeelNet::ShouldPlay(EFeelNetMode::Everyone, NM_DedicatedServer, true));
	TestTrue(TEXT("Everyone plays on a client"), FeelNet::ShouldPlay(EFeelNetMode::Everyone, NM_Client, false));
	TestTrue(TEXT("Owner only plays on the owner"), FeelNet::ShouldPlay(EFeelNetMode::OwnerOnly, NM_Client, true));
	TestFalse(TEXT("Owner only skips others"), FeelNet::ShouldPlay(EFeelNetMode::OwnerOnly, NM_ListenServer, false));
	TestFalse(TEXT("Skip owner skips the owner"), FeelNet::ShouldPlay(EFeelNetMode::SkipOwner, NM_Client, true));
	TestTrue(TEXT("Skip owner plays on others"), FeelNet::ShouldPlay(EFeelNetMode::SkipOwner, NM_ListenServer, false));

	FFeelPlayContext Context;
	Context.Parameters.Add(TEXT("Damage"), 42.0f);
	Context.Parameters.Add(TEXT("Speed"), 3.0f);
	Context.Direction = FVector(10.0, 0.0, 0.0);
	Context.Location = FVector(100.0, 200.0, 300.0);
	Context.Normal = FVector(0.0, 0.0, 5.0);
	Context.ContextTags.AddTag(FeelTags::Screen_Flash);

	FFeelTarget WidgetTarget;
	WidgetTarget.Type = EFeelTargetType::Widget;
	const FFeelNetPlay Play = FFeelNetPlay::Make(nullptr, FeelTags::Screen_Flash, WidgetTarget, 0.5f, Context, EFeelNetMode::SkipOwner, -10.0f);
	TestTrue(TEXT("Widget targets are local UI and are not sent"), Play.Target.Type == EFeelTargetType::None);
	TestEqual(TEXT("Negative relevancy distance means no limit"), Play.RelevancyDistance, 0.0f);
	TestTrue(TEXT("Mode kept"), Play.Mode == EFeelNetMode::SkipOwner);

	const FFeelPlayContext Unpacked = Play.ToContext();
	TestEqual(TEXT("Parameters survive packing"), Unpacked.Parameters.Num(), 2);
	TestEqual(TEXT("Parameter value"), Unpacked.Parameters.FindRef(TEXT("Damage")), 42.0f);
	TestTrue(TEXT("Direction is sent normalized"), Unpacked.Direction.Equals(FVector(1.0, 0.0, 0.0)));
	TestTrue(TEXT("Normal is sent normalized"), Unpacked.Normal.Equals(FVector(0.0, 0.0, 1.0)));
	TestTrue(TEXT("Location kept"), Unpacked.Location.Equals(Context.Location));
	TestTrue(TEXT("Tags kept"), Unpacked.ContextTags.HasTagExact(FeelTags::Screen_Flash));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelReplicationStandaloneTest, "FeelKit.Network.StandaloneComponent", FEEL_TEST_FLAGS)
bool FFeelReplicationStandaloneTest::RunTest(const FString& Parameters)
{
	using namespace FeelReachTests;

	FScopedWorld TestWorld;
	TStrongObjectPtr<UFeelRecipe> Recipe(MakeFlashRecipe());
	AActor* Actor = TestWorld.World->SpawnActor<AActor>();
	UFeelReplicationComponent* Component = NewObject<UFeelReplicationComponent>(Actor);
	Component->RegisterComponent();
	TestTrue(TEXT("The component replicates by default"), Component->GetIsReplicated());

	const FFeelHandle Everyone = Component->PlayFeelNetworked(Recipe.Get(), FFeelTarget::FromActor(Actor), 1.0f, FFeelPlayContext(), EFeelNetMode::Everyone);
	TestTrue(TEXT("Single player: Everyone plays here"), TestWorld.Subsystem()->IsPlaying(Everyone));

	const FFeelHandle OwnerOnly = Component->PlayFeelNetworked(Recipe.Get(), FFeelTarget::FromActor(Actor), 1.0f, FFeelPlayContext(), EFeelNetMode::OwnerOnly);
	TestTrue(TEXT("Single player: this machine owns everything, so Owner Only plays"), TestWorld.Subsystem()->IsPlaying(OwnerOnly));

	const FFeelHandle SkipOwner = Component->PlayFeelNetworked(Recipe.Get(), FFeelTarget::FromActor(Actor), 1.0f, FFeelPlayContext(), EFeelNetMode::SkipOwner);
	TestFalse(TEXT("Single player: Skip Owner plays nothing"), SkipOwner.IsValid());

	const FFeelHandle Far = Component->PlayFeelNetworked(Recipe.Get(), FFeelTarget::AtLocation(FVector(1.0e6, 0.0, 0.0)), 1.0f, FFeelPlayContext(), EFeelNetMode::Everyone, 100.0f);
	TestTrue(TEXT("Without local cameras the distance is unknown and the play passes"), Far.IsValid());

	TestFalse(TEXT("No recipe: nothing plays"), Component->PlayFeelNetworked(nullptr, FFeelTarget(), 1.0f, FFeelPlayContext()).IsValid());
	TestFalse(TEXT("No event tag: nothing plays"), Component->SendFeelEventNetworked(FGameplayTag(), FFeelTarget(), 1.0f, FFeelPlayContext()).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelNetTimePolicyTest, "FeelKit.Network.GlobalTimeDilationPolicy", FEEL_TEST_FLAGS)
bool FFeelNetTimePolicyTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UFeelSettings> Settings(NewObject<UFeelSettings>(GetTransientPackage()));
	Settings->bAllowGlobalTimeDilationInMultiplayer = false;
	TestFalse(TEXT("Single player uses world time"), Settings->ShouldLocalizeGlobalTimeDilation(NM_Standalone));
	TestTrue(TEXT("Clients slow only the play's actor"), Settings->ShouldLocalizeGlobalTimeDilation(NM_Client));
	TestTrue(TEXT("Listen servers slow only the play's actor"), Settings->ShouldLocalizeGlobalTimeDilation(NM_ListenServer));

	Settings->bAllowGlobalTimeDilationInMultiplayer = true;
	TestFalse(TEXT("Opt-in restores world time in multiplayer"), Settings->ShouldLocalizeGlobalTimeDilation(NM_Client));
	return true;
}

#endif
