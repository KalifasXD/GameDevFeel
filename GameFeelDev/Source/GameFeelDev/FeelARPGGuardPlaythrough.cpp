// Diagnostic (filter DiagFeel): plays the Action/RPG demo's guards like a player. Start the editor on
// /Game/Variant_Combat/Lvl_Combat. Stands the player in front of the guard enemy (its own attacks switched off, so every
// attack here is one the test chose) and checks, by hit points as well as by the recipes that play:
// the player's combo meets the enemy's guard (blocks, no damage), a charged strike breaks it, hits land while it is
// broken, the player's guard blocks the enemy's combo, a guard raised just before the hit parries it, the enemy's
// charged strike breaks the player's guard, and without a guard the player is hit. Saves a picture at the first block,
// guard break and parry (Saved/FeelKit/Guard_<n>_<recipe>.png). Editor builds only.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "CombatCharacter.h"
#include "CombatGuardComponent.h"
#include "CombatGuardEnemy.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "IMessageLogListing.h"
#include "ImageUtils.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "UnrealClient.h"

namespace FeelGuardPlaythrough
{
	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		bool bActed = false;
		int32 Step = 0;
		int32 Shots = 0;
		TSet<FString> Pictured;
		FString PendingPicture;
		TMap<FString, int32> Played;
		TArray<FString> Timeline;
		double Start = 0.0;
		FDelegateHandle StartedHandle;
		TWeakObjectPtr<ACombatGuardEnemy> Enemy;
		float Mark = 0.0f;
		int32 CountMark = 0;
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

	int32 Count(const FRun& Run, const TCHAR* Recipe)
	{
		const int32* Found = Run.Played.Find(Recipe);
		return Found ? *Found : 0;
	}

	float PlayerHP(const ACombatCharacter& Player)
	{
		const FFloatProperty* Property = CastField<FFloatProperty>(ACombatCharacter::StaticClass()->FindPropertyByName(TEXT("CurrentHP")));
		return Property ? Property->GetPropertyValue_InContainer(&Player) : -1.0f;
	}

