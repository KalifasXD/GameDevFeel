// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelRecipe.h"
#include "FeelReplicationComponent.h"
#include "FeelSettings.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostics (filter DiagFeel): networked Global Hitstop (R_Freeze, Mode Everyone) in a 2-player listen server PIE, with
 * and without Allow Global Time Dilation In Multiplayer. Both players walk forward the whole time; per world it records world
 * time dilation, each pawn's custom time dilation and how far each pawn moves before, during and after the hitstop.
 */
namespace FeelPIETimeDilationDiagnostics
{
	struct FPawnTrack
	{
		FVector Last = FVector::ZeroVector;
		bool bHasLast = false;
		double Before = 0.0;
		double During = 0.0;
		double After = 0.0;
		float MinCustom = 1.0f;
		float MaxCorrection = 0.0f;
	};

	struct FWorldTrack
	{
		float MinWorldDilation = 1.0f;
		float EndWorldDilation = 1.0f;
		TMap<FString, FPawnTrack> Pawns;
	};

	struct FRun
	{
		bool bAllowGlobal = false;
		bool bTriggerOnServer = true;
		bool bPreviousAllow = false;
		int32 Stage = 0;
		double StageStart = 0.0;
		double TriggerTime = 0.0;
		TMap<FString, FWorldTrack> Worlds;
	};

