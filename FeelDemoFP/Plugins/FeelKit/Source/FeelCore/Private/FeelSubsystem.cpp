// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "FeelCameraModifier.h"
#include "FeelComfortSubsystem.h"
#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "Engine/GameViewportClient.h"
// FEELKIT_PRO_BEGIN
#include "FeelMap.h"
// FEELKIT_PRO_END
#include "FeelNumberPopLayer.h"
#include "FeelPlayCapture.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
#include "FeelRecipe.h"
// FEELKIT_PRO_BEGIN
#include "FeelTriggerComponent.h"
// FEELKIT_PRO_END
#include "FeelSceneViewExtension.h"
#include "FeelLog.h"
#include "FeelSettings.h"
#include "FeelStep.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "SceneViewExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelSubsystem)


// FEELKIT_PRO_BEGIN
DECLARE_STATS_GROUP(TEXT("Feel"), STATGROUP_Feel, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Feel Tick"), STAT_FeelTick, STATGROUP_Feel);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Feel Instances"), STAT_FeelInstances, STATGROUP_Feel);
// FEELKIT_PRO_END

static TAutoConsoleVariable<bool> CVarFeelEnabled(
	TEXT("feel.Enabled"),
	true,
	TEXT("Enables FeelKit playback. 0 stops all playing recipes and blocks new ones."),
	ECVF_Default);

// FEELKIT_PRO_BEGIN
static TAutoConsoleVariable<float> CVarFeelGlobalScale(
	TEXT("feel.GlobalScale"),
	1.0f,
	TEXT("Multiplies the intensity of every FeelKit recipe. 1 is normal, 0 silences everything without stopping it."),
	ECVF_Default);
// FEELKIT_PRO_END

namespace FeelSubsystemPrivate
{
	constexpr int32 NumMotors = 4;

	/** Condition "Local Player Only": the target is a local player's camera, pawn or controller, or an actor owned by one of those. */
	bool IsLocalPlayerTarget(const FFeelTarget& Target, const UWorld* World)
	{
		if (Target.Type == EFeelTargetType::LocalPlayerCamera)
		{
			return Target.ResolvePlayerController(World) != nullptr;
		}

		constexpr int32 MaxOwnerDepth = 8;
		const AActor* Actor = Target.GetActor();
		for (int32 Depth = 0; Actor && Depth < MaxOwnerDepth; ++Depth)
		{
			if (const APawn* Pawn = Cast<APawn>(Actor))
			{
				const APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
				return PlayerController && PlayerController->IsLocalController();
			}
			if (const APlayerController* PlayerController = Cast<APlayerController>(Actor))
			{
				return PlayerController->IsLocalController();
			}
			Actor = Actor->GetOwner();
		}
		return false;
	}

	/** World location of a target: its location, component, actor, or the camera of its local player. */
	FVector GetTargetWorldLocation(const FFeelTarget& Target, const UWorld* World)
	{
		if (Target.Type == EFeelTargetType::LocalPlayerCamera)
		{
			const APlayerController* PlayerController = Target.ResolvePlayerController(World);
			return PlayerController && PlayerController->PlayerCameraManager ? PlayerController->PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector;
		}
		if (const USceneComponent* Component = Target.GetSceneComponent())
		{
			return Component->GetComponentLocation();
		}
		if (const AActor* Actor = Target.GetActor())
		{
			return Actor->GetActorLocation();
		}
		return Target.Location;
	}

	/** Play directions for steps: world direction, and directions in the target player's view space at play start. */
	void ComputeDirections(const FFeelTarget& Target, const FFeelPlayContext& Context, const UWorld* World, FFeelInstance& Instance)
	{
		const APlayerController* PlayerController = World ? Target.ResolvePlayerController(World) : nullptr;
		const FRotator ViewRotation = PlayerController && PlayerController->PlayerCameraManager ? PlayerController->PlayerCameraManager->GetCameraRotation() : FRotator::ZeroRotator;

		Instance.WorldDirection = Context.Direction.GetSafeNormal();
		Instance.ViewDirection = ViewRotation.UnrotateVector(Instance.WorldDirection);

		if (!Context.Location.IsZero())
		{
			const FVector FromLocation = (GetTargetWorldLocation(Target, World) - Context.Location).GetSafeNormal();
			Instance.ViewDirectionFromLocation = ViewRotation.UnrotateVector(FromLocation);
		}
	}

	/** Condition "Max Distance": distance from the target to the nearest local player camera, or -1 when there is none. */
	float MeasureTargetDistance(const FFeelTarget& Target, const UWorld* World)
	{
		if (!World || Target.Type == EFeelTargetType::LocalPlayerCamera || Target.Type == EFeelTargetType::None)
		{
			// Effects on a player's own camera are always at the listener.
			return World ? 0.0f : -1.0f;
		}

		FVector TargetLocation = Target.Location;
		if (const USceneComponent* Component = Target.GetSceneComponent())
		{
			TargetLocation = Component->GetComponentLocation();
		}
		else if (const AActor* Actor = Target.GetActor())
		{
			TargetLocation = Actor->GetActorLocation();
		}

		float Nearest = -1.0f;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			const APlayerController* PlayerController = It->Get();
			if (PlayerController && PlayerController->IsLocalController() && PlayerController->PlayerCameraManager)
			{
				const float Distance = static_cast<float>(FVector::Dist(PlayerController->PlayerCameraManager->GetCameraLocation(), TargetLocation));
				Nearest = Nearest < 0.0f ? Distance : FMath::Min(Nearest, Distance);
			}
		}
		return Nearest;
	}

	float ReadDilation(UObject* Target)
	{
		if (const AWorldSettings* WorldSettings = Cast<AWorldSettings>(Target))
		{
			return WorldSettings->TimeDilation;
		}
		if (const AActor* Actor = Cast<AActor>(Target))
		{
			return Actor->CustomTimeDilation;
		}
		return 1.0f;
	}

	void WriteDilation(UObject* Target, float Dilation)
	{
		if (AWorldSettings* WorldSettings = Cast<AWorldSettings>(Target))
		{
			WorldSettings->SetTimeDilation(Dilation);
		}
		else if (AActor* Actor = Cast<AActor>(Target))
		{
			Actor->CustomTimeDilation = Dilation;
		}
	}

	/** Starts, updates or stops one motor's dynamic force feedback. Motor order: left large, left small, right large, right small. */
	void UpdateMotor(APlayerController& PlayerController, uint64& Handle, int32 Motor, float Value)
	{
		const bool bLeftLarge = Motor == 0;
		const bool bLeftSmall = Motor == 1;
		const bool bRightLarge = Motor == 2;
		const bool bRightSmall = Motor == 3;

		if (Value <= 0.0f)
		{
			if (Handle != 0)
			{
				PlayerController.PlayDynamicForceFeedback(0.0f, -1.0f, bLeftLarge, bLeftSmall, bRightLarge, bRightSmall, EDynamicForceFeedbackAction::Stop, Handle);
				Handle = 0;
			}
			return;
		}

		// A negative duration keeps the effect running until it is stopped; the value is updated every frame.
		if (Handle == 0)
		{
			Handle = PlayerController.PlayDynamicForceFeedback(Value, -1.0f, bLeftLarge, bLeftSmall, bRightLarge, bRightSmall, EDynamicForceFeedbackAction::Start);
		}
		else
		{
			PlayerController.PlayDynamicForceFeedback(Value, -1.0f, bLeftLarge, bLeftSmall, bRightLarge, bRightSmall, EDynamicForceFeedbackAction::Update, Handle);
		}
	}
}

