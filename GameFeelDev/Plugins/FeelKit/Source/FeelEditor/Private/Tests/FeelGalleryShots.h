// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"

/**
 * Pictures of the demo levels for the store gallery. The playthrough diagnostics call Take at their moments when the
 * environment variable FEELKIT_GALLERY is set (Tools/Run/run_gallery.ps1). Each picture is the game's scene without its
 * interface, rendered by HighResShot at 3840 x 2160 on the next frame, to Saved/FeelKit/Gallery/<Level>_<Name>.png.
 */
namespace FeelGalleryShots
{
	inline bool IsOn()
	{
		return !FPlatformMisc::GetEnvironmentVariable(TEXT("FEELKIT_GALLERY")).IsEmpty();
	}

	inline void Take(UWorld* World, const FString& Level, const FString& Name, FAutomationTestBase* Test)
	{
		const FWorldContext* Context = World ? GEngine->GetWorldContextFromWorld(World) : nullptr;
		UGameViewportClient* Client = Context ? Context->GameViewport : nullptr;
		if (!IsOn() || !Client)
		{
			return;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("Gallery") / FString::Printf(TEXT("%s_%s.png"), *Level, *Name);
		Client->Exec(World, *FString::Printf(TEXT("HighResShot 3840x2160 filename=\"%s\""), *File), *GLog);
		Test->AddInfo(FString::Printf(TEXT("GALLERY %s (requested)"), *File));
	}
}

#endif
