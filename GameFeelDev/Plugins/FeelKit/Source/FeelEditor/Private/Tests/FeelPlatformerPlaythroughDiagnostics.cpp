// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelGalleryShots.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "FeelSwitch.h"
#include "FeelTriggerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostic (filter DiagFeel): plays the Platformer demo like a player. Start the editor on
 * /Game/Variant_Platforming/Lvl_Platforming, then this starts Play In Editor and, through the template's own input
 * functions, jumps, double jumps, dashes, gets launched the way the wall jump launches, and falls from high up. Records
 * which recipes start (with the landing speeds) and every Play In Editor warning.
 */
namespace FeelPlatformerPlaythrough
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		TArray<FString> Started;
		FDelegateHandle StartedHandle;
		int32 GalleryStage = -1;
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
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPlatformerPlaythroughCommand, TSharedRef<FeelPlatformerPlaythrough::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPlatformerPlaythroughCommand::Update()
{
	using namespace FeelPlatformerPlaythrough;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	ACharacter* Character = Controller ? Cast<ACharacter>(Controller->GetPawn()) : nullptr;
	auto Call = [Character](const TCHAR* FunctionName)
	{
		if (UFunction* Function = Character ? Character->FindFunction(FName(FunctionName)) : nullptr)
		{
			Character->ProcessEvent(Function, nullptr);
			return true;
		}
		return false;
	};
	auto Next = [this, Now](const TCHAR* Label)
	{
		Test->AddInfo(FString::Printf(TEXT("PLATFORMER %s"), Label));
		++Run->Stage;
		Run->StageStart = Now;
	};
	auto Grounded = [Character]() { return Character && Character->GetCharacterMovement() && !Character->GetCharacterMovement()->IsFalling(); };
	// One gallery picture per stage, At seconds into it.
	auto Gallery = [this, World, Elapsed](const TCHAR* Name, double At)
	{
		if (Run->GalleryStage != Run->Stage && Elapsed > At)
		{
			Run->GalleryStage = Run->Stage;
			FeelGalleryShots::Take(World, TEXT("Platformer"), Name, Test);
		}
	};

	if (Run->Stage > 0 && Run->Stage < 11 && !Character)
	{
		Test->AddError(TEXT("PLATFORMER the player's character is gone"));
		GEditor->RequestEndPlayMap();
		Run->Stage = 12;
		return false;
	}

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Character || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("PLATFORMER Play In Editor did not start"));
				return true;
			}
			return false;
		}
		int32 Switches = 0;
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			++Switches;
		}
		const UFeelTriggerComponent* Trigger = Character->FindComponentByClass<UFeelTriggerComponent>();
		Test->AddInfo(FString::Printf(TEXT("PLATFORMER character %s, Feel Trigger entries %d, Feel Switches %d"), *Character->GetClass()->GetName(), Trigger ? Trigger->Triggers.Num() : -1, Switches));
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			const double Start = Now;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef, Start, Character](FFeelHandle, UFeelRecipe* Recipe)
			{
				const float FallSpeed = Character && Character->GetCharacterMovement() ? static_cast<float>(-Character->GetCharacterMovement()->Velocity.Z) : 0.0f;
				RunRef->Started.Add(FString::Printf(TEXT("%.2fs %s (downward speed %.0f)"), FPlatformTime::Seconds() - Start, Recipe ? *Recipe->GetName() : TEXT("?"), FallSpeed));
			});
		}
		Call(TEXT("DoJumpStart"));
		Next(TEXT("jump pressed"));
		return false;
	}

	case 1:
		if (Elapsed < 0.1)
		{
			return false;
		}
		Call(TEXT("DoJumpEnd"));
		Next(TEXT("jump released"));
		return false;

	case 2:
		// Second press near the top of the jump: the double jump.
		if (Elapsed < 0.25)
		{
			return false;
		}
		Call(TEXT("DoJumpStart"));
		Next(TEXT("double jump pressed"));
		return false;

	case 3:
		if (Elapsed < 0.1)
		{
			return false;
		}
		Call(TEXT("DoJumpEnd"));
		Next(TEXT("double jump released"));
		return false;

	case 4:
		Gallery(TEXT("DoubleJump"), 0.15);
		if (!Grounded() && Elapsed < 5.0)
		{
			return false;
		}
		Next(TEXT("landed after the double jump"));
		return false;

	case 5:
		Gallery(TEXT("Land"), 0.02);
		if (Elapsed < 0.5)
		{
			return false;
		}
		Call(TEXT("DoDash"));
		Next(TEXT("dash"));
		return false;

	case 6:
		Gallery(TEXT("Dash"), 0.08);
		if (Elapsed < 1.5)
		{
			return false;
		}
		// The template's wall jump is a Launch Character away from the wall and up.
		Character->LaunchCharacter(Character->GetActorForwardVector() * -800.0 + FVector(0.0, 0.0, 900.0), true, true);
		Next(TEXT("launched like a wall jump"));
		return false;

	case 7:
		if ((!Grounded() || Elapsed < 0.3) && Elapsed < 5.0)
		{
			return false;
		}
		Next(TEXT("landed after the launch"));
		return false;

	case 8:
		Gallery(TEXT("LaunchLand"), 0.02);
		if (Elapsed < 0.5)
		{
			return false;
		}
		// A long fall: from well above the ground.
		Character->SetActorLocation(Character->GetActorLocation() + FVector(0.0, 0.0, 1400.0), false, nullptr, ETeleportType::TeleportPhysics);
		Next(TEXT("lifted 14 m for a long fall"));
		return false;

	case 9:
		if ((!Grounded() || Elapsed < 0.3) && Elapsed < 6.0)
		{
			return false;
		}
		Next(TEXT("landed after the long fall"));
		return false;

	case 10:
	{
		Gallery(TEXT("FallLand"), 0.02);
		if (Elapsed < 1.5)
		{
			return false;
		}
		for (const FString& Line : Run->Started)
		{
			Test->AddInfo(FString::Printf(TEXT("PLATFORMER started %s"), *Line));
		}
		FMessageLogModule& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
		int32 Warnings = 0;
		for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.GetLogListing(TEXT("PIE"))->GetFilteredMessages())
		{
			if (Message->GetSeverity() <= EMessageSeverity::Warning)
			{
				++Warnings;
				Test->AddInfo(FString::Printf(TEXT("PLATFORMER PIE message: %s"), *Message->ToText().ToString()));
			}
		}
		Test->AddInfo(FString::Printf(TEXT("PLATFORMER PIE warnings and errors: %d"), Warnings));
		GEditor->RequestEndPlayMap();
		Next(TEXT("done"));
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPlatformerPlaythroughDiagnostic, "DiagFeel.PlatformerPlaythrough", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelPlatformerPlaythroughDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelPlatformerPlaythroughCommand(MakeShared<FeelPlatformerPlaythrough::FRun>(), this));
	return true;
}

#endif