FFeelTarget FFeelInstance::MakeTarget() const
{
	FFeelTarget Target;
	Target.Type = TargetType;
	Target.Actor = TargetActor.Get();
	Target.Component = TargetComponent.Get();
	Target.Widget = TargetWidget.Get();
	Target.Location = TargetLocation;
	Target.LocalPlayerIndex = LocalPlayerIndex;
	return Target;
}

UFeelSubsystem* UFeelSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	return World ? World->GetSubsystem<UFeelSubsystem>() : nullptr;
}

FFeelHandle UFeelSubsystem::PlayFeel(UFeelRecipe* Recipe, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context)
{
	if (!Recipe || !CVarFeelEnabled.GetValueOnGameThread())
	{
		return FFeelHandle();
	}

	// Recipes are cosmetic; a dedicated server has no one to show them to.
	const UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_DedicatedServer)
	{
		return FFeelHandle();
	}

	if (Target.UsesObject() && !Target.GetActor() && !Target.GetSceneComponent() && !Target.Widget)
	{
		UE_LOG(LogFeel, Warning, TEXT("PlayFeel: %s was given an actor or component target that is not valid."), *GetNameSafe(Recipe));
		return FFeelHandle();
	}

	const FObjectKey TargetKey = MakeTargetKey(Target);
	int32 ActiveOnTarget = 0;
	for (const TArray<FFeelInstance>* List : { &Instances, &PendingInstances })
	{
		for (const FFeelInstance& Existing : *List)
		{
			if (!Existing.bFinished && Existing.Recipe == Recipe && Existing.TargetKey == TargetKey)
			{
				++ActiveOnTarget;
			}
		}
	}

	const double Now = FPlatformTime::Seconds();
	if (!PlaybackGate.CanPlay(*Recipe, TargetKey, Now, ActiveOnTarget))
	{
		return FFeelHandle();
	}
	PlaybackGate.NotifyPlayed(*Recipe, TargetKey, Now);

	// While instances advance, references into Instances are live, so new plays wait in a separate list.
	FFeelInstance& Instance = (bAdvancingInstances ? PendingInstances : Instances).AddDefaulted_GetRef();
	Instance.Id = NextInstanceId++;
	Instance.Recipe = Recipe;
	Instance.TargetType = Target.Type;
	Instance.TargetActor = Target.GetActor();
	Instance.TargetComponent = Target.GetSceneComponent();
	Instance.TargetWidget = Target.Widget;
	Instance.TargetLocation = Target.Location;
	Instance.LocalPlayerIndex = Target.LocalPlayerIndex;
	Instance.TargetKey = TargetKey;
	Instance.Intensity = FMath::Max(Intensity, 0.0f);
	Instance.Seed = FMath::Rand();
	Instance.Duration = Recipe->GetDuration();
	Instance.Lifecycle.Reset(Recipe->Tracks.Num());

	// Track conditions are decided once per play, so a track never pops in or out while the target moves.
	Instance.TargetDistance = FeelSubsystemPrivate::MeasureTargetDistance(Target, GetWorld());
	Instance.bTargetIsLocalPlayer = FeelSubsystemPrivate::IsLocalPlayerTarget(Target, GetWorld());

	Instance.PlayContext = Context;
	Instance.PlayContext.Instigator = nullptr;
	Instance.Instigator = Context.Instigator;
	Instance.bHadInstigator = Context.Instigator != nullptr;
	Instance.ParameterValues = Context.Parameters;
	for (const TPair<FName, float>& Pair : Context.Parameters)
	{
		Instance.ExplicitParameters.Add(Pair.Key);
	}
	FeelSubsystemPrivate::ComputeDirections(Target, Context, GetWorld(), Instance);
	RefreshAccumulatorParameters(Instance);

	// The built-in Distance parameter, when the recipe declares it and the game did not set it.
	static const FName DistanceParameterName(TEXT("Distance"));
	if (Recipe->FindParameter(DistanceParameterName) && !Instance.ParameterValues.Contains(DistanceParameterName) && Instance.TargetDistance >= 0.0f)
	{
		Instance.ParameterValues.Add(DistanceParameterName, Instance.TargetDistance);
	}

	// Length for this play, random durations included.
	Instance.Duration = FFeelEvaluator::GetRecipeDuration(*Recipe, MakeEvalParams(Instance, 1.0f));

	EnsureCameraModifiers();

	const FFeelHandle Handle(Instance.Id);
	// FEELKIT_PRO_BEGIN
	OnFeelStarted.Broadcast(Handle, Recipe);
	// FEELKIT_PRO_END
	return Handle;
}

// FEELKIT_PRO_BEGIN
FFeelHandle UFeelSubsystem::SendFeelEvent(const FGameplayTag& Event, const FFeelTarget& Target, float Intensity, const FFeelPlayContext& Context)
{
	TArray<const UFeelMap*> Maps;
	CollectFeelMaps(Target, Maps);

	const FFeelMapEntry* Entry = UFeelMap::FindBestEntry(Maps, Event, Context.ContextTags);
	if (!Entry || !Entry->Recipe)
	{
		// Say once per event why nothing plays: a missing Feel Map entry otherwise looks exactly like a broken recipe.
		if (!WarnedUnmatchedEvents.Contains(Event))
		{
			WarnedUnmatchedEvents.Add(Event);
			if (!Entry)
			{
				UE_LOG(LogFeel, Warning, TEXT("Send Feel Event %s: no Feel Map entry matches, so nothing plays. Feel Maps searched: %d (Project Settings > Plugins > FeelKit > Feel Maps, plus Feel Trigger components on the target). Add an entry for this event to one of them."),
					*Event.ToString(), Maps.Num());
			}
			else
			{
				UE_LOG(LogFeel, Warning, TEXT("Send Feel Event %s: the matching Feel Map entry has no recipe, so nothing plays."), *Event.ToString());
			}
		}
		return FFeelHandle();
	}
	return PlayFeel(Entry->Recipe, Target, Intensity * Entry->IntensityScale, Context);
}
// FEELKIT_PRO_END

