// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ForceFeedbackCurve.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelForceFeedbackDeliveryTests
{
	/** A minimal game world with a Feel subsystem, destroyed at the end of the scope. */
	class FScopedWorld
	{
	public:
		FScopedWorld()
			: PreviousDeltaTime(FApp::GetDeltaTime())
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			FApp::SetDeltaTime(PreviousDeltaTime);
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* GetSubsystem() const
		{
			return World->GetSubsystem<UFeelSubsystem>();
		}

		/** Advances the subsystem by Seconds of real time. */
		void Step(float Seconds)
		{
			FApp::SetDeltaTime(Seconds);
			if (UFeelSubsystem* Subsystem = GetSubsystem())
			{
				Subsystem->Tick(Seconds);
			}
		}

		UWorld* World = nullptr;

	private:
		double PreviousDeltaTime = 0.0;
	};

	/**
	 * True when the controller holds a dynamic force feedback action with this handle.
	 * Probing clears that action's values until FeelKit's next update sets them again.
	 */
	bool HasDynamicForceFeedback(APlayerController& PlayerController, uint64 Handle)
	{
		return PlayerController.PlayDynamicForceFeedback(0.0f, -1.0f, false, false, false, false, EDynamicForceFeedbackAction::Update, Handle) == Handle;
	}

	int32 CountActions(APlayerController& PlayerController, uint64 FirstHandle, int32 Count)
	{
		int32 Active = 0;
		for (uint64 Handle = FirstHandle; Handle < FirstHandle + Count; ++Handle)
		{
			Active += HasDynamicForceFeedback(PlayerController, Handle) ? 1 : 0;
		}
		return Active;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelForceFeedbackDeliveryTest, "FeelKit.Runtime.ForceFeedbackDelivery", FEEL_TEST_FLAGS)
bool FFeelForceFeedbackDeliveryTest::RunTest(const FString& Parameters)
{
	using namespace FeelForceFeedbackDeliveryTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.GetSubsystem();
	APlayerController* PlayerController = TestWorld.World->SpawnActor<APlayerController>();
	if (!TestNotNull(TEXT("Subsystem"), Subsystem) || !TestNotNull(TEXT("Player controller"), PlayerController))
	{
		return false;
	}
	TestTrue(TEXT("The spawned controller is the first player controller"), TestWorld.World->GetFirstPlayerController() == PlayerController);

	// Learn which handle the engine hands out next.
	const uint64 Baseline = PlayerController->PlayDynamicForceFeedback(0.1f, 1.0f, true, false, false, false, EDynamicForceFeedbackAction::Start);
	PlayerController->PlayDynamicForceFeedback(0.0f, 0.0f, true, false, false, false, EDynamicForceFeedbackAction::Stop, Baseline);

	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	Track.Step = NewObject<UFeelStep_ForceFeedbackCurve>(Recipe);
	Track.Duration = 1.0f;
	Track.IntensityCurve.GetRichCurve()->Reset();

	TestTrue(TEXT("Recipe starts"), Subsystem->PlayFeel(Recipe, FFeelTarget()).IsValid());
	TestWorld.Step(0.1f);

	FFeelFrameOutput PlayerOutput;
	TestTrue(TEXT("The player receives force feedback output"), Subsystem->GetCameraOutput(PlayerController, PlayerOutput) && !PlayerOutput.ForceFeedback.IsZero());
	TestEqual(TEXT("One engine force feedback action per motor while playing"), CountActions(*PlayerController, Baseline + 1, 4), 4);

	TestWorld.Step(0.5f);
	TestWorld.Step(0.5f);
	TestEqual(TEXT("Every motor action stops when the recipe ends"), CountActions(*PlayerController, Baseline + 1, 4), 0);
	TestFalse(TEXT("Subsystem goes idle"), Subsystem->IsTickable());

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
