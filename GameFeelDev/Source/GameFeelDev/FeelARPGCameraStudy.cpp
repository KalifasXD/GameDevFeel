// Diagnostic (filter DiagFeel): measures how well the Action/RPG demo camera shows a fight. Start the editor on
// /Game/Variant_Combat/Lvl_Combat. For each camera setup (arm length, arm position beside and above the character,
// field of view) it stands the character in front of the training dummy, runs the three-hit combo, and on every frame
// of the combo projects the character and the dummy onto the screen through the player's real view (FeelKit effects
// included). It reports how much of the dummy stays visible (not on screen or behind the character), and how large
// both appear. Editor builds only.
//
// -FeelCameraSetups="100,40,70,90;350,0,90,80" measures only those setups (arm, side, height, field of view) and saves
// a picture at each impact (Saved/FeelKit/Camera_<setup>_<n>_impact.png) and reports how much sharpness the impact frame keeps
// against the frame 0.15 s before it (motion blur lowers it). Without it, a grid of setups is measured, no pictures.
// -FeelCameraFeelOff runs with FeelKit switched off, to compare. -FeelCameraNoPictures skips the pictures, which stall the
// frames they are taken in, so frame timing and frame-to-frame moves are measured cleanly.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "Camera/CameraComponent.h"
#include "CombatCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "FeelFrameOutput.h"
#include "FeelSubsystem.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/App.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "ImageUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "UnrealClient.h"

namespace FeelCameraStudy
{
	struct FSetup
	{
		float Arm = 100.0f;
		float Side = 40.0f;
		float Height = 70.0f;
		float FieldOfView = 90.0f;

		FString Name() const
		{
			return FString::Printf(TEXT("%.0f_%.0f_%.0f_%.0f"), Arm, Side, Height, FieldOfView);
		}
	};

	struct FSetupResult
	{
		int32 Frames = 0;
		double VisibleSum = 0.0;
		double WorstVisible = 1.0;
		double BehindSum = 0.0;
		double OffScreenSum = 0.0;
		double PlayerHeightSum = 0.0;
		double DummyHeightSum = 0.0;
		double ArmSum = 0.0;
		/** FeelKit's own turn of the view (final view against the character's camera): largest angle and fastest change. */
		double MaxTurn = 0.0;
		double MaxTurnSpeed = 0.0;
		FRotator PreviousTurn = FRotator::ZeroRotator;
		double PreviousTime = -1.0;
		FString TurnCurve;
		double LongestFrame = 0.0;
		/** Largest on-screen move of the target caused by FeelKit, as a share of the screen width. */
		double MaxKnock = 0.0;
		/** Largest frame-to-frame move of the target on screen caused by the camera effects, as a share of the screen width. */
		double MaxKnockStep = 0.0;
		double MaxKnockStepTime = 0.0;
		FVector2D PreviousKnock = FVector2D::ZeroVector;
		bool bHasPreviousKnock = false;
		double StartTime = 0.0;
		FString SnapLog;
	};

	struct FRun
	{
		TArray<FSetup> Setups;
		bool bPictures = false;
		int32 SetupIndex = 0;
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Presses = 0;
		int32 Shots = 0;
		double SharpnessBefore = 0.0;
		bool bApplied = false;
		TWeakObjectPtr<AActor> Dummy;
		TArray<FSetupResult> Results;
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

	TArray<FSetup> ParseSetups(const FString& Text)
	{
		TArray<FSetup> Setups;
		TArray<FString> Entries;
		Text.ParseIntoArray(Entries, TEXT(";"));
		for (const FString& Entry : Entries)
		{
			TArray<FString> Values;
			Entry.ParseIntoArray(Values, TEXT(","));
			if (Values.Num() == 4)
			{
				FSetup Setup;
				Setup.Arm = FCString::Atof(*Values[0]);
				Setup.Side = FCString::Atof(*Values[1]);
				Setup.Height = FCString::Atof(*Values[2]);
				Setup.FieldOfView = FCString::Atof(*Values[3]);
				Setups.Add(Setup);
			}
		}
		return Setups;
	}