	/** Puts the player Distance in front of the enemy, both facing each other. */
	void FaceOff(ACombatCharacter& Player, APlayerController& Controller, ACombatGuardEnemy& Enemy, float Distance)
	{
		const FVector Forward = Enemy.GetActorForwardVector().GetSafeNormal2D();
		const FVector Stand = Enemy.GetActorLocation() + Forward * Distance;
		const FRotator TowardEnemy = (-Forward).Rotation();
		Player.SetActorLocationAndRotation(Stand, FRotator(0.0, TowardEnemy.Yaw, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
		Player.GetCharacterMovement()->Velocity = FVector::ZeroVector;
		Controller.SetControlRotation(FRotator(-12.0, TowardEnemy.Yaw, 0.0));
		Enemy.SetActorRotation(FRotator(0.0, Forward.Rotation().Yaw, 0.0));
	}

	void Shot(FRun& Run, FAutomationTestBase* Test, const FString& Label)
	{
		UWorld* World = PIEWorld();
		const FWorldContext* Context = World ? GEngine->GetWorldContextFromWorld(World) : nullptr;
		FViewport* Viewport = Context && Context->GameViewport ? Context->GameViewport->Viewport : nullptr;
		TArray<FColor> Pixels;
		if (!Viewport || !Viewport->ReadPixels(Pixels) || Pixels.Num() == 0)
		{
			return;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FIntPoint Size = Viewport->GetSizeXY();
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("Guard_%d_%s.png"), Run.Shots++, *Label);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("GUARD picture %s"), *File));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelGuardPlaythroughCommand, TSharedRef<FeelGuardPlaythrough::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelGuardPlaythroughCommand::Update()
{
	using namespace FeelGuardPlaythrough;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	UWorld* World = PIEWorld();
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	ACombatCharacter* Player = Controller ? Cast<ACombatCharacter>(Controller->GetPawn()) : nullptr;
	ACombatGuardEnemy* Enemy = Run->Enemy.Get();

	// A picture on the frame after the first block, guard break and parry started.
	if (!Run->PendingPicture.IsEmpty())
	{
		Shot(*Run, Test, Run->PendingPicture);
		Run->PendingPicture.Reset();
	}

	auto Next = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
		Run->bActed = false;
		Run->Step = 0;
	};
	auto Once = [this]()
	{
		if (Run->bActed)
		{
			return false;
		}
		Run->bActed = true;
		return true;
	};
	auto Check = [this](bool bPassed, const FString& What)
	{
		Test->AddInfo(FString::Printf(TEXT("GUARD %s %s"), bPassed ? TEXT("yes:") : TEXT("NO:"), *What));
		if (!bPassed)
		{
			Test->AddError(FString::Printf(TEXT("GUARD failed: %s"), *What));
		}
	};

	// From stage 8 on the enemy is meant to die and be removed.
	if (Run->Stage > 0 && Run->Stage < 8 && (!Player || !Enemy))
	{
		Test->AddError(TEXT("GUARD the player or the guard enemy is gone"));
		GEditor->RequestEndPlayMap();
		Run->Stage = 99;
		Run->StageStart = Now;
		return false;
	}

	switch (Run->Stage)
	{
	case 0:
	{
		if (!Player || Elapsed < 3.0)
		{
			if (Elapsed > 90.0)
			{
				Test->AddError(TEXT("GUARD Play In Editor did not start with the combat character"));
				return true;
			}
			return false;
		}
		for (TActorIterator<ACombatGuardEnemy> It(World); It; ++It)
		{
			Run->Enemy = *It;
			break;
		}
		if (!Run->Enemy.IsValid())
		{
			Test->AddError(TEXT("GUARD no guard enemy in this level"));
			GEditor->RequestEndPlayMap();
			return true;
		}
		// Only the attacks this test asks for.
		if (FBoolProperty* Own = CastField<FBoolProperty>(ACombatGuardEnemy::StaticClass()->FindPropertyByName(TEXT("bAttacksOnItsOwn"))))
		{
			Own->SetPropertyValue_InContainer(Run->Enemy.Get(), false);
		}
		if (UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
		{
			const TSharedRef<FRun> RunRef = Run;
			Run->Start = Now;
			Run->StartedHandle = Subsystem->OnFeelStarted.AddLambda([RunRef](FFeelHandle, UFeelRecipe* Recipe)
			{
				const FString Name = Recipe ? Recipe->GetName() : TEXT("?");
				RunRef->Played.FindOrAdd(Name)++;
				RunRef->Timeline.Add(FString::Printf(TEXT("%.2fs %s"), FPlatformTime::Seconds() - RunRef->Start, *Name));
				if ((Name.Contains(TEXT("Guard")) || Name.Contains(TEXT("Parry"))) && !RunRef->Pictured.Contains(Name))
				{
					RunRef->Pictured.Add(Name);
					RunRef->PendingPicture = Name;
				}
			});
		}
		const UCombatGuardComponent* PlayerGuard = Player->GetGuard();
		Check(PlayerGuard && PlayerGuard->BlockFeel && PlayerGuard->BreakFeel && PlayerGuard->ParryFeel, TEXT("the player's guard has its three recipes"));
		Check(Run->Enemy->GetGuard() && Run->Enemy->GetGuard()->BlockFeel && Run->Enemy->GetGuard()->BreakFeel, TEXT("the guard enemy's guard has its recipes"));
		Next();
		return false;
	}

	case 1:
		// The player's combo meets the raised guard.
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Run->Mark = Enemy->CurrentHP;
			Run->CountMark = Count(*Run, TEXT("FR_ARPG_GuardBlock"));
		}
		if (Run->Step < 3 && Elapsed > 0.6 + 0.55 * Run->Step)
		{
			Player->DoComboAttackStart();
			Player->DoComboAttackEnd();
			++Run->Step;
		}
		if (Elapsed < 3.2)
		{
			return false;
		}
		Check(Count(*Run, TEXT("FR_ARPG_GuardBlock")) > Run->CountMark, FString::Printf(TEXT("the combo is blocked (%d blocks)"), Count(*Run, TEXT("FR_ARPG_GuardBlock")) - Run->CountMark));
		Check(FMath::IsNearlyEqual(Enemy->CurrentHP, Run->Mark), FString::Printf(TEXT("the guarded enemy loses no hit points (%.0f to %.0f)"), Run->Mark, Enemy->CurrentHP));
		Check(Count(*Run, TEXT("FR_ARPG_HitLanded")) == 0, TEXT("no hit recipe plays for a blocked hit"));
		Next();
		return false;

	case 2:
		// A charged strike breaks the guard.
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Player->DoChargedAttackStart();
		}
		if (Run->Step == 0 && Elapsed > 1.2)
		{
			Player->DoChargedAttackEnd();
			Run->Step = 1;
		}
		if (Elapsed < 3.0 && Count(*Run, TEXT("FR_ARPG_GuardBreak")) == 0)
		{
			return false;
		}
		Check(Count(*Run, TEXT("FR_ARPG_GuardBreak")) >= 1, TEXT("the charged strike breaks the guard"));
		Check(Enemy->GetGuard()->IsStaggered(), TEXT("the enemy is open after its guard broke"));
		Next();
		return false;

	case 3:
		// While the guard is broken, hits land.
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Run->Mark = Enemy->CurrentHP;
			Player->DoComboAttackStart();
			Player->DoComboAttackEnd();
		}
		// The swing waits for the charged strike to finish, so wait for the hit rather than a fixed time.
		if (Elapsed < 3.0 && Count(*Run, TEXT("FR_ARPG_HitLanded")) == 0)
		{
			return false;
		}
		Check(Enemy->CurrentHP < Run->Mark, FString::Printf(TEXT("a hit on the broken guard deals damage (%.0f to %.0f)"), Run->Mark, Enemy->CurrentHP));
		Check(Count(*Run, TEXT("FR_ARPG_HitLanded")) >= 1, TEXT("and plays the normal hit recipe"));
		Next();
		return false;

