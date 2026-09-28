// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelGalleryShots.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "FeelSwitch.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "IMessageLogListing.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostic (filter DiagFeel): plays the Shooter demo like a player (project FeelDemoFP, editor started on
 * /Game/Variant_Shooter/Lvl_Shooter). Walks over every weapon pickup, then faces the nearest enemy and fires each weapon
 * for a while through the template's own DoStartFiring / DoStopFiring / DoSwitchWeapon. Records which recipes start and
 * every Play In Editor warning.
 */
namespace FeelShooterPlaythrough
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Pickup = 0;
		int32 Weapon = 0;
		bool bUsedJumpPad = false;
		int32 GalleryShots = 0;
		TArray<TWeakObjectPtr<AActor>> Pickups;
		TMap<FString, int32> Started;
		FDelegateHandle StartedHandle;
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

	/** The nearest living enemy (the template's enemies turn into ragdolls when they die). */
	AActor* NearestEnemy(UWorld* World, const AActor* From)
	{
		AActor* Best = nullptr;
		for (TActorIterator<ACharacter> It(World); It; ++It)
		{
			if (It->GetClass()->GetName().Contains(TEXT("ShooterNPC")) && !It->GetMesh()->IsSimulatingPhysics()
				&& (!Best || FVector::Dist(It->GetActorLocation(), From->GetActorLocation()) < FVector::Dist(Best->GetActorLocation(), From->GetActorLocation())))
			{
				Best = *It;
			}
		}
		return Best;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelShooterPlaythroughCommand, TSharedRef<FeelShooterPlaythrough::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelShooterPlaythroughCommand::Update()
{
	using namespace FeelShooterPlaythrough;
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
				Test->AddError(TEXT("SHOOTER Play In Editor did not start"));
				return true;
			}
			return false;
		}
		int32 Switches = 0;
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			++Switches;
		}
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetClass()->GetName().Contains(TEXT("ShooterPickup")))
			{
				Run->Pickups.Add(*It);
			}
		}
		Test->AddInfo(FString::Printf(TEXT("SHOOTER character %s, pickups %d, Feel Switches %d"), *Character->GetClass()->GetName(), Run->Pickups.Num(), Switches));
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef](FFeelHandle, UFeelRecipe* Recipe)
			{
				++RunRef->Started.FindOrAdd(Recipe ? Recipe->GetName() : TEXT("?"));
			});
		}
		// Pull the trigger once before picking anything up (no weapon yet).
		Call(TEXT("DoStartFiring"));
		Call(TEXT("DoStopFiring"));
		Next();
		return false;
	}

	case 1:
		// Walk over each pickup.
		if (Elapsed < 0.4)
		{
			return false;
		}
		if (Character && Run->Pickups.IsValidIndex(Run->Pickup))
		{
			if (AActor* Pickup = Run->Pickups[Run->Pickup].Get())
			{
				Character->SetActorLocation(Pickup->GetActorLocation() + FVector(0.0, 0.0, 60.0), false, nullptr, ETeleportType::TeleportPhysics);
			}
			++Run->Pickup;
			Run->StageStart = Now;
			return false;
		}
		// Then step onto a jump pad, if the level has one, and wait out the flight.
		if (!Run->bUsedJumpPad)
		{
			Run->bUsedJumpPad = true;
			for (TActorIterator<AActor> It(World); It; ++It)
			{
				if (Character && It->GetClass()->GetName().Contains(TEXT("JumpPad")))
				{
					Character->SetActorLocation(It->GetActorLocation() + FVector(0.0, 0.0, 100.0), false, nullptr, ETeleportType::TeleportPhysics);
					Test->AddInfo(FString::Printf(TEXT("SHOOTER stepped onto %s"), *It->GetName()));
					Run->StageStart = Now + 3.0;
					break;
				}
			}
			return false;
		}
		Next();
		return false;

	case 2:
	{
		// Face the nearest enemy from 9 m away and fire the current weapon for 2.5 s.
		if (Elapsed < 0.5 || !Character)
		{
			return false;
		}
		AActor* Enemy = NearestEnemy(World, Character);
		if (!Enemy)
		{
			// Nobody left: fire at the floor ahead, so every weapon (and the grenade's explosion) still plays.
			Controller->SetControlRotation(FRotator(-25.0, Controller->GetControlRotation().Yaw, 0.0));
			Call(TEXT("DoStartFiring"));
			Test->AddInfo(FString::Printf(TEXT("SHOOTER weapon %d firing at the floor"), Run->Weapon));
			Next();
			return false;
		}
		const FVector Away = (Character->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
		Character->SetActorLocation(Enemy->GetActorLocation() + Away * 900.0 + FVector(0.0, 0.0, 40.0), false, nullptr, ETeleportType::TeleportPhysics);
		Controller->SetControlRotation((Enemy->GetActorLocation() + FVector(0.0, 0.0, 30.0) - Character->GetPawnViewLocation()).Rotation());
		Call(TEXT("DoStartFiring"));
		Test->AddInfo(FString::Printf(TEXT("SHOOTER weapon %d firing at %s"), Run->Weapon, *Enemy->GetName()));
		Next();
		return false;
	}

	case 3:
		// Keep aiming while firing (enemies move).
		if (Character)
		{
			if (AActor* Enemy = NearestEnemy(World, Character))
			{
				Controller->SetControlRotation((Enemy->GetActorLocation() + FVector(0.0, 0.0, 30.0) - Character->GetPawnViewLocation()).Rotation());
			}
		}
		if (Run->GalleryShots < 3 && Elapsed > 0.6 + 0.6 * Run->GalleryShots)
		{
			FeelGalleryShots::Take(World, TEXT("Shooter"), FString::Printf(TEXT("Weapon%d_%d"), Run->Weapon, Run->GalleryShots), Test);
			++Run->GalleryShots;
		}
		if (Elapsed < 2.5)
		{
			return false;
		}
		Run->GalleryShots = 0;
		Call(TEXT("DoStopFiring"));
		Next();
		return false;

	case 4:
		if (Elapsed < 0.6)
		{
			return false;
		}
		if (++Run->Weapon < 3)
		{
			Call(TEXT("DoSwitchWeapon"));
			Run->Stage = 2;
			Run->StageStart = Now;
			return false;
		}
		Next();
		return false;

	case 5:
	{
		if (Elapsed < 2.0)
		{
			return false;
		}
		TArray<FString> Names;
		Run->Started.GetKeys(Names);
		Names.Sort();
		for (const FString& Name : Names)
		{
			Test->AddInfo(FString::Printf(TEXT("SHOOTER started %s x%d"), *Name, Run->Started[Name]));
		}
		FMessageLogModule& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
		int32 Warnings = 0;
		for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.GetLogListing(TEXT("PIE"))->GetFilteredMessages())
		{
			if (Message->GetSeverity() <= EMessageSeverity::Warning)
			{
				++Warnings;
				Test->AddInfo(FString::Printf(TEXT("SHOOTER PIE message: %s"), *Message->ToText().ToString()));
			}
		}
		Test->AddInfo(FString::Printf(TEXT("SHOOTER PIE warnings and errors: %d"), Warnings));
		GEditor->RequestEndPlayMap();
		Next();
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelShooterPlaythroughDiagnostic, "DiagFeel.ShooterPlaythrough", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelShooterPlaythroughDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelShooterPlaythroughCommand(MakeShared<FeelShooterPlaythrough::FRun>(), this));
	return true;
}

#endif