	TArray<FSetup> Grid()
	{
		TArray<FSetup> Setups;
		// The template's own camera first, for comparison.
		Setups.Add(FSetup());
		for (const float Arm : { 200.0f, 280.0f, 350.0f, 420.0f })
		{
			for (const float Side : { 0.0f, 30.0f, 50.0f })
			{
				for (const float Height : { 70.0f, 100.0f })
				{
					for (const float FieldOfView : { 90.0f, 80.0f, 70.0f })
					{
						Setups.Add({ Arm, Side, Height, FieldOfView });
					}
				}
			}
		}
		return Setups;
	}

	void Apply(ACombatCharacter& Character, const FSetup& Setup)
	{
		USpringArmComponent* Boom = Character.GetCameraBoom();
		UCameraComponent* Camera = Character.GetFollowCamera();
		Boom->TargetArmLength = Setup.Arm;
		Boom->SetRelativeLocation(FVector(0.0, Setup.Side, Setup.Height));
		Camera->SetFieldOfView(Setup.FieldOfView);
	}

	void StandInFront(APawn& Pawn, APlayerController& Controller, const AActor& Target)
	{
		const FVector Toward = (Pawn.GetActorLocation() - Target.GetActorLocation()).GetSafeNormal2D();
		const FVector Stand = Target.GetActorLocation() + Toward * 110.0f + FVector(0.0, 0.0, 20.0);
		const FRotator Facing = (-Toward).Rotation();
		Pawn.SetActorLocationAndRotation(Stand, Facing, false, nullptr, ETeleportType::TeleportPhysics);
		Controller.SetControlRotation(FRotator(-12.0, Facing.Yaw, 0.0));
	}

	/** Bone positions widened by a limb's thickness across the view, so the outline covers arms and legs, not only their centre lines. */
	void CharacterPoints(const ACharacter& Character, const FVector& ViewRight, const FVector& ViewUp, TArray<FVector>& OutPoints)
	{
		const USkeletalMeshComponent* Mesh = Character.GetMesh();
		for (int32 Bone = 0; Bone < Mesh->GetNumBones(); ++Bone)
		{
			const FVector Location = Mesh->GetBoneLocation(Mesh->GetBoneName(Bone));
			for (const FVector& Offset : { FVector::ZeroVector, ViewRight * 9.0, ViewRight * -9.0, ViewUp * 9.0, ViewUp * -9.0 })
			{
				OutPoints.Add(Location + Offset);
			}
		}
	}

	/** A grid through the dummy's own mesh bounds (not its base plate). */
	void DummyPoints(const AActor& Dummy, TArray<FVector>& OutPoints)
	{
		TInlineComponentArray<UStaticMeshComponent*> Meshes(&Dummy);
		for (const UStaticMeshComponent* Mesh : Meshes)
		{
			if (!Mesh->IsSimulatingPhysics())
			{
				continue;
			}
			const FBox Box = Mesh->Bounds.GetBox();
			for (int32 X = 0; X <= 2; ++X)
			{
				for (int32 Y = 0; Y <= 2; ++Y)
				{
					for (int32 Z = 0; Z <= 6; ++Z)
					{
						OutPoints.Add(Box.Min + (Box.Max - Box.Min) * FVector(X / 2.0, Y / 2.0, Z / 6.0));
					}
				}
			}
		}
	}

