// Copyright 2026 Billo. All Rights Reserved.

#include "FeelCameraModifier.h"

#include "Camera/CameraModifier_CameraShake.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "FeelComfortSubsystem.h"
#include "FeelComfortTypes.h"
#include "FeelFrameOutput.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelCameraModifier)

void UFeelCameraModifier::EnsureOn(APlayerController* PlayerController)
{
	APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager.Get() : nullptr;
	if (CameraManager && PlayerController->IsLocalController() && !CameraManager->FindCameraModifierByClass(UFeelCameraModifier::StaticClass()))
	{
		CameraManager->AddNewCameraModifier(UFeelCameraModifier::StaticClass());
	}
}

bool UFeelCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	const bool bStopChain = Super::ModifyCamera(DeltaTime, InOutPOV);

	APlayerCameraManager* CameraManager = CameraOwner;
	const UWorld* World = CameraManager ? CameraManager->GetWorld() : nullptr;
	if (!World)
	{
		return bStopChain;
	}

	// This player's comfort, or the project defaults without a local player.
	const APlayerController* PlayerController = CameraManager->GetOwningPlayerController();
	const ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
	const UFeelComfortSubsystem* Comfort = LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr;
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	const FFeelComfortScales& Scales = Comfort ? Comfort->GetComfortScalesRef() : Settings->DefaultComfortScales;

	if (Settings->bApplyComfortToEngineCameraShakes)
	{
		ScaleEngineCameraShakes(Scales);
	}

	const UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
	if (!Subsystem)
	{
		return bStopChain;
	}

	FFeelFrameOutput Output;
	Subsystem->GetCameraOutput(PlayerController, Output);

	// Motion comfort runs every frame, so a limited field of view change also eases back after the recipe ends.
	const float RealDeltaSeconds = static_cast<float>(FApp::GetDeltaTime());
	FeelComfort::ApplyMotionComfort(Scales, Output.CameraRotationOffset, Output.FieldOfViewOffset, PreviousFieldOfViewOffset, RealDeltaSeconds);
	Output.ApplyToView(InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV);

	// Post-process pulses blend over the scene, one weighted blend per parameter.
	Subsystem->GetScreenOutput().ForEachPostProcessBlend([CameraManager](FPostProcessSettings& PostProcess, float Weight)
	{
		CameraManager->AddCachedPPBlend(PostProcess, Weight);
	}, &PostProcessMaterialInstances);

	return bStopChain;
}

void UFeelCameraModifier::ScaleEngineCameraShakes(const FFeelComfortScales& Scales)
{
	const UCameraModifier_CameraShake* ShakeModifier = CameraOwner ? Cast<UCameraModifier_CameraShake>(CameraOwner->FindCameraModifierByClass(UCameraModifier_CameraShake::StaticClass())) : nullptr;
	if (!ShakeModifier)
	{
		EngineShakeBaseScales.Reset();
		return;
	}

	const float ComfortScale = FMath::Clamp(Scales.Master * Scales.CameraShake, 0.0f, 1.0f);
	TArray<FActiveCameraShakeInfo> ActiveShakes;
	ShakeModifier->GetActiveCameraShakes(ActiveShakes);

	TSet<UCameraShakeBase*> StillActive;
	for (const FActiveCameraShakeInfo& Info : ActiveShakes)
	{
		UCameraShakeBase* Shake = Info.ShakeInstance;
		if (!Shake)
		{
			continue;
		}
		StillActive.Add(Shake);
		const float* BaseScale = EngineShakeBaseScales.Find(Shake);
		if (!BaseScale)
		{
			BaseScale = &EngineShakeBaseScales.Add(Shake, Shake->ShakeScale);
		}
		Shake->ShakeScale = *BaseScale * ComfortScale;
	}

	for (auto It = EngineShakeBaseScales.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !StillActive.Contains(It.Key().Get()))
		{
			It.RemoveCurrent();
		}
	}
}