// FEELKIT_PRO_BEGIN
void UFeelSubsystem::CollectFeelMaps(const FFeelTarget& Target, TArray<const UFeelMap*>& OutMaps)
{
	// Maps on the target actor come first, so actors can override the project's answers.
	if (const AActor* Actor = Target.GetActor())
	{
		if (const UFeelTriggerComponent* Trigger = Actor->FindComponentByClass<UFeelTriggerComponent>())
		{
			for (const UFeelMap* Map : Trigger->FeelMaps)
			{
				OutMaps.Add(Map);
			}
		}
	}

	if (!bProjectFeelMapsLoaded)
	{
		bProjectFeelMapsLoaded = true;
		for (const TSoftObjectPtr<UFeelMap>& SoftMap : GetDefault<UFeelSettings>()->FeelMaps)
		{
			if (UFeelMap* Map = SoftMap.LoadSynchronous())
			{
				ProjectFeelMaps.Add(Map);
			}
		}
	}
	for (const UFeelMap* Map : ProjectFeelMaps)
	{
		OutMaps.Add(Map);
	}
}
// FEELKIT_PRO_END

void UFeelSubsystem::AddNumberPop(const FFeelNumberPop& Pop)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	FFeelNumberPop& Added = NumberPops.Add_GetRef(Pop);
	Added.StartTime = NumberPopTime;

	// One layer per world, created on the first pop.
	if (!NumberPopLayer.IsValid())
	{
		if (UGameViewportClient* Viewport = World->GetGameViewport())
		{
			NumberPopLayer = SNew(SFeelNumberPopLayer, this);
			Viewport->AddViewportWidgetContent(NumberPopLayer.ToSharedRef(), 50);
		}
	}
}

// FEELKIT_PRO_BEGIN
void UFeelSubsystem::ReleaseFeel(FFeelHandle Handle)
{
	if (FFeelInstance* Instance = FindInstance(Handle))
	{
		Instance->Clock.Release();
	}
}
// FEELKIT_PRO_END

// FEELKIT_PRO_BEGIN
bool UFeelSubsystem::SetFeelParameter(FFeelHandle Handle, FName ParameterName, float Value)
{
	FFeelInstance* Instance = FindInstance(Handle);
	if (!Instance || ParameterName.IsNone())
	{
		return false;
	}
	Instance->ParameterValues.Add(ParameterName, Value);
	Instance->ExplicitParameters.Add(ParameterName);
	return true;
}
// FEELKIT_PRO_END

FFeelInstance* UFeelSubsystem::FindInstance(FFeelHandle Handle)
{
	if (!Handle.IsValid())
	{
		return nullptr;
	}
	auto Matches = [Handle](const FFeelInstance& Instance) { return Instance.Id == Handle.GetId() && !Instance.bFinished; };
	FFeelInstance* Found = Instances.FindByPredicate(Matches);
	return Found ? Found : PendingInstances.FindByPredicate(Matches);
}

// FEELKIT_PRO_BEGIN
void UFeelSubsystem::AddToAccumulator(FName AccumulatorName, float Amount, const AActor* Actor)
{
	const FFeelAccumulatorDefinition* Definition = GetDefault<UFeelSettings>()->FindAccumulator(AccumulatorName);
	if (!Definition)
	{
		UE_LOG(LogFeel, Warning, TEXT("Accumulator %s is not defined in Project Settings > Plugins > FeelKit > Accumulators."), *AccumulatorName.ToString());
		return;
	}

	FAccumulatorValue* Value = FindAccumulatorValue(AccumulatorName, Actor);
	if (!Value)
	{
		Value = &AccumulatorValues.AddDefaulted_GetRef();
		Value->Name = AccumulatorName;
		Value->Actor = Actor;
		Value->bGlobal = Actor == nullptr;
	}
	Value->Value = FMath::Clamp(Value->Value + Amount, 0.0f, Definition->MaxValue);
	Value->LastAddTime = AccumulatorTime;
}
// FEELKIT_PRO_END

// FEELKIT_PRO_BEGIN
void UFeelSubsystem::SetAccumulator(FName AccumulatorName, float NewValue, const AActor* Actor)
{
	const FFeelAccumulatorDefinition* Definition = GetDefault<UFeelSettings>()->FindAccumulator(AccumulatorName);
	if (!Definition)
	{
		UE_LOG(LogFeel, Warning, TEXT("Accumulator %s is not defined in Project Settings > Plugins > FeelKit > Accumulators."), *AccumulatorName.ToString());
		return;
	}

	FAccumulatorValue* Value = FindAccumulatorValue(AccumulatorName, Actor);
	if (!Value)
	{
		Value = &AccumulatorValues.AddDefaulted_GetRef();
		Value->Name = AccumulatorName;
		Value->Actor = Actor;
		Value->bGlobal = Actor == nullptr;
	}
	Value->Value = FMath::Clamp(NewValue, 0.0f, Definition->MaxValue);
	Value->LastAddTime = AccumulatorTime;
}
// FEELKIT_PRO_END

void UFeelSubsystem::ForEachAccumulator(TFunctionRef<void(FName, const AActor*, float)> Visit) const
{
	for (const FAccumulatorValue& Value : AccumulatorValues)
	{
		Visit(Value.Name, Value.bGlobal ? nullptr : Value.Actor.Get(), Value.Value);
	}
}