	/** Convex hull (monotone chain) of screen points. */
	TArray<FVector2D> Hull(TArray<FVector2D> Points)
	{
		Points.Sort([](const FVector2D& A, const FVector2D& B) { return A.X < B.X || (A.X == B.X && A.Y < B.Y); });
		if (Points.Num() < 3)
		{
			return Points;
		}
		auto Cross = [](const FVector2D& O, const FVector2D& A, const FVector2D& B) { return (A.X - O.X) * (B.Y - O.Y) - (A.Y - O.Y) * (B.X - O.X); };
		TArray<FVector2D> Result;
		Result.SetNum(Points.Num() * 2);
		int32 Count = 0;
		for (int32 Index = 0; Index < Points.Num(); ++Index)
		{
			while (Count >= 2 && Cross(Result[Count - 2], Result[Count - 1], Points[Index]) <= 0.0)
			{
				--Count;
			}
			Result[Count++] = Points[Index];
		}
		for (int32 Index = Points.Num() - 2, Lower = Count + 1; Index >= 0; --Index)
		{
			while (Count >= Lower && Cross(Result[Count - 2], Result[Count - 1], Points[Index]) <= 0.0)
			{
				--Count;
			}
			Result[Count++] = Points[Index];
		}
		Result.SetNum(Count - 1);
		return Result;
	}

	bool InsideHull(const TArray<FVector2D>& HullPoints, const FVector2D& Point)
	{
		if (HullPoints.Num() < 3)
		{
			return false;
		}
		for (int32 Index = 0; Index < HullPoints.Num(); ++Index)
		{
			const FVector2D& A = HullPoints[Index];
			const FVector2D& B = HullPoints[(Index + 1) % HullPoints.Num()];
			if ((B.X - A.X) * (Point.Y - A.Y) - (B.Y - A.Y) * (Point.X - A.X) < 0.0)
			{
				return false;
			}
		}
		return true;
	}

