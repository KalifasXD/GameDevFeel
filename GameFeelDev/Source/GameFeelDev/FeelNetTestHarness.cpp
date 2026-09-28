// Development harness for FeelKit checks (networking, decal rendering). Lives in the host project, not in the plugin.
// Usage (both processes): -ExecCmds="feeltest.net <Mode 0=Everyone 1=OwnerOnly 2=SkipOwner> <TriggerOn server|client> [RelevancyDistance]"
// Editor builds only: it runs through UnrealEditor-Cmd -game and uses editor-only material data, so packaged games skip it.

#if WITH_EDITOR

#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FeelRecipe.h"
#include "FeelReplicationComponent.h"
#include "FeelSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "UObject/UObjectIterator.h"
#include "Components/DecalComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "MaterialShared.h"
#include "Steps/FeelStep_Spawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogFeelNetTest, Log, All);

namespace FeelNetTestHarness
{
	struct FState
	{
		int32 Mode = 0;
		bool bTriggerOnServer = false;
		double StartTime = 0.0;
		double TriggerTime = -1.0;
		bool bTriggered = false;
		int32 MaxInstances = 0;
		float MaxFlash = 0.0f;
		float MaxScale = 1.0f;
		bool bCameraOutput = false;
		float RelevancyDistance = 0.0f;
		TMap<FString, float> MaxPawnScale;
		FString ReplicationInfo;
	};

