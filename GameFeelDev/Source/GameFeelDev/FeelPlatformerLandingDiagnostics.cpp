// Diagnostic (filter DiagFeel): drops the Platforming character from several heights and records, every frame around
// the landing, what FeelKit adds to the camera: the final view (after camera modifiers) minus the character's own
// camera component. Reports how far FeelKit moves the view down and up, and how far it tilts it, per drop. Start the
// editor on /Game/Variant_Platforming/Lvl_Platforming. Editor builds only.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

namespace FeelLandingStudy
{
	struct FSample
	{
		double Time = 0.0;
		double Lift = 0.0;
		double Tilt = 0.0;
	};

	struct FRun
	{
		TArray<float> Heights = { 60.0f, 200.0f, 450.0f, 900.0f };
		int32 DropIndex = 0;
		int32 Stage = 0;
		double StageStart = 0.0;
		double LandedAt = 0.0;
		bool bLanded = false;
		FVector Floor = FVector::ZeroVector;
		TArray<FSample> Samples;
		TArray<FString> Started;
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

	void Report(FRun& Run, FAutomationTestBase* Test, float Height)
	{
		double Down = 0.0, Up = 0.0, TiltDown = 0.0, TiltUp = 0.0;
		int32 Deepest = 0;
		for (int32 Index = 0; Index < Run.Samples.Num(); ++Index)
		{
			const FSample& S = Run.Samples[Index];
			if (S.Lift < Down)
			{
				Down = S.Lift;
				Deepest = Index;
			}
			Up = FMath::Max(Up, S.Lift);
			TiltDown = FMath::Min(TiltDown, S.Tilt);
			TiltUp = FMath::Max(TiltUp, S.Tilt);
		}
		// Once the view starts settling back, it should only rise: a frame that drops again reads as shaking.
		int32 Reversals = 0;
		for (int32 Index = Deepest + 1; Index < Run.Samples.Num(); ++Index)
		{
			if (Run.Samples[Index].Lift < Run.Samples[Index - 1].Lift - 0.1 || Run.Samples[Index].Tilt < Run.Samples[Index - 1].Tilt - 0.05)
			{
				++Reversals;
			}
		}
		double SettledAt = 0.0;
		for (const FSample& S : Run.Samples)
		{
			if (FMath::Abs(S.Lift) > 0.1 || FMath::Abs(S.Tilt) > 0.05)
			{
				SettledAt = S.Time;
			}
		}
		Test->AddInfo(FString::Printf(TEXT("LAND drop %.0f cm: FeelKit moves the view down %.1f cm and up %.1f cm, tilts it down %.2f and up %.2f degrees, deepest at %.2fs, back at rest after %.2fs, frames that drop again while settling %d (%d frames)"),
			Height, -Down, Up, -TiltDown, TiltUp, Run.Samples.IsValidIndex(Deepest) ? Run.Samples[Deepest].Time : 0.0, SettledAt, Reversals, Run.Samples.Num()));
		FString Curve;
		for (int32 Index = 0; Index < Run.Samples.Num(); Index += 2)
		{
			Curve += FString::Printf(TEXT(" %.2f:%+.1f/%+.2f"), Run.Samples[Index].Time, Run.Samples[Index].Lift, Run.Samples[Index].Tilt);
		}
		Test->AddInfo(FString::Printf(TEXT("LAND drop %.0f cm curve (s:cm/deg)%s"), Height, *Curve));
		for (const FString& Line : Run.Started)
		{
			Test->AddInfo(FString::Printf(TEXT("LAND drop %.0f cm started %s"), Height, *Line));
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelLandingStudyCommand, TSharedRef<FeelLandingStudy::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelLandingStudyCommand::Update()
{
	using namespace FeelLandingStudy;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	ACharacter* Character = Controller ? Cast<ACharacter>(Controller->GetPawn()) : nullptr;
	const UCameraComponent* Camera = Character ? Character->FindComponentByClass<UCameraComponent>() : nullptr;

	switch (Run->Stage)
	{
	case 0:
		if (!Character || !Camera || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("LAND Play In Editor did not start with a character and a camera"));
				return true;
			}
			return false;
		}
		Run->Floor = Character->GetActorLocation() - FVector(0.0, 0.0, Character->GetSimpleCollisionHalfHeight());
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef](FFeelHandle, UFeelRecipe* Recipe)
			{
				RunRef->Started.Add(FString::Printf(TEXT("%.3fs %s"), FPlatformTime::Seconds() - RunRef->LandedAt, Recipe ? *Recipe->GetName() : TEXT("?")));
			});
		}
		Run->Stage = 1;
		Run->StageStart = Now;
		return false;

	case 1:
	{
		// Lift the character straight up above where it started and let it fall.
		const float Height = Run->Heights[Run->DropIndex];
		const FVector Top = Run->Floor + FVector(0.0, 0.0, Height + Character->GetSimpleCollisionHalfHeight());
		Character->SetActorLocation(Top, false, nullptr, ETeleportType::TeleportPhysics);
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		Run->Samples.Reset();
		Run->Started.Reset();
		Run->bLanded = false;
		Run->Stage = 2;
		Run->StageStart = Now;
		return false;
	}

	case 2:
	{
		const bool bFalling = Character->GetCharacterMovement()->IsFalling();
		if (!Run->bLanded && !bFalling && Elapsed > 0.05)
		{
			Run->bLanded = true;
			Run->LandedAt = Now;
		}
		if (Run->bLanded && Controller->PlayerCameraManager)
		{
			// What FeelKit adds: the final view against the character's own camera, measured along world up and in pitch.
			FSample& S = Run->Samples.AddDefaulted_GetRef();
			S.Time = Now - Run->LandedAt;
			S.Lift = Controller->PlayerCameraManager->GetCameraLocation().Z - Camera->GetComponentLocation().Z;
			S.Tilt = (Controller->PlayerCameraManager->GetCameraRotation() - Camera->GetComponentRotation()).GetNormalized().Pitch;
		}
		if (Run->bLanded && Now - Run->LandedAt > 1.2)
		{
			Report(*Run, Test, Run->Heights[Run->DropIndex]);
			++Run->DropIndex;
			Run->Stage = Run->DropIndex < Run->Heights.Num() ? 3 : 4;
			Run->StageStart = Now;
			if (Run->Stage == 4)
			{
				GEditor->RequestEndPlayMap();
			}
		}
		else if (!Run->bLanded && Elapsed > 6.0)
		{
			Test->AddError(TEXT("LAND the character never landed"));
			GEditor->RequestEndPlayMap();
			Run->Stage = 4;
			Run->StageStart = Now;
		}
		return false;
	}

	case 3:
		// Let everything from the last landing finish before the next drop.
		if (Elapsed > 1.0)
		{
			Run->Stage = 1;
			Run->StageStart = Now;
		}
		return false;

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLandingStudyDiagnostic, "DiagFeel.PlatformerLanding", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelLandingStudyDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelLandingStudyCommand(MakeShared<FeelLandingStudy::FRun>(), this));
	return true;
}

#endif