	TArray<UWorld*> PIEWorlds()
	{
		TArray<UWorld*> Worlds;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				Worlds.Add(Context.World());
			}
		}
		return Worlds;
	}

	FString WorldName(const UWorld* World)
	{
		return World->GetNetMode() == NM_ListenServer ? TEXT("Server") : TEXT("Client");
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPIETimeDilationCommand, TSharedRef<FeelPIETimeDilationDiagnostics::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPIETimeDilationCommand::Update()
{
	using namespace FeelPIETimeDilationDiagnostics;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	TArray<UWorld*> Worlds = PIEWorlds();

	// Every locally controlled pawn walks back and forth (direction flips every 0.75 s of real time), so no wall stops it.
	const double Direction = FMath::Fmod(Now, 1.5) < 0.75 ? 1.0 : -1.0;
	for (UWorld* World : Worlds)
	{
		for (TActorIterator<APawn> It(World); It; ++It)
		{
			if (It->IsLocallyControlled())
			{
				It->AddMovementInput(FVector(Direction, 0.0, 0.0), 1.0f);
			}
		}
	}

	switch (Run->Stage)
	{
	case 0:
	{
		int32 PawnCount = 0;
		for (UWorld* World : Worlds)
		{
			for (TActorIterator<APawn> It(World); It; ++It)
			{
				++PawnCount;
			}
		}
		if (Worlds.Num() == 2 && PawnCount == 4 && Elapsed > 4.0)
		{
			++Run->Stage;
			Run->StageStart = Now;
		}
		else if (Elapsed > 90.0)
		{
			Test->AddError(TEXT("TDDIAG PIE did not start"));
			GEditor->RequestEndPlayMap();
			return true;
		}
		return false;
	}

	case 1:
	{
		// Measure 1 s before, trigger, 1 s during (the hitstop lasts 1 s), 2 s after.
		const double RelativeToTrigger = Run->TriggerTime > 0.0 ? Now - Run->TriggerTime : -1.0;
		if (Run->TriggerTime == 0.0 && Elapsed > 1.0)
		{
			for (UWorld* World : Worlds)
			{
				if ((World->GetNetMode() == NM_ListenServer) != Run->bTriggerOnServer)
				{
					continue;
				}
				APawn* Pawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
				UFeelReplicationComponent* Component = Pawn ? Pawn->FindComponentByClass<UFeelReplicationComponent>() : nullptr;
				UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Freeze.R_Freeze"));
				if (!Component || !Recipe)
				{
					Test->AddError(TEXT("TDDIAG no Feel Replication component or R_Freeze"));
					GEditor->RequestEndPlayMap();
					return true;
				}
				Component->PlayFeelNetworked(Recipe, FFeelTarget::FromActor(Pawn), 1.0f, FFeelPlayContext(), EFeelNetMode::Everyone, 0.0f);
				Test->AddInfo(FString::Printf(TEXT("TDDIAG %s triggered R_Freeze, allow global %d"), *WorldName(World), Run->bAllowGlobal ? 1 : 0));
			}
			Run->TriggerTime = Now;
		}

		for (UWorld* World : Worlds)
		{
			FWorldTrack& Track = Run->Worlds.FindOrAdd(WorldName(World));
			const float WorldDilation = World->GetWorldSettings()->TimeDilation;
			if (RelativeToTrigger >= 0.0 && RelativeToTrigger < 1.2)
			{
				Track.MinWorldDilation = FMath::Min(Track.MinWorldDilation, WorldDilation);
			}
			Track.EndWorldDilation = WorldDilation;

			for (TActorIterator<APawn> It(World); It; ++It)
			{
				const FString Key = It->IsLocallyControlled() ? TEXT("localPawn") : TEXT("remotePawn");
				FPawnTrack& Pawn = Track.Pawns.FindOrAdd(Key);
				const FVector Location = It->GetActorLocation();
				if (Pawn.bHasLast)
				{
					const double Step = FVector::Dist2D(Location, Pawn.Last);
					if (RelativeToTrigger < 0.0)
					{
						Pawn.Before += Step;
					}
					else if (RelativeToTrigger < 1.0)
					{
						Pawn.During += Step;
					}
					else if (RelativeToTrigger >= 1.0 && RelativeToTrigger < 2.0)
					{
						Pawn.After += Step;
					}
					Pawn.MaxCorrection = FMath::Max(Pawn.MaxCorrection, static_cast<float>(Step));
				}
				Pawn.Last = Location;
				Pawn.bHasLast = true;
				if (RelativeToTrigger >= 0.0 && RelativeToTrigger < 1.2)
				{
					Pawn.MinCustom = FMath::Min(Pawn.MinCustom, It->CustomTimeDilation);
				}
			}
		}

		if (RelativeToTrigger < 3.0)
		{
			return false;
		}

		for (const TPair<FString, FWorldTrack>& World : Run->Worlds)
		{
			FString Pawns;
			for (const TPair<FString, FPawnTrack>& Pawn : World.Value.Pawns)
			{
				Pawns += FString::Printf(TEXT(" | %s moved before(1s)=%.0f during(1s)=%.0f after(next 1s)=%.0f minCustomDilation=%.2f maxFrameStep=%.0f"),
					*Pawn.Key, Pawn.Value.Before, Pawn.Value.During, Pawn.Value.After, Pawn.Value.MinCustom, Pawn.Value.MaxCorrection);
			}
			Test->AddInfo(FString::Printf(TEXT("TDDIAG RESULT allow=%d triggerOnServer=%d %s minWorldDilation=%.3f endWorldDilation=%.2f%s"),
				Run->bAllowGlobal ? 1 : 0, Run->bTriggerOnServer ? 1 : 0, *World.Key, World.Value.MinWorldDilation, World.Value.EndWorldDilation, *Pawns));
		}
		GetMutableDefault<UFeelSettings>()->bAllowGlobalTimeDilationInMultiplayer = Run->bPreviousAllow;
		GEditor->RequestEndPlayMap();
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFeelPIETimeDilationDiagnostic, "DiagFeel.PIETimeDilation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FFeelPIETimeDilationDiagnostic::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	for (const TCHAR* Allow : { TEXT("local"), TEXT("global") })
	{
		for (const TCHAR* Trigger : { TEXT("server"), TEXT("client") })
		{
			OutBeautifiedNames.Add(FString::Printf(TEXT("%s_%s"), Allow, Trigger));
			OutTestCommands.Add(FString::Printf(TEXT("%s %s"), Allow, Trigger));
		}
	}
}

bool FFeelPIETimeDilationDiagnostic::RunTest(const FString& Parameters)
{
	TArray<FString> Parts;
	Parameters.ParseIntoArrayWS(Parts);
	const TSharedRef<FeelPIETimeDilationDiagnostics::FRun> Run = MakeShared<FeelPIETimeDilationDiagnostics::FRun>();
	Run->bAllowGlobal = Parts.IsValidIndex(0) && Parts[0] == TEXT("global");
	Run->bTriggerOnServer = !Parts.IsValidIndex(1) || Parts[1] == TEXT("server");
	Run->bPreviousAllow = GetDefault<UFeelSettings>()->bAllowGlobalTimeDilationInMultiplayer;
	GetMutableDefault<UFeelSettings>()->bAllowGlobalTimeDilationInMultiplayer = Run->bAllowGlobal;

	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(2);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_ListenServer);
	PlaySettings->SetRunUnderOneProcess(true);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	Params.WorldType = EPlaySessionWorldType::PlayInEditor;
	GEditor->RequestPlaySession(Params);

	ADD_LATENT_AUTOMATION_COMMAND(FFeelPIETimeDilationCommand(Run, this));
	return true;
}

#endif