	UWorld* FindGameWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game && Context.World())
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	const TCHAR* NetModeName(ENetMode Mode)
	{
		switch (Mode)
		{
		case NM_ListenServer: return TEXT("ListenServer");
		case NM_Client: return TEXT("Client");
		case NM_Standalone: return TEXT("Standalone");
		default: return TEXT("Other");
		}
	}

	bool Tick(FState& State)
	{
		UWorld* World = FindGameWorld();
		const double Now = FPlatformTime::Seconds();
		if (Now - State.StartTime > 60.0)
		{
			UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST timeout"));
			FPlatformMisc::RequestExit(false);
			return false;
		}
		if (!World)
		{
			return true;
		}

		const bool bServer = World->GetNetMode() == NM_ListenServer;
		UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();

		// The server gives every pawn a replicated Feel Replication component.
		if (bServer)
		{
			for (TActorIterator<APawn> It(World); It; ++It)
			{
				if (!It->FindComponentByClass<UFeelReplicationComponent>())
				{
					UFeelReplicationComponent* Component = NewObject<UFeelReplicationComponent>(*It, TEXT("FeelNetTestReplication"));
					Component->RegisterComponent();
					It->AddInstanceComponent(Component);
				}
			}
		}

		int32 NumPawns = 0;
		for (TActorIterator<APawn> It(World); It; ++It)
		{
			NumPawns += It->FindComponentByClass<UFeelReplicationComponent>() ? 1 : 0;
		}

		// The client starts 8 s after the server: server times are 8 s later so both act at the same moment.
		const double Elapsed = Now - State.StartTime - (bServer ? 8.0 : 0.0);
		if (!State.bTriggered && NumPawns >= 2 && Elapsed > 14.0 && bServer == State.bTriggerOnServer)
		{
			APlayerController* LocalController = World->GetFirstPlayerController();
			APawn* LocalPawn = LocalController ? LocalController->GetPawn() : nullptr;
			UFeelReplicationComponent* Component = LocalPawn ? LocalPawn->FindComponentByClass<UFeelReplicationComponent>() : nullptr;
			UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Hit.R_Hit"));
			if (Component && Recipe)
			{
				UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST %s triggers mode %d on %s"), NetModeName(World->GetNetMode()), State.Mode, *LocalPawn->GetName());
				Component->PlayFeelNetworked(Recipe, FFeelTarget::FromActor(LocalPawn), 1.0f, FFeelPlayContext(), static_cast<EFeelNetMode>(State.Mode), State.RelevancyDistance);
				State.bTriggered = true;
			}
		}
		if (State.TriggerTime < 0.0 && NumPawns >= 2 && Elapsed > 12.0)
		{
			State.TriggerTime = Now;
		}

		if (State.TriggerTime > 0.0 && Subsystem)
		{
			State.MaxInstances = FMath::Max(State.MaxInstances, Subsystem->GetNumActiveInstances());
			State.MaxFlash = FMath::Max(State.MaxFlash, Subsystem->GetScreenOutput().FlashAlpha);
			for (const FFeelInstance& Instance : Subsystem->GetInstances())
			{
				if (APawn* Pawn = Cast<APawn>(Instance.TargetActor.Get()))
				{
					State.MaxScale = FMath::Max(State.MaxScale, static_cast<float>(Pawn->GetRootComponent()->GetRelativeScale3D().X));
				}
			}
			// Scale of every pawn's root and mesh on this machine, whether or not this machine plays anything.
			for (TActorIterator<APawn> It(World); It; ++It)
			{
				const bool bLocal = It->IsLocallyControlled();
				const FString Key = bLocal ? TEXT("localPawn") : TEXT("remotePawn");
				float& Max = State.MaxPawnScale.FindOrAdd(Key + TEXT(".root"));
				Max = FMath::Max(Max, static_cast<float>(It->GetRootComponent()->GetRelativeScale3D().X));
				if (const ACharacter* Character = Cast<ACharacter>(*It))
				{
					float& MeshMax = State.MaxPawnScale.FindOrAdd(Key + TEXT(".mesh"));
					MeshMax = FMath::Max(MeshMax, static_cast<float>(Character->GetMesh()->GetRelativeScale3D().X));
					if (State.ReplicationInfo.IsEmpty())
					{
						State.ReplicationInfo = FString::Printf(TEXT("rootReplicates=%d meshReplicates=%d"), It->GetRootComponent()->GetIsReplicated() ? 1 : 0, Character->GetMesh()->GetIsReplicated() ? 1 : 0);
					}
				}
			}
			FFeelFrameOutput CameraOutput;
			if (World->GetFirstPlayerController() && Subsystem->GetCameraOutput(World->GetFirstPlayerController(), CameraOutput))
			{
				State.bCameraOutput |= !CameraOutput.CameraRotationOffset.IsNearlyZero() || !CameraOutput.CameraLocationOffset.IsNearlyZero();
			}

			if (Now - State.TriggerTime > 8.0)
			{
				FString PawnScales;
				for (const TPair<FString, float>& Pair : State.MaxPawnScale)
				{
					PawnScales += FString::Printf(TEXT(" %s=%.2f"), *Pair.Key, Pair.Value);
				}
				UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST RESULT %s mode=%d triggerOnServer=%d relevancy=%.0f instances=%d flash=%.2f camera=%d%s %s"),
					NetModeName(World->GetNetMode()), State.Mode, State.bTriggerOnServer ? 1 : 0, State.RelevancyDistance, State.MaxInstances, State.MaxFlash, State.bCameraOutput ? 1 : 0, *PawnScales, *State.ReplicationInfo);
				FPlatformMisc::RequestExit(false);
				return false;
			}
		}
		return true;
	}

	/** Rendering check for Spawn Decal: plays R_Decal on the local pawn next to two decals spawned directly, then screenshots. */
	static FAutoConsoleCommand DecalCommand(
		TEXT("feeltest.decal"),
		TEXT("FeelKit decal render check"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			const double Start = FPlatformTime::Seconds();
			TSharedRef<int32> Stage = MakeShared<int32>(0);
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Start, Stage](float)
			{
				const double Elapsed = FPlatformTime::Seconds() - Start;
				UWorld* World = FindGameWorld();
				APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
				APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
				if (*Stage == 0 && Pawn && Elapsed > 10.0)
				{
					UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/FeelKitTests/R_Decal.R_Decal"));
					UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FeelKitTests/M_DecalTest.M_DecalTest"));
					if (Recipe)
					{
						for (const FFeelTrack& Track : Recipe->Tracks)
						{
							if (const UFeelStep_SpawnDecal* Step = Cast<UFeelStep_SpawnDecal>(Track.Step))
							{
								UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST decal step material=%s size=%s findSurface=%d distance=%.0f lifetime=%.1f start=%.2f dur=%.2f enabled=%d channel=%s"),
									*GetNameSafe(Step->DecalMaterial), *Step->DecalSize.ToString(), Step->bFindSurface ? 1 : 0, Step->SurfaceSearchDistance, Step->Lifetime, Track.StartTime, Track.Duration, Track.bEnabled ? 1 : 0, *Track.Channel.ToString());
							}
						}
						// Same recipe with the engine's decal material, which is known to render, to check placement end to end.
						UFeelRecipe* Proof = DuplicateObject<UFeelRecipe>(Recipe, GetTransientPackage());
						for (FFeelTrack& Track : Proof->Tracks)
						{
							if (UFeelStep_SpawnDecal* Step = Cast<UFeelStep_SpawnDecal>(Track.Step))
							{
								Step->DecalMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultDeferredDecalMaterial.DefaultDeferredDecalMaterial"));
								Step->Lifetime = 60.0f;
							}
						}
						Proof->AddToRoot();
						const FFeelHandle Handle = World->GetSubsystem<UFeelSubsystem>()->PlayFeel(Proof, FFeelTarget::FromActor(Pawn));
						UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST played R_Decal handle valid %d at pawn %s"), Handle.IsValid() ? 1 : 0, *Pawn->GetActorLocation().ToString());
					}
					const FVector Right = Pawn->GetActorRightVector();
					const FVector Feet = Pawn->GetActorLocation() - FVector(0.0, 0.0, 96.0);
					if (Material)
					{
						UGameplayStatics::SpawnDecalAtLocation(World, Material, FVector(50.0, 60.0, 60.0), Feet + Right * 250.0, FRotator(-90.0, 0.0, 0.0), 80.0f);
					}
					UGameplayStatics::SpawnDecalAtLocation(World, LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultDeferredDecalMaterial.DefaultDeferredDecalMaterial")), FVector(50.0, 60.0, 60.0), Feet - Right * 250.0, FRotator(-90.0, 0.0, 0.0), 80.0f);
					*Stage = 1;
				}
				if (*Stage == 1 && Elapsed > 45.0)
				{
					for (TObjectIterator<UDecalComponent> It; It; ++It)
					{
						if (It->GetWorld() == World)
						{
							UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST decal %s material=%s location=%s rotation=%s size=%s visible=%d registered=%d owner=%s fadeScreen=%.3f"),
								*It->GetName(), *GetNameSafe(It->GetDecalMaterial()), *It->GetComponentLocation().ToString(), *It->GetComponentRotation().ToString(), *It->DecalSize.ToString(), It->IsVisible() ? 1 : 0, It->IsRegistered() ? 1 : 0, *GetNameSafe(It->GetOwner()), It->FadeScreenSize);
						}
					}
					if (UMaterialInterface* DecalMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/FeelKitTests/M_DecalTest.M_DecalTest")))
					{
						const UMaterial* Base = DecalMaterial->GetMaterial();
						FMaterialResource* Resource = DecalMaterial->GetMaterialResource(World->GetFeatureLevel());
						FString Errors;
						if (Resource)
						{
							for (const FString& Error : Resource->GetCompileErrors())
							{
								Errors += Error + TEXT(" | ");
							}
						}
						UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST material domain=%d blend=%d resource=%d compiling=%d errors=%s"),
							static_cast<int32>(Base->MaterialDomain), static_cast<int32>(Base->BlendMode), 
							Resource ? 1 : 0, Resource && Resource->IsCompilationFinished() ? 0 : 1, *Errors);
#if WITH_EDITORONLY_DATA
						if (const UMaterialEditorOnlyData* Data = Base->GetEditorOnlyData())
						{
							UE_LOG(LogFeelNetTest, Display, TEXT("FEELTEST material inputs baseColor=%d emissive=%d opacity=%d normal=%d roughness=%d expressions=%d"),
								Data->BaseColor.IsConnected() ? 1 : 0, Data->EmissiveColor.IsConnected() ? 1 : 0, Data->Opacity.IsConnected() ? 1 : 0, Data->Normal.IsConnected() ? 1 : 0, Data->Roughness.IsConnected() ? 1 : 0, Data->ExpressionCollection.Expressions.Num());
						}
#endif
					}
					FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("FeelKitDecalCheck.png"), false, false);
					*Stage = 2;
				}
				if (*Stage == 2 && Elapsed > 50.0)
				{
					FPlatformMisc::RequestExit(false);
					return false;
				}
				return Elapsed < 90.0;
			}), 0.02f);
		}));

	static FAutoConsoleCommand Command(
		TEXT("feeltest.net"),
		TEXT("FeelKit network harness: feeltest.net <Mode> <server|client>"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			TSharedRef<FState> State = MakeShared<FState>();
			State->Mode = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 0;
			State->bTriggerOnServer = Args.Num() > 1 && Args[1] == TEXT("server");
			State->RelevancyDistance = Args.Num() > 2 ? FCString::Atof(*Args[2]) : 0.0f;
			State->StartTime = FPlatformTime::Seconds();
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([State](float)
			{
				return Tick(*State);
			}), 0.02f);
		}));
}

#endif // WITH_EDITOR