	/** One frame: how much of the dummy the player can see, and how big both are on screen. */
	void Measure(FSetupResult& Result, APlayerController& Controller, ACombatCharacter& Character, const AActor& Dummy)
	{
		int32 Width = 0;
		int32 Height = 0;
		Controller.GetViewportSize(Width, Height);
		if (Width <= 0 || Height <= 0)
		{
			return;
		}
		FVector ViewLocation;
		FRotator ViewRotation;
		Controller.GetPlayerViewPoint(ViewLocation, ViewRotation);
		const FRotationMatrix View(ViewRotation);

		TArray<FVector> CharacterWorld;
		CharacterPoints(Character, View.GetUnitAxis(EAxis::Y), View.GetUnitAxis(EAxis::Z), CharacterWorld);
		TArray<FVector2D> CharacterScreen;
		for (const FVector& Point : CharacterWorld)
		{
			FVector2D Screen;
			if (UGameplayStatics::ProjectWorldToScreen(&Controller, Point, Screen))
			{
				CharacterScreen.Add(Screen);
			}
		}
		const TArray<FVector2D> Outline = Hull(CharacterScreen);

		TArray<FVector> DummyWorld;
		DummyPoints(Dummy, DummyWorld);
		if (DummyWorld.Num() == 0)
		{
			return;
		}
		int32 Visible = 0;
		int32 Behind = 0;
		int32 OffScreen = 0;
		float DummyTop = TNumericLimits<float>::Max();
		float DummyBottom = TNumericLimits<float>::Lowest();
		for (const FVector& Point : DummyWorld)
		{
			FVector2D Screen;
			const bool bProjected = UGameplayStatics::ProjectWorldToScreen(&Controller, Point, Screen);
			if (!bProjected || Screen.X < 0.0 || Screen.Y < 0.0 || Screen.X > Width || Screen.Y > Height)
			{
				++OffScreen;
				continue;
			}
			DummyTop = FMath::Min(DummyTop, static_cast<float>(Screen.Y));
			DummyBottom = FMath::Max(DummyBottom, static_cast<float>(Screen.Y));
			if (InsideHull(Outline, Screen))
			{
				++Behind;
				continue;
			}
			++Visible;
		}

		float CharacterTop = TNumericLimits<float>::Max();
		float CharacterBottom = TNumericLimits<float>::Lowest();
		for (const FVector2D& Point : CharacterScreen)
		{
			CharacterTop = FMath::Min(CharacterTop, static_cast<float>(FMath::Clamp(Point.Y, 0.0, static_cast<double>(Height))));
			CharacterBottom = FMath::Max(CharacterBottom, static_cast<float>(FMath::Clamp(Point.Y, 0.0, static_cast<double>(Height))));
		}

		const double Total = DummyWorld.Num();
		const double VisibleShare = Visible / Total;
		++Result.Frames;
		Result.VisibleSum += VisibleShare;
		Result.WorstVisible = FMath::Min(Result.WorstVisible, VisibleShare);
		Result.BehindSum += Behind / Total;
		Result.OffScreenSum += OffScreen / Total;
		Result.PlayerHeightSum += CharacterScreen.Num() > 0 ? (CharacterBottom - CharacterTop) / Height : 0.0;
		Result.DummyHeightSum += DummyBottom > DummyTop ? (DummyBottom - DummyTop) / Height : 0.0;
		Result.ArmSum += FVector::Dist(ViewLocation, Character.GetActorLocation());

		const FRotator Turn = (ViewRotation - Character.GetFollowCamera()->GetComponentRotation()).GetNormalized();
		const double TurnAngle = FMath::Sqrt(FMath::Square(Turn.Pitch) + FMath::Square(Turn.Yaw) + FMath::Square(Turn.Roll));
		Result.MaxTurn = FMath::Max(Result.MaxTurn, TurnAngle);
		const double Time = FPlatformTime::Seconds();
		if (Result.PreviousTime > 0.0 && Time > Result.PreviousTime)
		{
			const FRotator Change = (Turn - Result.PreviousTurn).GetNormalized();
			const double ChangeAngle = FMath::Sqrt(FMath::Square(Change.Pitch) + FMath::Square(Change.Yaw) + FMath::Square(Change.Roll));
			Result.MaxTurnSpeed = FMath::Max(Result.MaxTurnSpeed, ChangeAngle / (Time - Result.PreviousTime));
		}
		Result.PreviousTurn = Turn;
		Result.PreviousTime = Time;
		Result.LongestFrame = FMath::Max(Result.LongestFrame, FApp::GetDeltaTime());

		// Knock: how far FeelKit moves the target on screen, as a share of the screen width. The dummy's centre is projected
		// through the final view and through the character's own camera; push, turn and zoom all count.
		if (Controller.PlayerCameraManager)
		{
			const FVector Target = Dummy.GetComponentsBoundingBox().GetCenter();
			auto Project = [Width](const FVector& Point, const FVector& From, const FRotator& Facing, float FieldOfView)
			{
				const FVector Local = Facing.UnrotateVector(Point - From);
				const double Scale = (Width * 0.5) / FMath::Tan(FMath::DegreesToRadians(FieldOfView * 0.5));
				return Local.X > 1.0 ? FVector2D(Local.Y / Local.X * Scale, -Local.Z / Local.X * Scale) : FVector2D::ZeroVector;
			};
			const UCameraComponent* Own = Character.GetFollowCamera();
			const FVector2D WithFeel = Project(Target, Controller.PlayerCameraManager->GetCameraLocation(), Controller.PlayerCameraManager->GetCameraRotation(), Controller.PlayerCameraManager->GetFOVAngle());
			const FVector2D Without = Project(Target, Own->GetComponentLocation(), Own->GetComponentRotation(), Own->FieldOfView);
			Result.MaxKnock = FMath::Max(Result.MaxKnock, FVector2D::Distance(WithFeel, Without) / Width);

			// Snap: the largest move of the target on screen from one frame to the next caused by the camera effects
			// (FeelKit and engine shakes), and every frame's offsets around it.
			const FVector2D Knock = (WithFeel - Without) / Width;
			if (Result.bHasPreviousKnock)
			{
				const double Step = FVector2D::Distance(Knock, Result.PreviousKnock);
				if (Step > Result.MaxKnockStep)
				{
					Result.MaxKnockStep = Step;
					Result.MaxKnockStepTime = FPlatformTime::Seconds();
				}
			}
			Result.PreviousKnock = Knock;
			Result.bHasPreviousKnock = true;
			const FVector ScreenMove = Own->GetComponentRotation().UnrotateVector(Controller.PlayerCameraManager->GetCameraLocation() - Own->GetComponentLocation());
			const FRotator ScreenTurn = (Controller.PlayerCameraManager->GetCameraRotation() - Own->GetComponentRotation()).GetNormalized();
			FFeelFrameOutput Feel;
			if (const UFeelSubsystem* FeelSubsystem = Controller.GetWorld()->GetSubsystem<UFeelSubsystem>())
			{
				FeelSubsystem->GetCameraOutput(&Controller, Feel);
			}
			if (Knock.Size() > 0.001 || !ScreenMove.IsNearlyZero(0.5))
			{
				Result.SnapLog += FString::Printf(TEXT("\n  t %.3f knock %+.2f%%,%+.2f%% | screen move %+.0f %+.0f %+.0f turn %+.2f %+.2f %+.2f | feelkit move %+.0f %+.0f %+.0f turn %+.2f %+.2f %+.2f fov %+.1f"),
					FPlatformTime::Seconds() - Result.StartTime, 100.0 * Knock.X, 100.0 * Knock.Y,
					ScreenMove.X, ScreenMove.Y, ScreenMove.Z, ScreenTurn.Pitch, ScreenTurn.Yaw, ScreenTurn.Roll,
					Feel.CameraLocationOffset.X, Feel.CameraLocationOffset.Y, Feel.CameraLocationOffset.Z,
					Feel.CameraRotationOffset.Pitch, Feel.CameraRotationOffset.Yaw, Feel.CameraRotationOffset.Roll, Feel.FieldOfViewOffset);
			}
		}
		// Per frame: real frame time, world time dilation, FeelKit's computed camera pitch, the pitch on screen, plays running.
		FFeelFrameOutput FeelOutput;
		const UFeelSubsystem* Subsystem = Controller.GetWorld()->GetSubsystem<UFeelSubsystem>();
		if (Subsystem)
		{
			Subsystem->GetCameraOutput(&Controller, FeelOutput);
		}
		if (TurnAngle > 0.05 || !FeelOutput.CameraRotationOffset.IsNearlyZero(0.05))
		{
			Result.TurnCurve += FString::Printf(TEXT(" [dt %.0fms dil %.2f feel %+.2f screen %+.2f n%d]"), FApp::GetDeltaTime() * 1000.0,
				Controller.GetWorldSettings() ? Controller.GetWorldSettings()->TimeDilation : 1.0f, FeelOutput.CameraRotationOffset.Pitch, Turn.Pitch,
				Subsystem ? Subsystem->GetNumActiveInstances() : -1);
		}
	}

