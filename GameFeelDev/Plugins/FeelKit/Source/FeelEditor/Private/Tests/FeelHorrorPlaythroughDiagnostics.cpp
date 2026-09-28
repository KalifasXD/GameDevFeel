// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/TriggerBox.h"
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
 * Diagnostic (filter DiagFeel): plays the Horror demo like a player (project FeelDemoFP, editor started on
 * /Game/Variant_Horror/Lvl_Horror). Sprints until out of breath and waits for the recovery, then steps into every
 * Trigger Box that has a Feel Trigger. Records which recipes start and stop, and every Play In Editor warning.
 */
namespace FeelHorrorPlaythrough
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Zone = 0;
		int32 GalleryShots = 0;
		TArray<TWeakObjectPtr<AActor>> Zones;
		TArray<FString> Events;
		FDelegateHandle StartedHandle;
		FDelegateHandle StoppedHandle;
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelHorrorPlaythroughCommand, TSharedRef<FeelHorrorPlaythrough::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelHorrorPlaythroughCommand::Update()
{
	using namespace FeelHorrorPlaythrough;
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
		}
	};
	auto Next = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
	};

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Character || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("HORROR Play In Editor did not start"));
				return true;
			}
			return false;
		}
		int32 Switches = 0;
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			++Switches;
		}
		// Everything in the level that reacts: trigger boxes (doorways, scare spots) and the lamps themselves.
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (*It != Character && It->FindComponentByClass<UFeelTriggerComponent>())
			{
				Run->Zones.Add(*It);
			}
		}
		Test->AddInfo(FString::Printf(TEXT("HORROR character %s, actors with a Feel Trigger %d, Feel Switches %d"), *Character->GetClass()->GetName(), Run->Zones.Num(), Switches));
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			const double Start = Now;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef, Start](FFeelHandle, UFeelRecipe* Recipe)
			{
				RunRef->Events.Add(FString::Printf(TEXT("%.2fs started %s"), FPlatformTime::Seconds() - Start, Recipe ? *Recipe->GetName() : TEXT("?")));
			});
			Run->StoppedHandle = Subsystem->OnFeelFinished.AddLambda([RunRef, Start](FFeelHandle, UFeelRecipe* Recipe, bool bInterrupted)
			{
				RunRef->Events.Add(FString::Printf(TEXT("%.2fs finished %s%s"), FPlatformTime::Seconds() - Start, Recipe ? *Recipe->GetName() : TEXT("?"), bInterrupted ? TEXT(" (interrupted)") : TEXT("")));
			});
		}
		Call(TEXT("DoStartSprint"));
		Next();
		return false;
	}

	case 1:
		// Sprint for 5 s (the stamina lasts 3 s), turning away from walls so the run keeps going.
		if (Character)
		{
			Character->AddMovementInput(Controller->GetControlRotation().Vector().GetSafeNormal2D(), 1.0f);
			if (Elapsed > 0.5 && Character->GetVelocity().Size2D() < 100.0)
			{
				Controller->SetControlRotation(Controller->GetControlRotation() + FRotator(0.0, 90.0, 0.0));
			}
		}
		if (Run->GalleryShots < 2 && Elapsed > 1.5 + 1.5 * Run->GalleryShots)
		{
			FeelGalleryShots::Take(World, TEXT("Horror"), FString::Printf(TEXT("Sprint_%d"), Run->GalleryShots), Test);
			++Run->GalleryShots;
		}
		if (Elapsed < 5.0)
		{
			return false;
		}
		Run->GalleryShots = 0;
		Call(TEXT("DoEndSprint"));
		Next();
		return false;

	case 2:
		// Stand still until the stamina has recovered, then jump once for the landing.
		if (Elapsed < 5.0)
		{
			return false;
		}
		Call(TEXT("DoJumpStart"));
		Call(TEXT("DoJumpEnd"));
		Next();
		return false;

	case 3:
		// Step into each trigger box, one every 1.5 s.
		if (Run->Zone > 0 && Run->GalleryShots < 2 && Elapsed > 0.3 + 0.6 * Run->GalleryShots)
		{
			FeelGalleryShots::Take(World, TEXT("Horror"), FString::Printf(TEXT("Zone%d_%d"), Run->Zone - 1, Run->GalleryShots), Test);
			++Run->GalleryShots;
		}
		if (Elapsed < 1.5)
		{
			return false;
		}
		Run->GalleryShots = 0;
		if (Character && Run->Zones.IsValidIndex(Run->Zone))
		{
			if (AActor* Zone = Run->Zones[Run->Zone].Get())
			{
				// Lamps hang from the ceiling: stand on the floor under them.
				const bool bLamp = Zone->GetClass()->GetName().StartsWith(TEXT("Light_C"));
				Character->SetActorLocation(Zone->GetActorLocation() - FVector(0.0, 0.0, bLamp ? 200.0 : 0.0), false, nullptr, ETeleportType::TeleportPhysics);
				Test->AddInfo(FString::Printf(TEXT("HORROR stepped into %s"), *Zone->GetActorLabel()));
			}
			++Run->Zone;
			Run->StageStart = Now;
			return false;
		}
		Next();
		return false;

	case 4:
	{
		if (Elapsed < 3.0)
		{
			return false;
		}
		for (const FString& Line : Run->Events)
		{
			Test->AddInfo(FString::Printf(TEXT("HORROR %s"), *Line));
		}
		FMessageLogModule& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
		int32 Warnings = 0;
		for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.GetLogListing(TEXT("PIE"))->GetFilteredMessages())
		{
			if (Message->GetSeverity() <= EMessageSeverity::Warning)
			{
				++Warnings;
				Test->AddInfo(FString::Printf(TEXT("HORROR PIE message: %s"), *Message->ToText().ToString()));
			}
		}
		Test->AddInfo(FString::Printf(TEXT("HORROR PIE warnings and errors: %d"), Warnings));
		GEditor->RequestEndPlayMap();
		Next();
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelHorrorPlaythroughDiagnostic, "DiagFeel.HorrorPlaythrough", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelHorrorPlaythroughDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelHorrorPlaythroughCommand(MakeShared<FeelHorrorPlaythrough::FRun>(), this));
	return true;
}

#endif