// FEELKIT_PRO_BEGIN
void UFeelSubsystem::HandleShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	static const FName FeelCategory(TEXT("Feel"));
	if (!HUD || !Canvas || HUD->GetWorld() != GetWorld() || !DisplayInfo.IsDisplayOn(FeelCategory))
	{
		return;
	}

	FDisplayDebugManager& Debug = Canvas->DisplayDebugManager;
	Debug.SetFont(GEngine->GetSmallFont());
	Debug.SetDrawColor(FColor(255, 160, 40));
	Debug.DrawString(FString::Printf(TEXT("FEELKIT: %d playing, %d accumulator values, %d number pops"), GetNumActiveInstances(), AccumulatorValues.Num(), NumberPops.Num()));

	Debug.SetDrawColor(FColor::White);
	for (const FFeelInstance& Instance : Instances)
	{
		if (Instance.bFinished || !Instance.Recipe)
		{
			continue;
		}
		const FFeelTarget Target = Instance.MakeTarget();
		FString Parameters;
		for (const TPair<FName, float>& Pair : Instance.ParameterValues)
		{
			Parameters += FString::Printf(TEXT(" %s=%.2f"), *Pair.Key.ToString(), Pair.Value);
		}
		const bool bLooping = FFeelPlaybackClock::HasSustain(*Instance.Recipe) && !Instance.Clock.bReleased;
		Debug.DrawString(FString::Printf(TEXT("  %s on %s  %.2f / %.2f s%s  x%.2f%s"),
			*Instance.Recipe->GetName(),
			Target.GetActor() ? *Target.GetActor()->GetActorNameOrLabel() : TEXT("-"),
			Instance.Clock.RecipeTime, Instance.Duration,
			bLooping ? TEXT(" (sustaining)") : TEXT(""),
			Instance.Intensity, *Parameters));
	}

	for (const FAccumulatorValue& Value : AccumulatorValues)
	{
		Debug.DrawString(FString::Printf(TEXT("  Accumulator %s (%s) = %.2f"), *Value.Name.ToString(),
			Value.bGlobal ? TEXT("global") : (Value.Actor.IsValid() ? *Value.Actor->GetActorNameOrLabel() : TEXT("gone")), Value.Value));
	}

	const APlayerController* PlayerController = HUD->GetOwningPlayerController();
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (const UFeelComfortSubsystem* Comfort = LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr)
	{
		const FFeelComfortScales& Scales = Comfort->GetComfortScalesRef();
		Debug.SetDrawColor(FColor(120, 200, 255));
		Debug.DrawString(FString::Printf(TEXT("  Comfort: master %.2f shake %.2f motion %.2f flashes %.2f hitstop %.2f distortion %.2f haptics %.2f  limiter %s %.1f/s  roll %s"),
			Scales.Master, Scales.CameraShake, Scales.CameraMotion, Scales.Flashes, Scales.HitstopAndSlowMo, Scales.ScreenDistortion, Scales.Haptics,
			Scales.bLimitFlashes ? TEXT("on") : TEXT("off"), Scales.MaxFlashesPerSecond, Scales.bAllowCameraRoll ? TEXT("on") : TEXT("off")));

		const float ComfortScale = Comfort->GetEffectiveForceFeedbackScale();
		const float ControllerScale = PlayerController->ForceFeedbackScale;
		if (ControllerScale <= UE_KINDA_SMALL_NUMBER)
		{
			Debug.SetDrawColor(FColor(255, 110, 90));
		}
		Debug.DrawString(FString::Printf(TEXT("  Controller vibration: comfort scale %.2f, controller scale %.2f%s"),
			ComfortScale, ControllerScale, ControllerScale <= UE_KINDA_SMALL_NUMBER ? TEXT("  NOTHING REACHES THE CONTROLLER") : TEXT("")));
	}
}
// FEELKIT_PRO_END

// FEELKIT_PRO_BEGIN
float UFeelSubsystem::GetAccumulator(FName AccumulatorName, const AActor* Actor) const
{
	const FAccumulatorValue* Value = FindAccumulatorValue(AccumulatorName, Actor);
	return Value ? Value->Value : 0.0f;
}
// FEELKIT_PRO_END

UFeelSubsystem::FAccumulatorValue* UFeelSubsystem::FindAccumulatorValue(FName AccumulatorName, const AActor* Actor)
{
	return AccumulatorValues.FindByPredicate([AccumulatorName, Actor](const FAccumulatorValue& Value)
	{
		return Value.Name == AccumulatorName && (Actor ? (!Value.bGlobal && Value.Actor.Get() == Actor) : Value.bGlobal);
	});
}

const UFeelSubsystem::FAccumulatorValue* UFeelSubsystem::FindAccumulatorValue(FName AccumulatorName, const AActor* Actor) const
{
	return AccumulatorValues.FindByPredicate([AccumulatorName, Actor](const FAccumulatorValue& Value)
	{
		return Value.Name == AccumulatorName && (Actor ? (!Value.bGlobal && Value.Actor.Get() == Actor) : Value.bGlobal);
	});
}

void UFeelSubsystem::UpdateAccumulators(float RealDeltaSeconds)
{
	AccumulatorTime += RealDeltaSeconds;
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();

	for (FAccumulatorValue& Value : AccumulatorValues)
	{
		const FFeelAccumulatorDefinition* Definition = Settings->FindAccumulator(Value.Name);
		if (Definition && AccumulatorTime - Value.LastAddTime >= Definition->DecayDelay)
		{
			Value.Value = FMath::Max(Value.Value - Definition->DecayPerSecond * RealDeltaSeconds, 0.0f);
		}
	}

	AccumulatorValues.RemoveAll([Settings](const FAccumulatorValue& Value)
	{
		const FFeelAccumulatorDefinition* Definition = Settings->FindAccumulator(Value.Name);
		const bool bDecayedAway = Value.Value <= 0.0f && (!Definition || Definition->DecayPerSecond > 0.0f);
		const bool bActorGone = !Value.bGlobal && !Value.Actor.IsValid();
		return bDecayedAway || bActorGone;
	});
}

void UFeelSubsystem::RefreshAccumulatorParameters(FFeelInstance& Instance) const
{
	if (!Instance.Recipe)
	{
		return;
	}

	const AActor* TargetActor = Instance.MakeTarget().GetActor();
	for (const FFeelRecipeParameter& Parameter : Instance.Recipe->Parameters)
	{
		if (Parameter.Accumulator.IsNone() || Instance.ExplicitParameters.Contains(Parameter.Name))
		{
			continue;
		}

		// The target's own value first, then the instigator's (a reaction played on whoever was hit reads the value of whoever
		// hit it), then the global one.
		const FAccumulatorValue* Value = TargetActor ? FindAccumulatorValue(Parameter.Accumulator, TargetActor) : nullptr;
		const AActor* InstigatorActor = Instance.Instigator.Get();
		if (!Value && InstigatorActor && InstigatorActor != TargetActor)
		{
			Value = FindAccumulatorValue(Parameter.Accumulator, InstigatorActor);
		}
		if (!Value)
		{
			Value = FindAccumulatorValue(Parameter.Accumulator, nullptr);
		}
		Instance.ParameterValues.Add(Parameter.Name, Value ? Value->Value : 0.0f);
	}
}

void UFeelSubsystem::StopFeel(FFeelHandle Handle, bool bBlendOut)
{
	if (FFeelInstance* Instance = FindInstance(Handle))
	{
		if (bBlendOut && GetDefault<UFeelSettings>()->BlendOutTime > 0.0f)
		{
			Instance->bStopping = true;
		}
		else
		{
			FinishInstance(*Instance, true);
		}
	}
}

void UFeelSubsystem::StopAllFeel(const AActor* Target)
{
	// Finish callbacks may start new plays; keep them out of the arrays being iterated.
	const bool bWasAdvancing = bAdvancingInstances;
	TArray<FFeelInstance> StartedDuringStop;
	if (!bWasAdvancing)
	{
		bAdvancingInstances = true;
		Swap(StartedDuringStop, PendingInstances);
	}

	for (TArray<FFeelInstance>* List : { &Instances, &StartedDuringStop })
	{
		for (FFeelInstance& Instance : *List)
		{
			if (!Instance.bFinished && (!Target || Instance.TargetActor.Get() == Target))
			{
				FinishInstance(Instance, true);
			}
		}
	}

	if (!bWasAdvancing)
	{
		bAdvancingInstances = false;
		StartedDuringStop.Append(MoveTemp(PendingInstances));
		PendingInstances = MoveTemp(StartedDuringStop);
	}
}

