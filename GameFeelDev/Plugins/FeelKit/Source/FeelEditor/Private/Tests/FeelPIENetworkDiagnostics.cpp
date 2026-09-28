// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelRecipe.h"
#include "FeelReplicationComponent.h"
#include "FeelSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "InputKeyEventArgs.h"
#include "Components/SkeletalMeshComponent.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"

/**
 * Diagnostics in the setup users test with: Play In Editor, 2 players, listen server, one process. Measures on every PIE
 * world what a networked play does (plays, flash, camera, scale of each pawn's capsule and mesh). Not part of the FeelKit
 * suite (filter DiagFeel); needs the host project's /Game/FeelKitTests/R_Hit recipe.
 */
namespace FeelPIENetworkDiagnostics
{
	struct FWorldStats
	{
		int32 MaxInstances = 0;
		float MaxFlash = 0.0f;
		bool bCamera = false;
		TMap<FString, float> MaxScale;
	};

	struct FRun
	{
		int32 Mode = 0;
		bool bTriggerOnServer = true;
		float Relevancy = 0.0f;
		bool bFar = false;
		bool bPressKey = false;
		int32 Stage = 0;
		double StageStart = 0.0;
		TMap<FString, FWorldStats> Stats;
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

	FString WorldName(UWorld* World)
	{
		return World->GetNetMode() == NM_ListenServer ? TEXT("Server") : (World->GetNetMode() == NM_Client ? TEXT("Client") : TEXT("Other"));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPIENetworkCommand, TSharedRef<FeelPIENetworkDiagnostics::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPIENetworkCommand::Update()
{
	using namespace FeelPIENetworkDiagnostics;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	TArray<UWorld*> Worlds = PIEWorlds();

	auto NextStage = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
	};

	switch (Run->Stage)
	{
	case 0:
	{
		// Wait for both worlds with two pawns each; the server gives every pawn a Feel Replication component.
		int32 Ready = 0;
		for (UWorld* World : Worlds)
		{
			int32 PawnsWithComponent = 0;
			for (TActorIterator<APawn> It(World); It; ++It)
			{
				if (!Run->bPressKey && World->GetNetMode() == NM_ListenServer && !It->FindComponentByClass<UFeelReplicationComponent>())
				{
					UFeelReplicationComponent* Component = NewObject<UFeelReplicationComponent>(*It, TEXT("FeelDiagReplication"));
					Component->RegisterComponent();
					It->AddInstanceComponent(Component);
				}
				PawnsWithComponent += It->FindComponentByClass<UFeelReplicationComponent>() ? 1 : 0;
			}
			Ready += PawnsWithComponent >= 2 ? 1 : 0;
		}
		if (Worlds.Num() == 2 && Ready == 2 && Elapsed > 3.0)
		{
			if (Run->bFar)
			{
				// Move the player who does not trigger 30 m away, on the server so it replicates.
				for (UWorld* World : Worlds)
				{
					if (World->GetNetMode() != NM_ListenServer)
					{
						continue;
					}
					for (TActorIterator<APawn> It(World); It; ++It)
					{
						if (It->IsLocallyControlled() != Run->bTriggerOnServer)
						{
							It->SetActorLocation(It->GetActorLocation() + FVector(3000.0, 0.0, 0.0), false, nullptr, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
			NextStage();
		}
		else if (Elapsed > 90.0)
		{
			Test->AddError(FString::Printf(TEXT("PIEDIAG timeout: %d PIE worlds, %d ready"), Worlds.Num(), Ready));
			GEditor->RequestEndPlayMap();
			return true;
		}
		return false;
	}

	case 1:
	{
		if (Run->bFar && Elapsed < 2.0)
		{
			return false;
		}
		for (UWorld* World : Worlds)
		{
			TArray<APawn*> Pawns;
			for (TActorIterator<APawn> It(World); It; ++It)
			{
				Pawns.Add(*It);
			}
			if (Pawns.Num() == 2)
			{
				Test->AddInfo(FString::Printf(TEXT("PIEDIAG %s pawn distance %.0f"), *WorldName(World), FVector::Dist(Pawns[0]->GetActorLocation(), Pawns[1]->GetActorLocation())));
			}
		}
		for (UWorld* World : Worlds)
		{
			if ((World->GetNetMode() == NM_ListenServer) != Run->bTriggerOnServer)
			{
				continue;
			}
			APlayerController* Controller = World->GetFirstPlayerController();
			APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
			if (Run->bPressKey && Controller)
			{
				// The user's path: the K key event of the character Blueprint.
				Controller->InputKey(FInputKeyEventArgs(nullptr, FInputDeviceId::CreateFromInternalId(0), EKeys::K, IE_Pressed, 1.0f, false, FPlatformTime::Cycles64()));
				Test->AddInfo(FString::Printf(TEXT("PIEDIAG %s pressed K"), *WorldName(World)));
				continue;
			}
			UFeelReplicationComponent* Component = Pawn ? Pawn->FindComponentByClass<UFeelReplicationComponent>() : nullptr;
			UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Hit.R_Hit"));
			if (!Component || !Recipe)
			{
				Test->AddError(TEXT("PIEDIAG could not trigger (no component or R_Hit)"));
				GEditor->RequestEndPlayMap();
				return true;
			}
			const FFeelHandle Handle = Component->PlayFeelNetworked(Recipe, FFeelTarget::FromActor(Pawn), 1.0f, FFeelPlayContext(), static_cast<EFeelNetMode>(Run->Mode), Run->Relevancy);
			Test->AddInfo(FString::Printf(TEXT("PIEDIAG %s triggered mode %d relevancy %.0f on %s, local handle valid %d"), *WorldName(World), Run->Mode, Run->Relevancy, *Pawn->GetName(), Handle.IsValid() ? 1 : 0));
		}
		NextStage();
		return false;
	}

	case 2:
	{
		for (UWorld* World : Worlds)
		{
			FWorldStats& Stats = Run->Stats.FindOrAdd(WorldName(World));
			if (const UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>())
			{
				Stats.MaxInstances = FMath::Max(Stats.MaxInstances, Subsystem->GetNumActiveInstances());
				Stats.MaxFlash = FMath::Max(Stats.MaxFlash, Subsystem->GetScreenOutput().FlashAlpha);
				FFeelFrameOutput Camera;
				if (World->GetFirstPlayerController() && Subsystem->GetCameraOutput(World->GetFirstPlayerController(), Camera))
				{
					Stats.bCamera |= !Camera.CameraRotationOffset.IsNearlyZero() || !Camera.CameraLocationOffset.IsNearlyZero();
				}
			}
			for (TActorIterator<ACharacter> It(World); It; ++It)
			{
				const FString Key = It->IsLocallyControlled() ? TEXT("localPawn") : TEXT("remotePawn");
				const float Z = static_cast<float>(It->GetActorLocation().Z);
				float& MinZ = Stats.MaxScale.FindOrAdd(Key + TEXT(".minZ"), Z);
				MinZ = FMath::Min(MinZ, Z);
				float& MaxZ = Stats.MaxScale.FindOrAdd(Key + TEXT(".maxZ"), Z);
				MaxZ = FMath::Max(MaxZ, Z);
				float& Actor = Stats.MaxScale.FindOrAdd(Key + TEXT(".actor"));
				Actor = FMath::Max(Actor, static_cast<float>(It->GetActorScale3D().X));
				float& Root = Stats.MaxScale.FindOrAdd(Key + TEXT(".root"));
				Root = FMath::Max(Root, static_cast<float>(It->GetRootComponent()->GetRelativeScale3D().X));
				float& Mesh = Stats.MaxScale.FindOrAdd(Key + TEXT(".mesh"));
				Mesh = FMath::Max(Mesh, static_cast<float>(It->GetMesh()->GetRelativeScale3D().X));
			}
		}
		if (Run->bPressKey && Elapsed > 0.3 && Elapsed < 0.5)
		{
			for (UWorld* World : Worlds)
			{
				if ((World->GetNetMode() == NM_ListenServer) == Run->bTriggerOnServer && World->GetFirstPlayerController())
				{
					World->GetFirstPlayerController()->InputKey(FInputKeyEventArgs(nullptr, FInputDeviceId::CreateFromInternalId(0), EKeys::K, IE_Released, 0.0f, false, FPlatformTime::Cycles64()));
				}
			}
		}
		if (Elapsed < 4.0)
		{
			return false;
		}
		for (const TPair<FString, FWorldStats>& Pair : Run->Stats)
		{
			FString Scales;
			for (const TPair<FString, float>& Scale : Pair.Value.MaxScale)
			{
				Scales += FString::Printf(TEXT(" %s=%.2f"), *Scale.Key, Scale.Value);
			}
			Test->AddInfo(FString::Printf(TEXT("PIEDIAG RESULT %s mode=%d triggerOnServer=%d relevancy=%.0f instances=%d flash=%.2f camera=%d%s"),
				*Pair.Key, Run->Mode, Run->bTriggerOnServer ? 1 : 0, Run->Relevancy, Pair.Value.MaxInstances, Pair.Value.MaxFlash, Pair.Value.bCamera ? 1 : 0, *Scales));
		}
		GEditor->RequestEndPlayMap();
		NextStage();
		return false;
	}

	default:
		return Elapsed > 3.0;
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFeelPIENetworkDiagnostic, "DiagFeel.PIENetwork", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FFeelPIENetworkDiagnostic::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	for (int32 Mode = 0; Mode < 3; ++Mode)
	{
		for (const TCHAR* Trigger : { TEXT("server"), TEXT("client") })
		{
			OutBeautifiedNames.Add(FString::Printf(TEXT("Mode%d_%s"), Mode, Trigger));
			OutTestCommands.Add(FString::Printf(TEXT("%d %s 0"), Mode, Trigger));
		}
	}
	for (const TCHAR* Trigger : { TEXT("server"), TEXT("client") })
	{
		OutBeautifiedNames.Add(FString::Printf(TEXT("RelevancyNear_%s"), Trigger));
		OutTestCommands.Add(FString::Printf(TEXT("0 %s 1000 near"), Trigger));
		OutBeautifiedNames.Add(FString::Printf(TEXT("RelevancyFar_%s"), Trigger));
		OutTestCommands.Add(FString::Printf(TEXT("0 %s 1000 far"), Trigger));
	}
	for (const TCHAR* Trigger : { TEXT("server"), TEXT("client") })
	{
		OutBeautifiedNames.Add(FString::Printf(TEXT("KeyK_%s"), Trigger));
		OutTestCommands.Add(FString::Printf(TEXT("1 %s 300 near key"), Trigger));
	}
}

bool FFeelPIENetworkDiagnostic::RunTest(const FString& Parameters)
{
	TArray<FString> Parts;
	Parameters.ParseIntoArrayWS(Parts);
	const TSharedRef<FeelPIENetworkDiagnostics::FRun> Run = MakeShared<FeelPIENetworkDiagnostics::FRun>();
	Run->Mode = Parts.IsValidIndex(0) ? FCString::Atoi(*Parts[0]) : 0;
	Run->bTriggerOnServer = !Parts.IsValidIndex(1) || Parts[1] == TEXT("server");
	Run->Relevancy = Parts.IsValidIndex(2) ? FCString::Atof(*Parts[2]) : 0.0f;
	Run->bFar = Parts.IsValidIndex(3) && Parts[3] == TEXT("far");
	Run->bPressKey = Parts.IsValidIndex(4) && Parts[4] == TEXT("key");

	ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
	PlaySettings->SetPlayNumberOfClients(2);
	PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_ListenServer);
	PlaySettings->SetRunUnderOneProcess(true);
	PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;

	FRequestPlaySessionParams Params;
	Params.EditorPlaySettings = PlaySettings;
	Params.WorldType = EPlaySessionWorldType::PlayInEditor;
	GEditor->RequestPlaySession(Params);

	ADD_LATENT_AUTOMATION_COMMAND(FFeelPIENetworkCommand(Run, this));
	return true;
}

#endif
