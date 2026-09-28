// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelComfortSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/ForceFeedbackEffect.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostics for controller rumble in Play In Editor (single player): whether force feedback is enabled and scaled on
 * the player controller, and the motor values the engine computes for its own Client Play Force Feedback node and for
 * FeelKit's Haptic Pattern. Values above 0 mean the software side works and anything missing is between the engine and
 * the device. Needs the host project's /Game/FeelKitTests/FF_Test and R_Haptic.
 */
namespace FeelPIEForceFeedbackDiagnostics
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float MaxEngine = 0.0f;
		float MaxFeelKit = 0.0f;
		float MaxAfterComfortZero = 0.0f;
		float MaxEffectAfterZero = 0.0f;
	};

	UWorld* PIEWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	float MaxMotor(const FForceFeedbackValues& Values)
	{
		return FMath::Max(FMath::Max(Values.LeftLarge, Values.LeftSmall), FMath::Max(Values.RightLarge, Values.RightSmall));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPIEForceFeedbackCommand, TSharedRef<FeelPIEForceFeedbackDiagnostics::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPIEForceFeedbackCommand::Update()
{
	using namespace FeelPIEForceFeedbackDiagnostics;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;

	auto Next = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
	};

	switch (Run->Stage)
	{
	case 0:
		if (Controller && Controller->GetPawn() && Elapsed > 3.0)
		{
			Test->AddInfo(FString::Printf(TEXT("FFDIAG controller forceFeedbackEnabled=%d forceFeedbackScale=%.2f"), Controller->bForceFeedbackEnabled ? 1 : 0, Controller->ForceFeedbackScale));
			UForceFeedbackEffect* Effect = LoadObject<UForceFeedbackEffect>(nullptr, TEXT("/Game/FeelKitTests/FF_Test.FF_Test"));
			Test->AddInfo(FString::Printf(TEXT("FFDIAG FF_Test loaded=%d channels=%d"), Effect ? 1 : 0, Effect ? Effect->ChannelDetails.Num() : 0));
			if (Effect)
			{
				Controller->ClientPlayForceFeedback(Effect);
			}
			Next();
		}
		else if (Elapsed > 90.0)
		{
			Test->AddError(TEXT("FFDIAG PIE did not start"));
			GEditor->RequestEndPlayMap();
			return true;
		}
		return false;

	case 1:
		if (Controller)
		{
			Run->MaxEngine = FMath::Max(Run->MaxEngine, MaxMotor(Controller->ForceFeedbackValues));
		}
		if (Elapsed < 2.0)
		{
			return false;
		}
		Test->AddInfo(FString::Printf(TEXT("FFDIAG engine Client Play Force Feedback max motor=%.2f"), Run->MaxEngine));
		if (UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Haptic.R_Haptic")); Recipe && Controller)
		{
			const FFeelHandle Handle = World->GetSubsystem<UFeelSubsystem>()->PlayFeel(Recipe, FFeelTarget::FromActor(Controller->GetPawn()));
			Test->AddInfo(FString::Printf(TEXT("FFDIAG R_Haptic played=%d"), Handle.IsValid() ? 1 : 0));
		}
		Next();
		return false;

	case 2:
		if (Controller)
		{
			Run->MaxFeelKit = FMath::Max(Run->MaxFeelKit, MaxMotor(Controller->ForceFeedbackValues));
		}
		if (Elapsed < 2.0)
		{
			return false;
		}
		Test->AddInfo(FString::Printf(TEXT("FFDIAG FeelKit Haptic Pattern max motor=%.2f"), Run->MaxFeelKit));

		// The user's Blueprint sets Haptics comfort to 0 on the same key that plays the engine effect. Repeat that here.
		if (UFeelComfortSubsystem* Comfort = Controller && Controller->GetLocalPlayer() ? Controller->GetLocalPlayer()->GetSubsystem<UFeelComfortSubsystem>() : nullptr)
		{
			Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 0.0f);
			Test->AddInfo(FString::Printf(TEXT("FFDIAG after Haptics comfort 0: controller forceFeedbackScale=%.2f"), Controller->ForceFeedbackScale));
			if (UForceFeedbackEffect* Effect = LoadObject<UForceFeedbackEffect>(nullptr, TEXT("/Game/FeelKitTests/FF_Test.FF_Test")))
			{
				Controller->ClientPlayForceFeedback(Effect);
			}
		}
		Next();
		return false;

	case 3:
		if (Controller)
		{
			Run->MaxAfterComfortZero = FMath::Max(Run->MaxAfterComfortZero, MaxMotor(Controller->ForceFeedbackValues));
			for (const FActiveForceFeedbackEffect& Active : Controller->ActiveForceFeedbackEffects)
			{
				FForceFeedbackValues ActiveValues;
				Active.GetValues(ActiveValues);
				Run->MaxEffectAfterZero = FMath::Max(Run->MaxEffectAfterZero, MaxMotor(ActiveValues));
			}
		}
		if (Elapsed < 2.0)
		{
			return false;
		}
		Test->AddInfo(FString::Printf(TEXT("FFDIAG with Haptics comfort 0: what showdebug lists per effect=%.2f, what actually reaches the controller=%.2f"),
			Run->MaxEffectAfterZero, Run->MaxAfterComfortZero));
		if (UFeelComfortSubsystem* Comfort = Controller && Controller->GetLocalPlayer() ? Controller->GetLocalPlayer()->GetSubsystem<UFeelComfortSubsystem>() : nullptr)
		{
			Comfort->SetComfortGroupScale(EFeelComfortGroup::Haptics, 1.0f);
			Test->AddInfo(FString::Printf(TEXT("FFDIAG after Haptics comfort back to 1: forceFeedbackScale=%.2f"), Controller->ForceFeedbackScale));
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPIEForceFeedbackDiagnostic, "DiagFeel.PIEForceFeedback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelPIEForceFeedbackDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelPIEForceFeedbackCommand(MakeShared<FeelPIEForceFeedbackDiagnostics::FRun>(), this));
	return true;
}

#endif