bool UFeelSubsystem::IsPlaying(FFeelHandle Handle) const
{
	return const_cast<UFeelSubsystem*>(this)->FindInstance(Handle) != nullptr;
}

int32 UFeelSubsystem::GetNumActiveInstances() const
{
	int32 Count = 0;
	for (const TArray<FFeelInstance>* List : { &Instances, &PendingInstances })
	{
		for (const FFeelInstance& Instance : *List)
		{
			Count += Instance.bFinished ? 0 : 1;
		}
	}
	return Count;
}

bool UFeelSubsystem::GetCameraOutput(const APlayerController* PlayerController, FFeelFrameOutput& OutOutput) const
{
	if (const FFeelFrameOutput* Found = CameraOutputs.Find(FObjectKey(PlayerController)))
	{
		OutOutput = *Found;
		return true;
	}
	return false;
}

void UFeelSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ViewExtension = FSceneViewExtensions::NewExtension<FFeelSceneViewExtension>(GetWorld(), this);
	// FEELKIT_PRO_BEGIN
	ShowDebugHandle = AHUD::OnShowDebugInfo.AddUObject(this, &UFeelSubsystem::HandleShowDebugInfo);
	// FEELKIT_PRO_END
}

void UFeelSubsystem::Deinitialize()
{
	// FEELKIT_PRO_BEGIN
	AHUD::OnShowDebugInfo.Remove(ShowDebugHandle);
	// FEELKIT_PRO_END

	// World teardown: no step callbacks, but every clock, component and controller goes back.
	bAdvancingInstances = true;
	for (TArray<FFeelInstance>* List : { &Instances, &PendingInstances })
	{
		for (FFeelInstance& Instance : *List)
		{
			FinishInstance(Instance, true, false);
		}
	}
	bAdvancingInstances = false;
	Instances.Reset();
	PendingInstances.Reset();
	RestoreEverything();

	NumberPops.Reset();
	if (NumberPopLayer.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			if (UGameViewportClient* Viewport = World->GetGameViewport())
			{
				Viewport->RemoveViewportWidgetContent(NumberPopLayer.ToSharedRef());
			}
		}
		NumberPopLayer.Reset();
	}
	ViewExtension.Reset();

	Super::Deinitialize();
}

void UFeelSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// FEELKIT_PRO_BEGIN
	SCOPE_CYCLE_COUNTER(STAT_FeelTick);
	// FEELKIT_PRO_END

	// Unscaled frame time, so hitstops and slow motion never slow the recipes themselves.
	const float RealDeltaSeconds = static_cast<float>(FApp::GetDeltaTime());
	const bool bEnabled = CVarFeelEnabled.GetValueOnGameThread();

	UpdateAccumulators(RealDeltaSeconds);

	NumberPopTime += RealDeltaSeconds;
	NumberPops.RemoveAll([this](const FFeelNumberPop& Pop) { return NumberPopTime - Pop.StartTime > Pop.Lifetime; });

	bAdvancingInstances = true;
	for (FFeelInstance& Instance : Instances)
	{
		AdvanceInstance(Instance, RealDeltaSeconds, bEnabled);
	}
	bAdvancingInstances = false;

	// Plays started from step callbacks during the update join now and advance from the next frame.
	Instances.Append(MoveTemp(PendingInstances));
	PendingInstances.Reset();
	Instances.RemoveAll([](const FFeelInstance& Instance) { return Instance.bFinished; });
	// FEELKIT_PRO_BEGIN
	SET_DWORD_STAT(STAT_FeelInstances, Instances.Num());
	// FEELKIT_PRO_END

	ResolveAndDeliver();
}

bool UFeelSubsystem::IsTickable() const
{
	return Instances.Num() > 0
		|| PendingInstances.Num() > 0
		|| TimeArbiter.IsOwningAny()
		|| ActorDelivery.IsModifyingAny()
		|| ForceFeedbackPlayers.Num() > 0
		|| CameraOutputs.Num() > 0
		|| ScreenOutput.FlashAlpha > 0.0f
		|| ScreenOutput.FadeAlpha > 0.0f
		|| ScreenOutput.TintWeight > 0.0f
		|| ScreenOutput.PostProcessMaterials.Num() > 0
		|| NumberPops.Num() > 0
		|| WidgetDelivery.IsModifyingAny()
		|| AudioDelivery.IsActive()
		|| AccumulatorValues.Num() > 0;
}

TStatId UFeelSubsystem::GetStatId() const
{
	// FEELKIT_PRO_BEGIN
	RETURN_QUICK_DECLARE_CYCLE_STAT(UFeelSubsystem, STATGROUP_Feel);
	// FEELKIT_PRO_END
	// FEELKIT_LITE: RETURN_QUICK_DECLARE_CYCLE_STAT(UFeelSubsystem, STATGROUP_Tickables);
}

bool UFeelSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Editor preview worlds (such as the animation editor) let anim notifies play recipes while scrubbing animations.
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE || WorldType == EWorldType::EditorPreview;
}

