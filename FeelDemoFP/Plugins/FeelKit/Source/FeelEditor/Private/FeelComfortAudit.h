// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"

class UFeelRecipe;
class UFeelSettings;

/** One thing the comfort audit found. */
struct FFeelComfortAuditFinding
{
	/** Warnings are likely comfort or accessibility problems; notes are worth a look. */
	bool bWarning = true;
	FText Message;

	/** The recipe and track the finding is about. Null recipe for project settings findings. */
	TWeakObjectPtr<const UFeelRecipe> Recipe;
	int32 TrackIndex = INDEX_NONE;
};

/**
 * Comfort audit: checks recipes and project settings against common photosensitivity and motion comfort
 * guidance, such as no more than three flashes in any second and no saturated red flashes. A readiness helper that
 * catches likely problems early, not a certification: test the finished game with a photosensitivity analysis tool.
 */
class FFeelComfortAudit
{
public:
	/** Flashes allowed to start within any one second. */
	static constexpr int32 MaxFlashesPerSecond = 3;

	/** Checks one recipe. */
	static void AuditRecipe(const UFeelRecipe& Recipe, const UFeelSettings& Settings, TArray<FFeelComfortAuditFinding>& OutFindings);

	/** Checks project comfort settings. */
	static void AuditSettings(const UFeelSettings& Settings, TArray<FFeelComfortAuditFinding>& OutFindings);

	/** Whether a color counts as saturated red: red makes up at least 80% of its color. */
	static bool IsSaturatedRed(const FLinearColor& Color);

	/** Loads every recipe in the project, audits it and the settings, and shows the results in the Message Log. */
	static void RunProjectAudit();

	/** Registers the message log listing. */
	static void Startup();
	static void Shutdown();
};
