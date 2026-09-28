// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "FeelGalleryShots.h"
#include "FeelRecipe.h"
#include "FeelSwitch.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "IMessageLogListing.h"
#include "ImageUtils.h"
#include "Logging/TokenizedMessage.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MessageLogModule.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostic (filter DiagFeel): plays the Action/RPG demo like a player. Start the editor on /Game/Variant_Combat/Lvl_Combat,
 * then this starts Play In Editor, stands the character in front of the training dummy, presses the real attack button
 * for the three-hit combo, and records which recipes start, whether the dummy flashes, and pictures of the impacts
 * (Saved/FeelKit/ARPG_<n>.png). Then it finishes an enemy with a combo and reports every Play In Editor warning, so
 * effects still playing when an enemy turns into a ragdoll are checked too.
 */
namespace FeelARPGPlaythrough
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		int32 Presses = 0;
		int32 Shots = 0;
		bool bSawFlash = false;
		TArray<FString> Started;
		FDelegateHandle StartedHandle;
		TWeakObjectPtr<AActor> Dummy;
		TWeakObjectPtr<AActor> Enemy;
		double EnemyFound = 0.0;
		bool bWalkedIntoArena = false;
		int32 GalleryPass = 0;
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

	/** Reads the game's own viewport, so the picture shows what the player sees. */
	void Shot(FRun& Run, FAutomationTestBase* Test, const TCHAR* Label)
	{
		UWorld* World = PIEWorld();
		const FWorldContext* Context = World ? GEngine->GetWorldContextFromWorld(World) : nullptr;
		FViewport* Viewport = Context && Context->GameViewport ? Context->GameViewport->Viewport : nullptr;
		TArray<FColor> Pixels;
		if (!Viewport || !Viewport->ReadPixels(Pixels) || Pixels.Num() == 0)
		{
			Test->AddInfo(TEXT("ARPG picture: the game viewport could not be read"));
			return;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FIntPoint Size = Viewport->GetSizeXY();
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("ARPG_%d_%s.png"), Run.Shots++, Label);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("ARPG picture %s"), *File));
	}

	bool DummyFlashes(const AActor* Dummy)
	{
		if (!Dummy)
		{
			return false;
		}
		TInlineComponentArray<UMeshComponent*> Meshes(Dummy);
		for (const UMeshComponent* Mesh : Meshes)
		{
			if (Cast<UMaterialInstanceDynamic>(Mesh->GetOverlayMaterial()))
			{
				return true;
			}
		}
		return false;
	}

	/** Puts the pawn in front of Target, facing it. */
	void StandInFront(APawn* Pawn, APlayerController* Controller, const AActor* Target)
	{
		const FVector Toward = (Pawn->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
		const FVector Stand = Target->GetActorLocation() + Toward * 110.0f + FVector(0.0, 0.0, 20.0);
		const FRotator Facing = (-Toward).Rotation();
		Pawn->SetActorLocationAndRotation(Stand, Facing, false, nullptr, ETeleportType::TeleportPhysics);
		Controller->SetControlRotation(FRotator(-15.0, Facing.Yaw, 0.0));
	}

	/** True once any of the actor's bodies is simulated by physics (the template's enemies turn into ragdolls when they die). */
	bool IsRagdoll(const AActor* Actor)
	{
		TInlineComponentArray<UPrimitiveComponent*> Primitives(Actor);
		for (const UPrimitiveComponent* Primitive : Primitives)
		{
			if (Primitive->IsSimulatingPhysics())
			{
				return true;
			}
		}
		return false;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelARPGPlaythroughCommand, TSharedRef<FeelARPGPlaythrough::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelARPGPlaythroughCommand::Update()
{
	using namespace FeelARPGPlaythrough;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

	if (Run->Dummy.IsValid() && DummyFlashes(Run->Dummy.Get()))
	{
		Run->bSawFlash = true;
	}

	// The template's attack input calls DoComboAttackStart / DoComboAttackEnd on the character; call them the same way.
	auto CallOnPawn = [Pawn](const TCHAR* FunctionName)
	{
		if (UFunction* Function = Pawn ? Pawn->FindFunction(FName(FunctionName)) : nullptr)
		{
			Pawn->ProcessEvent(Function, nullptr);
			return true;
		}
		return false;
	};

	if (Run->Stage < 4 && Run->Stage > 0 && !Pawn)
	{
		Test->AddError(TEXT("ARPG the player's pawn is gone"));
		GEditor->RequestEndPlayMap();
		Run->Stage = 4;
		Run->StageStart = Now;
		return false;
	}

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Pawn || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("ARPG Play In Editor did not start"));
				return true;
			}
			return false;
		}
		// The training dummy nearest to the player.
		AActor* Nearest = nullptr;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetClass()->GetName().Contains(TEXT("CombatDummy")) && (!Nearest || FVector::Dist(It->GetActorLocation(), Pawn->GetActorLocation()) < FVector::Dist(Nearest->GetActorLocation(), Pawn->GetActorLocation())))
			{
				Nearest = *It;
			}
		}
		if (!Nearest)
		{
			Test->AddError(TEXT("ARPG no training dummy in this level"));
			return true;
		}
		Run->Dummy = Nearest;
		StandInFront(Pawn, Controller, Nearest);

		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			const double Start = Now;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef, Start](FFeelHandle, UFeelRecipe* Recipe)
			{
				RunRef->Started.Add(FString::Printf(TEXT("%.2fs %s"), FPlatformTime::Seconds() - Start, Recipe ? *Recipe->GetName() : TEXT("?")));
			});
		}
		int32 Switches = 0;
		FString SwitchTitle;
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			++Switches;
			SwitchTitle = It->StartCardTitle.ToString();
		}
		Test->AddInfo(FString::Printf(TEXT("ARPG Feel Switches in the level: %d (start card '%s'), feel %s"), Switches, *SwitchTitle, UFeelBlueprintLibrary::IsFeelEnabled() ? TEXT("on") : TEXT("off")));
		Test->AddInfo(FString::Printf(TEXT("ARPG standing in front of %s"), *Nearest->GetName()));
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	case 1:
		// Three presses, each while the previous swing can still continue the combo.
		if (Elapsed > 0.5 + 0.55 * Run->Presses && Run->Presses < 3)
		{
			const bool bCalled = CallOnPawn(TEXT("DoComboAttackStart")) && CallOnPawn(TEXT("DoComboAttackEnd"));
			if (Run->Presses == 0)
			{
				Test->AddInfo(FString::Printf(TEXT("ARPG attack on %s: %s"), *Pawn->GetClass()->GetName(), bCalled ? TEXT("called") : TEXT("no DoComboAttackStart on this pawn")));
			}
			++Run->Presses;
		}
		// Pictures shortly after each expected impact (the attack traces are about 0.47 s into each swing).
		if (Run->Shots < 3 && Elapsed > 0.5 + 0.47 + 0.01 + 0.55 * Run->Shots)
		{
			FeelGalleryShots::Take(World, TEXT("ARPG"), FString::Printf(TEXT("%s_%d"), Run->GalleryPass == 0 ? TEXT("On") : TEXT("Off"), Run->Shots), Test);
			Shot(*Run, Test, TEXT("impact"));
		}
		if (Elapsed < 4.0)
		{
			return false;
		}
		Shot(*Run, Test, TEXT("after"));
		// Gallery pictures: the same combo again with FeelKit switched off, for a side by side.
		if (FeelGalleryShots::IsOn() && Run->GalleryPass == 0)
		{
			Run->GalleryPass = 1;
			UFeelBlueprintLibrary::SetFeelEnabled(false);
			StandInFront(Pawn, Controller, Run->Dummy.Get());
			Run->Presses = 0;
			Run->Shots = 0;
			Run->StageStart = Now;
			return false;
		}
		UFeelBlueprintLibrary::SetFeelEnabled(true);
		for (const FString& Line : Run->Started)
		{
			Test->AddInfo(FString::Printf(TEXT("ARPG started %s"), *Line));
		}
		Test->AddInfo(FString::Printf(TEXT("ARPG dummy flashed: %s"), Run->bSawFlash ? TEXT("yes") : TEXT("no")));
		Run->Started.Reset();
		Run->Presses = 0;
		++Run->Stage;
		Run->StageStart = Now;
		return false;

	case 2:
	{
		// Finish the nearest enemy with a combo (three hits of 1 against 3 HP).
		if (!Run->Enemy.IsValid())
		{
			for (TActorIterator<ACharacter> It(World); It; ++It)
			{
				if (It->GetClass()->GetName().Contains(TEXT("CombatEnemy")) && !IsRagdoll(*It)
					&& (!Run->Enemy.IsValid() || FVector::Dist(It->GetActorLocation(), Pawn->GetActorLocation()) < FVector::Dist(Run->Enemy->GetActorLocation(), Pawn->GetActorLocation())))
				{
					Run->Enemy = *It;
				}
			}
			if (!Run->Enemy.IsValid() && !Run->bWalkedIntoArena)
			{
				// The level's spawners wait until the player walks into an activation area; walk into the nearest one.
				AActor* Area = nullptr;
				for (TActorIterator<AActor> It(World); It; ++It)
				{
					if (It->GetClass()->GetName().Contains(TEXT("ActivationVolume")) && (!Area || FVector::Dist(It->GetActorLocation(), Pawn->GetActorLocation()) < FVector::Dist(Area->GetActorLocation(), Pawn->GetActorLocation())))
					{
						Area = *It;
					}
				}
				Run->bWalkedIntoArena = true;
				if (Area)
				{
					Pawn->SetActorLocation(Area->GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
					Test->AddInfo(FString::Printf(TEXT("ARPG walked into %s"), *Area->GetName()));
				}
			}
			if (!Run->Enemy.IsValid() && Elapsed < 15.0)
			{
				return false;
			}
			if (!Run->Enemy.IsValid())
			{
				Test->AddInfo(TEXT("ARPG no enemy in this level"));
				GEditor->RequestEndPlayMap();
				Run->Stage = 4;
				Run->StageStart = Now;
				return false;
			}
			Test->AddInfo(FString::Printf(TEXT("ARPG finishing %s"), *Run->Enemy->GetName()));
			Run->EnemyFound = Elapsed;
		}
		const double Fighting = Elapsed - Run->EnemyFound;
		const bool bDown = !Run->Enemy.IsValid() || IsRagdoll(Run->Enemy.Get());
		if (!bDown && Fighting > 0.5 + 0.55 * Run->Presses && Run->Presses < 9)
		{
			StandInFront(Pawn, Controller, Run->Enemy.Get());
			CallOnPawn(TEXT("DoComboAttackStart"));
			CallOnPawn(TEXT("DoComboAttackEnd"));
			++Run->Presses;
		}
		if (bDown || Fighting > 8.0)
		{
			Test->AddInfo(FString::Printf(TEXT("ARPG enemy down: %s after %d presses"), bDown ? TEXT("yes") : TEXT("no"), Run->Presses));
			++Run->Stage;
			Run->StageStart = Now;
		}
		return false;
	}

	case 3:
	{
		// Let the death recipe and anything still playing on the ragdoll run out, then read the Play In Editor warnings.
		if (Elapsed < 3.0)
		{
			return false;
		}
		for (const FString& Line : Run->Started)
		{
			Test->AddInfo(FString::Printf(TEXT("ARPG started %s"), *Line));
		}
		FMessageLogModule& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
		int32 Warnings = 0;
		for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.GetLogListing(TEXT("PIE"))->GetFilteredMessages())
		{
			if (Message->GetSeverity() <= EMessageSeverity::Warning)
			{
				++Warnings;
				Test->AddInfo(FString::Printf(TEXT("ARPG PIE message: %s"), *Message->ToText().ToString()));
			}
		}
		Test->AddInfo(FString::Printf(TEXT("ARPG PIE warnings and errors: %d"), Warnings));
		GEditor->RequestEndPlayMap();
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelARPGPlaythroughDiagnostic, "DiagFeel.ARPGPlaythrough", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelARPGPlaythroughDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelARPGPlaythroughCommand(MakeShared<FeelARPGPlaythrough::FRun>(), this));
	return true;
}

#endif