void UFeelSubsystem::AdvanceInstance(FFeelInstance& Instance, float RealDeltaSeconds, bool bEnabled)
{
	if (Instance.bFinished)
	{
		return;
	}

	if (!bEnabled || !Instance.Recipe || !IsTargetAlive(Instance))
	{
		FinishInstance(Instance, true);
		return;
	}

	Instance.Elapsed += RealDeltaSeconds;

	float BlendWeight = 1.0f;
	if (Instance.bStopping)
	{
		Instance.StopElapsed += RealDeltaSeconds;
		const float BlendOutTime = GetDefault<UFeelSettings>()->BlendOutTime;
		BlendWeight = BlendOutTime > 0.0f ? 1.0f - static_cast<float>(Instance.StopElapsed) / BlendOutTime : 0.0f;
		if (BlendWeight <= 0.0f)
		{
			FinishInstance(Instance, true);
			return;
		}
	}

	RefreshAccumulatorParameters(Instance);
	const UFeelRecipe& Recipe = *Instance.Recipe;
	if (Instance.TrackScales.Num() != Recipe.Tracks.Num())
	{
		Instance.TrackScales.Init(1.0f, Recipe.Tracks.Num());
	}
	FFeelEvalParams Params = MakeEvalParams(Instance, BlendWeight);
	Instance.bHasComfortSnapshot = Params.Comfort.Scales != nullptr;
	if (Params.Comfort.Scales)
	{
		Instance.ComfortSnapshot = *Params.Comfort.Scales;
	}
	auto MakeTrackContext = [this, &Instance](int32 TrackIndex, float TrackIntensity)
	{
		return MakeContext(Instance, TrackIndex, TrackIntensity);
	};

	// Side effects (OnStart / OnStop) through the lifecycle shared with the editor preview. A sustain loop first finishes
	// the region, then rewinds so tracks inside it start again.
	if (Instance.Clock.Advance(Recipe, RealDeltaSeconds))
	{
		Instance.Lifecycle.Update(Recipe, Recipe.SustainEnd, Params, MakeTrackContext);
		Instance.Lifecycle.Rewind(Recipe, Recipe.SustainStart, MakeTrackContext);
	}
	const float Time = Instance.Clock.RecipeTime;
	Instance.Lifecycle.Update(Recipe, Time, Params, MakeTrackContext);

	// Flashes that just started pass through their player's flash limiter.
	for (int32 StartedTrack : Instance.Lifecycle.GetStartedThisUpdate())
	{
		const FFeelTrack& Track = Recipe.Tracks[StartedTrack];
		if (!Params.Comfort.Scales || Params.Comfort.GetGroup(Track.Channel) != EFeelComfortGroup::Flashes)
		{
			continue;
		}
		const APlayerController* PlayerController = Instance.MakeTarget().ResolvePlayerController(GetWorld());
		FFeelFlashLimiter& Limiter = FlashLimiters.FindOrAdd(FObjectKey(PlayerController));
		Instance.TrackScales[StartedTrack] = Limiter.RegisterFlash(AccumulatorTime, *Params.Comfort.Scales);
	}

	// Recipe edits during PIE (such as a longer track) apply even to instances already playing.
	Instance.Duration = FFeelEvaluator::GetRecipeDuration(Recipe, Params);

	const bool bLooping = FFeelPlaybackClock::HasSustain(Recipe) && !Instance.Clock.bReleased;
	if (!bLooping && Time > Instance.Duration)
	{
		FinishInstance(Instance, false);
		return;
	}

	// Tracks for the play target and tracks for the instigator go to separate outputs, delivered to their own actor.
	Params.TargetFilter = EFeelTrackTarget::PlayTarget;
	FFeelOutputAccumulator TargetAccumulator;
	FFeelEvaluator::Evaluate(*Instance.Recipe, Time, Params, TargetAccumulator);
	Instance.Output = TargetAccumulator.Output;

	Instance.InstigatorOutput.Reset();
	if (Params.bHasInstigator && FFeelEvaluator::HasInstigatorTracks(*Instance.Recipe))
	{
		Params.TargetFilter = EFeelTrackTarget::Instigator;
		FFeelOutputAccumulator InstigatorAccumulator;
		FFeelEvaluator::Evaluate(*Instance.Recipe, Time, Params, InstigatorAccumulator);
		Instance.InstigatorOutput = InstigatorAccumulator.Output;
	}
}

void UFeelSubsystem::FinishInstance(FFeelInstance& Instance, bool bInterrupted, bool bNotifySteps)
{
	if (Instance.bFinished)
	{
		return;
	}
	Instance.bFinished = true;

	if (bNotifySteps)
	{
		Instance.Lifecycle.StopAll(Instance.Recipe, bInterrupted, [this, &Instance](int32 TrackIndex, float TrackIntensity)
		{
			return MakeContext(Instance, TrackIndex, TrackIntensity);
		});
	}

#if !UE_BUILD_SHIPPING
	// Moment capture: everything needed to replay this play exactly in the recipe editor. Also recorded when the world
	// ends mid-play (stopping Play In Editor), from values stored on the instance, so no player objects are needed.
	if (Instance.Recipe)
	{
		const FFeelTarget Target = Instance.MakeTarget();
		const AActor* TargetActor = Target.GetActor();

		FFeelPlayCapture Capture;
		Capture.Recipe = Instance.Recipe;
		Capture.RecipeName = Instance.Recipe->GetName();
		Capture.TargetName = Target.Widget ? Target.Widget->GetName() : (IsValid(TargetActor) ? TargetActor->GetActorNameOrLabel() : FString(TEXT("-")));
		Capture.WorldName = GetWorld() ? GetWorld()->GetMapName() : FString();
		Capture.EndedAt = FDateTime::Now();
		Capture.Seed = Instance.Seed;
		Capture.Intensity = Instance.Intensity;
		Capture.ParameterValues = Instance.ParameterValues;
		Capture.TargetDistance = Instance.TargetDistance;
		Capture.bTargetIsLocalPlayer = Instance.bTargetIsLocalPlayer;
		Capture.bHadInstigator = Instance.bHadInstigator;
		Capture.ViewDirection = Instance.ViewDirection;
		Capture.ViewDirectionFromLocation = Instance.ViewDirectionFromLocation;
		Capture.bHasComfort = Instance.bHasComfortSnapshot;
		Capture.ComfortScales = Instance.ComfortSnapshot;
		Capture.TrackScales = Instance.TrackScales;
		Capture.PlayedSeconds = static_cast<float>(Instance.Elapsed);
		Capture.bReleased = Instance.Clock.bReleased;
		Capture.bInterrupted = bInterrupted;
		FFeelPlayCaptureStore::Get().Add(MoveTemp(Capture));
	}
#endif

	// FEELKIT_PRO_BEGIN
	// Copied first: listeners may start new plays.
	const FFeelHandle Handle(Instance.Id);
	UFeelRecipe* FinishedRecipe = Instance.Recipe;
	OnFeelFinished.Broadcast(Handle, FinishedRecipe, bInterrupted);
	// FEELKIT_PRO_END
}