	/** Saves a picture of the game view when Label is set, and returns its sharpness (0 when the view cannot be read). */
	double Shot(FRun& Run, FAutomationTestBase* Test, const FSetup& Setup, const TCHAR* Label)
	{
		UWorld* World = PIEWorld();
		const FWorldContext* Context = World ? GEngine->GetWorldContextFromWorld(World) : nullptr;
		FViewport* Viewport = Context && Context->GameViewport ? Context->GameViewport->Viewport : nullptr;
		TArray<FColor> Pixels;
		if (!Viewport || !Viewport->ReadPixels(Pixels) || Pixels.Num() == 0)
		{
			return 0.0;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FIntPoint Size = Viewport->GetSizeXY();
		// Sharpness: mean brightness difference between neighbouring pixels. Motion blur lowers it.
		double Edges = 0.0;
		auto Brightness = [&Pixels, &Size](int32 X, int32 Y)
		{
			const FColor& Pixel = Pixels[Y * Size.X + X];
			return 0.299 * Pixel.R + 0.587 * Pixel.G + 0.114 * Pixel.B;
		};
		for (int32 Y = 0; Y + 1 < Size.Y; ++Y)
		{
			for (int32 X = 0; X + 1 < Size.X; ++X)
			{
				const double Here = Brightness(X, Y);
				Edges += FMath::Abs(Brightness(X + 1, Y) - Here) + FMath::Abs(Brightness(X, Y + 1) - Here);
			}
		}
		const double Sharpness = Edges / FMath::Max(1.0, static_cast<double>(Size.X - 1) * (Size.Y - 1));
		if (Label)
		{
			const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("Camera_%s_%s.png"), *Setup.Name(), Label);
			FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
			Test->AddInfo(FString::Printf(TEXT("CAMERA picture %s sharpness %.2f"), *File, Sharpness));
		}
		return Sharpness;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelCameraStudyCommand, TSharedRef<FeelCameraStudy::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelCameraStudyCommand::Update()
{
	using namespace FeelCameraStudy;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	ACombatCharacter* Character = Controller ? Cast<ACombatCharacter>(Controller->GetPawn()) : nullptr;

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Character || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("CAMERA Play In Editor did not start with a combat character"));
				return true;
			}
			return false;
		}
		AActor* Nearest = nullptr;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetClass()->GetName().Contains(TEXT("CombatDummy")) && (!Nearest || FVector::Dist(It->GetActorLocation(), Character->GetActorLocation()) < FVector::Dist(Nearest->GetActorLocation(), Character->GetActorLocation())))
			{
				Nearest = *It;
			}
		}
		if (!Nearest)
		{
			Test->AddError(TEXT("CAMERA no training dummy in this level"));
			GEditor->RequestEndPlayMap();
			return true;
		}
		Run->Dummy = Nearest;
		Run->Results.SetNum(Run->Setups.Num());
		if (FParse::Param(FCommandLine::Get(), TEXT("FeelCameraFeelOff")))
		{
			UFeelBlueprintLibrary::SetFeelEnabled(false);
			Test->AddInfo(TEXT("CAMERA feel switched off for this run"));
		}
		Test->AddInfo(FString::Printf(TEXT("CAMERA measuring %d setups at %s"), Run->Setups.Num(), *Nearest->GetName()));
		Run->Stage = 1;
		Run->StageStart = Now;
		return false;
	}

	case 1:
	{
		// Next setup: apply it, stand in front of the dummy, let the camera lag settle and the dummy stop rocking.
		if (!Character || !Run->Dummy.IsValid())
		{
			Test->AddError(TEXT("CAMERA the character or the dummy is gone"));
			GEditor->RequestEndPlayMap();
			Run->Stage = 3;
			Run->StageStart = Now;
			return false;
		}
		if (!Run->bApplied)
		{
			Apply(*Character, Run->Setups[Run->SetupIndex]);
			StandInFront(*Character, *Controller, *Run->Dummy);
			Run->bApplied = true;
		}
		if (Elapsed < 1.6)
		{
			return false;
		}
		Run->bApplied = false;
		Run->Presses = 0;
		Run->Shots = 0;
		Run->Stage = 2;
		Run->StageStart = Now;
		Run->Results[Run->SetupIndex].StartTime = Now;
		return false;
	}

	case 2:
	{
		// The combo: three presses, each while the previous swing can still continue it. Measure every frame.
		const FSetup& Setup = Run->Setups[Run->SetupIndex];
		if (Character && Elapsed > 0.1 + 0.55 * Run->Presses && Run->Presses < 3)
		{
			Character->DoComboAttackStart();
			Character->DoComboAttackEnd();
			++Run->Presses;
		}
		if (Character && Controller && Run->Dummy.IsValid() && Elapsed > 0.1)
		{
			Measure(Run->Results[Run->SetupIndex], *Controller, *Character, *Run->Dummy);
		}
		// Pictures: a frame 0.15 s before each impact and one at it, so the impact's blur is compared with the same view.
		if (Run->bPictures && Run->Shots < 6)
		{
			const int32 Impact = Run->Shots / 2;
			const bool bBefore = Run->Shots % 2 == 0;
			if (Elapsed > 0.1 + 0.48 + 0.55 * Impact - (bBefore ? 0.15 : 0.0))
			{
				const FString Label = FString::Printf(TEXT("%d_%s"), Impact, bBefore ? TEXT("before") : TEXT("impact"));
				const double Sharpness = Shot(*Run, Test, Setup, bBefore ? nullptr : *Label);
				if (bBefore)
				{
					Run->SharpnessBefore = Sharpness;
				}
				else
				{
					Test->AddInfo(FString::Printf(TEXT("CAMERA impact %d keeps %.0f%% of the sharpness of the frame 0.15 s before it"), Impact, Run->SharpnessBefore > 0.0 ? 100.0 * Sharpness / Run->SharpnessBefore : 0.0));
				}
				++Run->Shots;
			}
		}
		if (Elapsed < 2.3)
		{
			return false;
		}
		const FSetupResult& Result = Run->Results[Run->SetupIndex];
		const double Frames = FMath::Max(Result.Frames, 1);
		Test->AddInfo(FString::Printf(TEXT("CAMERA setup arm %.0f side %.0f height %.0f fov %.0f: dummy visible %.0f%% (worst frame %.0f%%), behind the character %.0f%%, off screen %.0f%%, character %.0f%% of screen height, dummy %.0f%%, camera %.0f cm from the character, FeelKit turns the view up to %.1f degrees at up to %.0f degrees per second, knocks the target %.1f%% of the screen width, %d frames"),
			Setup.Arm, Setup.Side, Setup.Height, Setup.FieldOfView,
			100.0 * Result.VisibleSum / Frames, 100.0 * Result.WorstVisible, 100.0 * Result.BehindSum / Frames, 100.0 * Result.OffScreenSum / Frames,
			100.0 * Result.PlayerHeightSum / Frames, 100.0 * Result.DummyHeightSum / Frames, Result.ArmSum / Frames, Result.MaxTurn, Result.MaxTurnSpeed, 100.0 * Result.MaxKnock, Result.Frames));
		Test->AddInfo(FString::Printf(TEXT("CAMERA largest frame-to-frame move of the target %.2f%% of the screen width at %.3f s; frames:%s"), 100.0 * Result.MaxKnockStep, Result.MaxKnockStepTime - Result.StartTime, *Result.SnapLog.Left(6000)));
		Test->AddInfo(FString::Printf(TEXT("CAMERA longest frame during the combo %.0f ms; frames:%s"), Result.LongestFrame * 1000.0, *Result.TurnCurve.Left(1500)));
		++Run->SetupIndex;
		Run->Presses = 0;
		Run->Stage = Run->SetupIndex < Run->Setups.Num() ? 1 : 3;
		Run->StageStart = Now;
		if (Run->Stage == 3)
		{
			GEditor->RequestEndPlayMap();
		}
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCameraStudyDiagnostic, "DiagFeel.ARPGCameraStudy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelCameraStudyDiagnostic::RunTest(const FString& Parameters)
{
	using namespace FeelCameraStudy;
	const TSharedRef<FRun> Run = MakeShared<FRun>();
	FString SetupText;
	if (FParse::Value(FCommandLine::Get(), TEXT("FeelCameraSetups="), SetupText, false) && !SetupText.IsEmpty())
	{
		Run->Setups = ParseSetups(SetupText);
		// Saving a picture stalls the frame it is taken in (about 250 ms), so timing measurements run without them.
		Run->bPictures = !FParse::Param(FCommandLine::Get(), TEXT("FeelCameraNoPictures"));
	}
	if (Run->Setups.Num() == 0)
	{
		Run->Setups = Grid();
	}

	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelCameraStudyCommand(Run, this));
	return true;
}

#endif
