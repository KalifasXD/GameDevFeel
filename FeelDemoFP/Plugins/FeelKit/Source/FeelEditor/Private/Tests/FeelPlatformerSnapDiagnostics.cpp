// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelBlueprintLibrary.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostic (filter DiagFeel): records the camera and the character's mesh every frame through a jump and a double
 * jump on the Platforming level, once with FeelKit on and once off, and reports the largest single-frame changes, to find
 * visible snaps. Start the editor on /Game/Variant_Platforming/Lvl_Platforming.
 */
namespace FeelPlatformerSnap
{
	struct FSample
	{
		double Time = 0.0;
		float DeltaTime = 0.0f;
		FVector CameraOffset = FVector::ZeroVector;
		FRotator CameraRotation = FRotator::ZeroRotator;
		float FOV = 0.0f;
		FVector MeshScale = FVector::OneVector;
		FVector MeshOffset = FVector::ZeroVector;
		FRotator MeshRotation = FRotator::ZeroRotator;
		FString Playing;
	};

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		double PassStart = 0.0;
		bool bFeelOn = true;
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

	void Report(FRun& Run, FAutomationTestBase* Test)
	{
		const TCHAR* Pass = Run.bFeelOn ? TEXT("ON") : TEXT("OFF");
		struct FWorst
		{
			float Value = 0.0f;
			int32 Index = 0;
		};
		FWorst Camera, CameraTurn, FOV, Scale, MeshMove, MeshTurn;
		for (int32 Index = 1; Index < Run.Samples.Num(); ++Index)
		{
			const FSample& A = Run.Samples[Index - 1];
			const FSample& B = Run.Samples[Index];
			auto Keep = [Index](FWorst& Worst, float Value) { if (Value > Worst.Value) { Worst.Value = Value; Worst.Index = Index; } };
			Keep(Camera, static_cast<float>(FVector::Dist(A.CameraOffset, B.CameraOffset)));
			Keep(CameraTurn, static_cast<float>(FMath::Abs((B.CameraRotation - A.CameraRotation).GetNormalized().Pitch) + FMath::Abs((B.CameraRotation - A.CameraRotation).GetNormalized().Roll)));
			Keep(FOV, FMath::Abs(B.FOV - A.FOV));
			Keep(Scale, static_cast<float>((B.MeshScale - A.MeshScale).GetAbsMax()));
			Keep(MeshMove, static_cast<float>(FVector::Dist(A.MeshOffset, B.MeshOffset)));
			Keep(MeshTurn, static_cast<float>(FMath::Abs((B.MeshRotation - A.MeshRotation).GetNormalized().Yaw) + FMath::Abs((B.MeshRotation - A.MeshRotation).GetNormalized().Pitch) + FMath::Abs((B.MeshRotation - A.MeshRotation).GetNormalized().Roll)));
		}
		auto Line = [&Run, Test, Pass](const TCHAR* Name, const FWorst& Worst)
		{
			const FSample& S = Run.Samples.IsValidIndex(Worst.Index) ? Run.Samples[Worst.Index] : FSample();
			Test->AddInfo(FString::Printf(TEXT("SNAP %s largest frame change in %s: %.3f at %.3fs (frame %.1f ms) playing [%s]"), Pass, Name, Worst.Value, S.Time, S.DeltaTime * 1000.0f, *S.Playing));
		};
		Test->AddInfo(FString::Printf(TEXT("SNAP %s frames recorded: %d"), Pass, Run.Samples.Num()));
		Line(TEXT("camera offset (cm)"), Camera);
		Line(TEXT("camera pitch+roll (deg)"), CameraTurn);
		Line(TEXT("FOV (deg)"), FOV);
		Line(TEXT("mesh scale"), Scale);
		Line(TEXT("mesh offset (cm)"), MeshMove);
		Line(TEXT("mesh rotation (deg)"), MeshTurn);

		// The frames around the largest camera and mesh scale changes, to see their shape.
		TArray<int32> Shown;
		for (const int32 Around : { Camera.Index, Scale.Index })
		for (int32 Index = FMath::Max(0, Around - 4); Index <= FMath::Min(Run.Samples.Num() - 1, Around + 8); ++Index)
		{
			if (Shown.Contains(Index))
			{
				continue;
			}
			Shown.Add(Index);
			const FSample& S = Run.Samples[Index];
			Test->AddInfo(FString::Printf(TEXT("SNAP %s frame %.3fs cam (%.1f %.1f %.1f) pitch %.2f roll %.2f fov %.2f meshZ %.3f meshOff (%.1f %.1f %.1f) [%s]"), Pass, S.Time,
				S.CameraOffset.X, S.CameraOffset.Y, S.CameraOffset.Z, S.CameraRotation.Pitch, S.CameraRotation.Roll, S.FOV, S.MeshScale.Z, S.MeshOffset.X, S.MeshOffset.Y, S.MeshOffset.Z, *S.Playing));
		}
		for (const FString& Line2 : Run.Started)
		{
			Test->AddInfo(FString::Printf(TEXT("SNAP %s started %s"), Pass, *Line2));
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPlatformerSnapCommand, TSharedRef<FeelPlatformerSnap::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPlatformerSnapCommand::Update()
{
	using namespace FeelPlatformerSnap;
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

	// Record while a pass runs (stages 1 to 6).
	if (Character && Controller->PlayerCameraManager && Run->Stage >= 1 && Run->Stage <= 6)
	{
		FSample& S = Run->Samples.AddDefaulted_GetRef();
		S.Time = Now - Run->PassStart;
		S.DeltaTime = World->GetDeltaSeconds();
		S.CameraOffset = Controller->PlayerCameraManager->GetCameraLocation() - Character->GetActorLocation();
		S.CameraRotation = Controller->PlayerCameraManager->GetCameraRotation();
		S.FOV = Controller->PlayerCameraManager->GetFOVAngle();
		if (const USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			S.MeshScale = Mesh->GetRelativeScale3D();
			S.MeshOffset = Mesh->GetComponentLocation() - Character->GetActorLocation();
			S.MeshRotation = Mesh->GetRelativeRotation();
		}
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			S.Playing = FString::FromInt(Subsystem->GetNumActiveInstances());
		}
	}

	switch (Run->Stage)
	{
	case 0:
		if (!Character || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("SNAP Play In Editor did not start"));
				return true;
			}
			return false;
		}
		if (!Run->StartedHandle.IsValid())
		{
			if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
			{
				const TSharedRef<FRun> RunRef = Run;
				Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef](FFeelHandle, UFeelRecipe* Recipe)
				{
					RunRef->Started.Add(FString::Printf(TEXT("%.3fs %s"), FPlatformTime::Seconds() - RunRef->PassStart, Recipe ? *Recipe->GetName() : TEXT("?")));
				});
			}
		}
		UFeelBlueprintLibrary::SetFeelEnabled(Run->bFeelOn);
		Run->Samples.Reset();
		Run->Started.Reset();
		Run->PassStart = Now;
		Next();
		return false;

	case 1:
		if (Elapsed < 0.3)
		{
			return false;
		}
		Call(TEXT("DoJumpStart"));
		Next();
		return false;

	case 2:
		if (Elapsed < 0.1)
		{
			return false;
		}
		Call(TEXT("DoJumpEnd"));
		Next();
		return false;

	case 3:
		if (Elapsed < 0.25)
		{
			return false;
		}
		Call(TEXT("DoJumpStart"));
		Next();
		return false;

	case 4:
		if (Elapsed < 0.1)
		{
			return false;
		}
		Call(TEXT("DoJumpEnd"));
		Next();
		return false;

	case 5:
		if (Character && Character->GetCharacterMovement()->IsFalling() && Elapsed < 5.0)
		{
			return false;
		}
		Next();
		return false;

	case 6:
		if (Elapsed < 1.0)
		{
			return false;
		}
		Report(*Run, Test);
		if (Run->bFeelOn)
		{
			Run->bFeelOn = false;
			Run->Stage = 0;
			Run->StageStart = Now;
			return false;
		}
		UFeelBlueprintLibrary::SetFeelEnabled(true);
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPlatformerSnapDiagnostic, "DiagFeel.PlatformerSnap", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelPlatformerSnapDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelPlatformerSnapCommand(MakeShared<FeelPlatformerSnap::FRun>(), this));
	return true;
}

#endif