void UFeelSubsystem::ResolveAndDeliver()
{
	using FOutputList = TArray<const FFeelFrameOutput*, TInlineAllocator<8>>;

	UWorld* World = GetWorld();
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	UObject* WorldSettings = World ? World->GetWorldSettings(false, false) : nullptr;

	TMap<FObjectKey, FOutputList> PlayerInputs;
	TMap<FObjectKey, TWeakObjectPtr<APlayerController>> PlayerControllers;
	FOutputList ScreenInputs;

	ActorDelivery.BeginFrame();
	WidgetDelivery.BeginFrame();
	TArray<FFeelSoundClassAdjust, TInlineAllocator<1>> SoundClassAdjusts;

	// Each instance delivers up to two outputs: its play target's, and its instigator's (tracks set to apply to the instigator).
	TArray<TPair<FFeelTarget, const FFeelFrameOutput*>, TInlineAllocator<16>> Deliveries;
	for (const FFeelInstance& Instance : Instances)
	{
		Deliveries.Emplace(Instance.MakeTarget(), &Instance.Output);
		if (AActor* InstigatorActor = Instance.Instigator.Get())
		{
			Deliveries.Emplace(FFeelTarget::FromActor(InstigatorActor), &Instance.InstigatorOutput);
		}
	}

	const bool bLocalizeGlobalTime = GetDefault<UFeelSettings>()->ShouldLocalizeGlobalTimeDilation(World->GetNetMode());
	for (const TPair<FFeelTarget, const FFeelFrameOutput*>& Delivery : Deliveries)
	{
		const FFeelTarget& Target = Delivery.Key;
		const FFeelFrameOutput& Output = *Delivery.Value;

		if (APlayerController* PlayerController = Target.ResolvePlayerController(World))
		{
			const FObjectKey PlayerKey(PlayerController);
			PlayerInputs.FindOrAdd(PlayerKey).Add(&Output);
			PlayerControllers.Add(PlayerKey, PlayerController);
		}
		// Camera, screen and controller effects for another machine's player show on that machine only.
		if (!Target.BelongsToRemotePlayer())
		{
			ScreenInputs.Add(&Output);
		}

		AActor* TargetActor = Target.GetActor();
		if (bLocalizeGlobalTime)
		{
			// World time is shared in networked games, so global requests slow only the play's actor here.
			TimeArbiter.Submit(TargetActor, Output.GlobalTimeDilation);
		}
		else
		{
			TimeArbiter.Submit(WorldSettings, Output.GlobalTimeDilation);
		}
		if (TargetActor)
		{
			TimeArbiter.Submit(TargetActor, Output.TargetTimeDilation);
		}

		if (!Output.TargetScaleDelta.IsZero() || !Output.TargetLocationOffset.IsZero() || !Output.TargetRotationOffset.IsZero())
		{
			TArray<USceneComponent*, TInlineAllocator<4>> MotionComponents;
			FFeelActorDelivery::GetMotionComponents(Target.GetSceneComponent(), Target.Type == EFeelTargetType::Actor, MotionComponents);
			for (USceneComponent* MotionComponent : MotionComponents)
			{
				ActorDelivery.AddScale(MotionComponent, Output.TargetScaleDelta);
				ActorDelivery.AddTransform(MotionComponent, Output.TargetLocationOffset, Output.TargetRotationOffset);
			}
		}
		ActorDelivery.AddLight(Target.GetSceneComponent(), Output.LightIntensityDelta, Output.LightColor, Output.LightColorWeight);
		WidgetDelivery.AddWidget(Target.Widget, Output.WidgetTranslation, Output.WidgetScaleDelta, Output.WidgetAngle, Output.WidgetColor, Output.WidgetColorWeight);
		for (const FFeelSoundClassAdjust& Adjust : Output.SoundClassAdjusts)
		{
			FFeelFrameOutput::MergeSoundClassAdjust(SoundClassAdjusts, Adjust);
		}

		if (Output.MaterialParameters.Num() > 0)
		{
			// A primitive component target gets the parameters itself; otherwise every mesh of the target actor does.
			UPrimitiveComponent* PrimitiveTarget = Target.Type == EFeelTargetType::SceneComponent ? Cast<UPrimitiveComponent>(Target.GetSceneComponent()) : nullptr;
			if (PrimitiveTarget)
			{
				ActorDelivery.AddMaterialParameters(PrimitiveTarget, Output.MaterialParameters);
			}
			else if (TargetActor)
			{
				TInlineComponentArray<UMeshComponent*> Meshes(TargetActor);
				for (UMeshComponent* Mesh : Meshes)
				{
					ActorDelivery.AddMaterialParameters(Mesh, Output.MaterialParameters);
				}
			}
		}

		if (Output.OverlayFlashAmount > 0.0f && Output.OverlayFlashMaterial)
		{
			// Same targets as material parameters: a mesh component target itself, otherwise every mesh of the target actor.
			UMeshComponent* MeshTarget = Target.Type == EFeelTargetType::SceneComponent ? Cast<UMeshComponent>(Target.GetSceneComponent()) : nullptr;
			if (MeshTarget)
			{
				ActorDelivery.AddOverlayFlash(MeshTarget, Output.OverlayFlashMaterial, Output.OverlayFlashColor, Output.OverlayFlashAmount);
			}
			else if (TargetActor)
			{
				TInlineComponentArray<UMeshComponent*> Meshes(TargetActor);
				for (UMeshComponent* Mesh : Meshes)
				{
					ActorDelivery.AddOverlayFlash(Mesh, Output.OverlayFlashMaterial, Output.OverlayFlashColor, Output.OverlayFlashAmount);
				}
			}
		}
	}

	// Camera and force feedback per local player.
	FFeelCameraCaps Caps;
	Caps.MaxLocationOffset = Settings->MaxCameraLocationOffset;
	Caps.MaxRotationOffset = Settings->MaxCameraRotationOffset;
	Caps.MaxFieldOfViewOffset = Settings->MaxFieldOfViewOffset;

	CameraOutputs.Reset();
	for (const TPair<FObjectKey, FOutputList>& Pair : PlayerInputs)
	{
		FFeelFrameOutput& PlayerOutput = CameraOutputs.Add(Pair.Key);
		FeelArbiters::ResolveCamera(Pair.Value, Settings->CameraArbitration, Caps, PlayerOutput);
		FeelArbiters::ResolveForceFeedback(Pair.Value, PlayerOutput);
	}
	DeliverForceFeedback(PlayerControllers);

	// Screen.
	ScreenOutput.Reset();
	FeelArbiters::ResolveScreen(ScreenInputs, ScreenOutput);

	// Time.
	TimeArbiter.Apply(&FeelSubsystemPrivate::ReadDilation, &FeelSubsystemPrivate::WriteDilation);

	// Actor, widget and audio changes, restoring whatever is no longer requested.
	ActorDelivery.EndFrame();
	WidgetDelivery.EndFrame();
	AudioDelivery.Apply(World, SoundClassAdjusts);

	if (Instances.Num() > 0)
	{
		EnsureCameraModifiers();
	}
}

void UFeelSubsystem::DeliverForceFeedback(const TMap<FObjectKey, TWeakObjectPtr<APlayerController>>& PlayerControllers)
{
	for (TPair<FObjectKey, FForceFeedbackPlayer>& Pair : ForceFeedbackPlayers)
	{
		Pair.Value.bRequested = false;
	}

	for (const TPair<FObjectKey, FFeelFrameOutput>& Pair : CameraOutputs)
	{
		const FFeelForceFeedbackValues& Values = Pair.Value.ForceFeedback;
		const TWeakObjectPtr<APlayerController>* WeakController = PlayerControllers.Find(Pair.Key);
		APlayerController* PlayerController = WeakController ? WeakController->Get() : nullptr;
		if (Values.IsZero() || !PlayerController)
		{
			continue;
		}

		FForceFeedbackPlayer& Player = ForceFeedbackPlayers.FindOrAdd(Pair.Key);
		Player.PlayerController = PlayerController;
		Player.bRequested = true;

		// FeelKit values already include comfort. Comfort also scales the controller's force feedback, so divide it out
		// here: only the comfort part, never a scale the game set itself.
		float Compensation = 1.0f;
		const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		const UFeelComfortSubsystem* Comfort = LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr;
		const float ComfortScale = Comfort ? Comfort->GetEffectiveForceFeedbackScale() : 1.0f;
		if (ComfortScale > UE_KINDA_SMALL_NUMBER)
		{
			Compensation = 1.0f / ComfortScale;
		}

		const float MotorValues[FeelSubsystemPrivate::NumMotors] = { Values.LeftLarge, Values.LeftSmall, Values.RightLarge, Values.RightSmall };
		for (int32 Motor = 0; Motor < FeelSubsystemPrivate::NumMotors; ++Motor)
		{
			FeelSubsystemPrivate::UpdateMotor(*PlayerController, Player.MotorHandles[Motor], Motor, FMath::Min(MotorValues[Motor] * Compensation, 1.0f));
		}
	}

	for (auto It = ForceFeedbackPlayers.CreateIterator(); It; ++It)
	{
		if (!It.Value().bRequested)
		{
			StopForceFeedback(It.Value());
			It.RemoveCurrent();
		}
	}
}