	case 4:
		// Let the guard recover, then the player holds block against the enemy's combo.
		if (Elapsed < 2.0)
		{
			return false;
		}
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Player->DoBlockStart();
			Run->Mark = PlayerHP(*Player);
			Run->CountMark = Count(*Run, TEXT("FR_ARPG_GuardBlock"));
		}
		if (Run->Step == 0 && Elapsed > 2.6)
		{
			Enemy->DoAIComboAttack();
			Run->Step = 1;
		}
		if (Elapsed < 5.0)
		{
			return false;
		}
		Check(Count(*Run, TEXT("FR_ARPG_GuardBlock")) > Run->CountMark, TEXT("the player's guard blocks the enemy's combo"));
		Check(FMath::IsNearlyEqual(PlayerHP(*Player), Run->Mark), FString::Printf(TEXT("the guarding player loses no hit points (%.0f to %.0f)"), Run->Mark, PlayerHP(*Player)));
		Check(Count(*Run, TEXT("FR_ARPG_Parry")) == 0, TEXT("a guard held for a while blocks rather than parries"));
		Player->DoBlockEnd();
		Next();
		return false;

	case 5:
		// A guard raised just before the enemy's hit parries it. The first swing's hit comes about 0.47 s into the attack.
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Enemy->DoAIComboAttack();
		}
		if (Run->Step == 0 && Elapsed > 0.36)
		{
			Player->DoBlockStart();
			Run->Step = 1;
		}
		if (Elapsed < 1.5)
		{
			return false;
		}
		Check(Count(*Run, TEXT("FR_ARPG_Parry")) >= 1, TEXT("a guard raised just before the hit parries it"));
		Check(Enemy->GetGuard()->IsStaggered(), TEXT("the parried enemy is staggered"));
		Player->DoBlockEnd();
		Next();
		return false;

	case 6:
		// The enemy's charged strike against the player's guard.
		if (Elapsed < 1.8)
		{
			return false;
		}
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Player->DoBlockStart();
			Run->CountMark = Count(*Run, TEXT("FR_ARPG_GuardBreak"));
		}
		if (Run->Step == 0 && Elapsed > 2.4)
		{
			Enemy->DoAIChargedAttack();
			Run->Step = 1;
		}
		if (Elapsed < 9.0 && Count(*Run, TEXT("FR_ARPG_GuardBreak")) == Run->CountMark)
		{
			return false;
		}
		Check(Count(*Run, TEXT("FR_ARPG_GuardBreak")) > Run->CountMark, TEXT("the enemy's charged strike breaks the player's guard"));
		Check(Player->GetGuard()->IsStaggered(), TEXT("the player is open after the guard broke"));
		Player->DoBlockEnd();
		Next();
		return false;

	case 7:
		// No guard: the player is hit.
		if (Elapsed < 2.5)
		{
			return false;
		}
		if (Once())
		{
			FaceOff(*Player, *Controller, *Enemy, 150.0f);
			Run->Mark = PlayerHP(*Player);
			Enemy->DoAIComboAttack();
		}
		if (Elapsed < 4.0)
		{
			return false;
		}
		Check(PlayerHP(*Player) < Run->Mark, FString::Printf(TEXT("without a guard the player is hit (%.0f to %.0f)"), Run->Mark, PlayerHP(*Player)));
		Next();
		return false;

	case 8:
		// When the guard enemy dies, a fresh one takes its place.
		if (Once())
		{
			Enemy->ApplyDamage(100.0f, Player, Enemy->GetActorLocation(), FVector::ZeroVector);
		}
		if (Elapsed < 8.0)
		{
			return false;
		}
		{
			int32 Standing = 0;
			for (TActorIterator<ACombatGuardEnemy> It(World); It; ++It)
			{
				if (*It != Enemy && It->CurrentHP > 0.0f)
				{
					++Standing;
				}
			}
			Check(Standing == 1, FString::Printf(TEXT("a fresh guard enemy takes the place of the dead one (%d standing)"), Standing));
		}
		Run->Stage = 9;
		Run->StageStart = Now;
		return false;

	case 9:
	{
		for (const FString& Line : Run->Timeline)
		{
			Test->AddInfo(FString::Printf(TEXT("GUARD played %s"), *Line));
		}
		FMessageLogModule& MessageLog = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
		int32 Warnings = 0;
		for (const TSharedRef<FTokenizedMessage>& Message : MessageLog.GetLogListing(TEXT("PIE"))->GetFilteredMessages())
		{
			if (Message->GetSeverity() <= EMessageSeverity::Warning)
			{
				++Warnings;
				Test->AddInfo(FString::Printf(TEXT("GUARD PIE message: %s"), *Message->ToText().ToString()));
			}
		}
		Test->AddInfo(FString::Printf(TEXT("GUARD PIE warnings and errors: %d"), Warnings));
		GEditor->RequestEndPlayMap();
		Run->Stage = 99;
		Run->StageStart = Now;
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelGuardPlaythroughDiagnostic, "DiagFeel.ARPGGuardPlaythrough", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelGuardPlaythroughDiagnostic::RunTest(const FString& Parameters)
{
	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(1);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	GEditor->RequestPlaySession(Params);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelGuardPlaythroughCommand(MakeShared<FeelGuardPlaythrough::FRun>(), this));
	return true;
}

#endif