void UFeelSubsystem::StopForceFeedback(FForceFeedbackPlayer& Player)
{
	APlayerController* PlayerController = Player.PlayerController.Get();
	for (int32 Motor = 0; Motor < FeelSubsystemPrivate::NumMotors; ++Motor)
	{
		if (PlayerController)
		{
			FeelSubsystemPrivate::UpdateMotor(*PlayerController, Player.MotorHandles[Motor], Motor, 0.0f);
		}
		Player.MotorHandles[Motor] = 0;
	}
}

void UFeelSubsystem::EnsureCameraModifiers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// The buyer adds nothing; every local player's camera manager gets the modifier on demand.
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		UFeelCameraModifier::EnsureOn(It->Get());
	}
}

void UFeelSubsystem::RestoreEverything()
{
	TimeArbiter.RestoreAll(&FeelSubsystemPrivate::WriteDilation);
	ActorDelivery.RestoreAll();
	WidgetDelivery.RestoreAll();
	AudioDelivery.RestoreAll();

	for (TPair<FObjectKey, FForceFeedbackPlayer>& Pair : ForceFeedbackPlayers)
	{
		StopForceFeedback(Pair.Value);
	}
	ForceFeedbackPlayers.Reset();

	CameraOutputs.Reset();
	ScreenOutput.Reset();
}

bool UFeelSubsystem::IsTargetAlive(const FFeelInstance& Instance) const
{
	switch (Instance.TargetType)
	{
	case EFeelTargetType::Actor:
		return Instance.TargetActor.IsValid();
	case EFeelTargetType::SceneComponent:
		return Instance.TargetComponent.IsValid();
	case EFeelTargetType::Widget:
		return Instance.TargetWidget.IsValid();
	default:
		return true;
	}
}

FFeelContext UFeelSubsystem::MakeContext(const FFeelInstance& Instance, int32 TrackIndex, float TrackIntensity) const
{
	const bool bValidTrack = Instance.Recipe && Instance.Recipe->Tracks.IsValidIndex(TrackIndex);
	const bool bAppliesToInstigator = bValidTrack && Instance.Recipe->Tracks[TrackIndex].AppliesTo == EFeelTrackTarget::Instigator;
	const FFeelTarget Target = bAppliesToInstigator ? FFeelTarget::FromActor(Instance.Instigator.Get()) : Instance.MakeTarget();
	APlayerController* PlayerController = Target.ResolvePlayerController(GetWorld());

	FFeelContext Context;
	Context.World = GetWorld();
	Context.Target = Target.GetActor();
	Context.TargetComponent = Target.GetSceneComponent();
	Context.Instigator = Instance.Instigator.Get();
	Context.PlayContext = Instance.PlayContext;
	Context.PlayContext.Instigator = Context.Instigator;
	Context.PlayContext.Parameters = Instance.ParameterValues;
	Context.PlayerController = PlayerController;
	Context.Recipe = Instance.Recipe;
	Context.ElapsedRealTime = static_cast<float>(Instance.Elapsed);
	Context.Intensity = TrackIntensity;
	Context.InstanceId = Instance.Id;
	Context.TrackIndex = TrackIndex;
	Context.TrackDuration = bValidTrack ? FFeelEvaluator::GetTrackDuration(*Instance.Recipe, TrackIndex, MakeEvalParams(Instance, 1.0f)) : 0.0f;

	if (Target.Type == EFeelTargetType::WorldLocation)
	{
		Context.TargetLocation = Target.Location;
	}
	else if (Target.Type == EFeelTargetType::LocalPlayerCamera && PlayerController && PlayerController->PlayerCameraManager)
	{
		Context.TargetLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
	}
	else if (Context.TargetComponent)
	{
		Context.TargetLocation = Context.TargetComponent->GetComponentLocation();
	}
	else if (Context.Target)
	{
		Context.TargetLocation = Context.Target->GetActorLocation();
	}
	return Context;
}

FFeelComfortContext UFeelSubsystem::MakeComfortContext(const FFeelInstance& Instance) const
{
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();

	FFeelComfortContext Context;
	Context.Scales = &Settings->DefaultComfortScales;
	Context.Mappings = Settings->ChannelComfortGroups;

	// Use the comfort of the local player this instance plays for; project defaults otherwise.
	const APlayerController* PlayerController = Instance.MakeTarget().ResolvePlayerController(GetWorld());
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	if (const UFeelComfortSubsystem* PlayerComfort = LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr)
	{
		Context.Scales = &PlayerComfort->GetComfortScalesRef();
	}
	return Context;
}

FFeelEvalParams UFeelSubsystem::MakeEvalParams(const FFeelInstance& Instance, float BlendWeight) const
{
	FFeelEvalParams Params;
	// FEELKIT_PRO_BEGIN
	Params.Intensity = Instance.Intensity * BlendWeight * FMath::Max(CVarFeelGlobalScale.GetValueOnGameThread(), 0.0f);
	// FEELKIT_PRO_END
	// FEELKIT_LITE: Params.Intensity = Instance.Intensity * BlendWeight;
	Params.InstanceSeed = Instance.Seed;
	Params.WorldDirection = Instance.WorldDirection;
	Params.ViewDirection = Instance.ViewDirection;
	Params.ViewDirectionFromLocation = Instance.ViewDirectionFromLocation;
	Params.TargetDistance = Instance.TargetDistance;
	Params.bTargetIsLocalPlayer = Instance.bTargetIsLocalPlayer;
	Params.ParameterValues = &Instance.ParameterValues;
	Params.bHasInstigator = Instance.Instigator.IsValid();
	Params.TrackScales = Instance.TrackScales;
	Params.Comfort = MakeComfortContext(Instance);
	return Params;
}

FObjectKey UFeelSubsystem::MakeTargetKey(const FFeelTarget& Target) const
{
	switch (Target.Type)
	{
	case EFeelTargetType::Actor:
		return FObjectKey(Target.GetActor());
	case EFeelTargetType::SceneComponent:
		return FObjectKey(Target.GetSceneComponent());
	case EFeelTargetType::LocalPlayerCamera:
		return FObjectKey(Target.ResolvePlayerController(GetWorld()));
	case EFeelTargetType::Widget:
		return FObjectKey(Target.Widget.Get());
	default:
		return FObjectKey();
	}
}
