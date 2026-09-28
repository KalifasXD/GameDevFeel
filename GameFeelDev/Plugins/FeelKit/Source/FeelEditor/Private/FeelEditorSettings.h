// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FeelEditorSettings.generated.h"

/** Per-user settings of the FeelKit recipe editor. */
UCLASS(Config = EditorPerProjectUserSettings, meta = (DisplayName = "FeelKit"))
class UFeelEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Snap timeline edits to whole frames. */
	UPROPERTY(Config, EditAnywhere, Category = "Timeline")
	bool bSnapToFrames = true;

	/** Frame rate used for snapping. */
	UPROPERTY(Config, EditAnywhere, Category = "Timeline", meta = (ClampMin = "1", ClampMax = "240"))
	int32 SnapFrameRate = 60;

	/**
	 * Allows editing and saving the recipes that ship with FeelKit (/FeelKit/Library). They are read-only by default,
	 * because the plugin is shared by every project on this engine: copy a library recipe into your project instead
	 * (right-click in the Content Browser, FeelKit, Recipe from Template). Turn this on only to author the library itself.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Library")
	bool bAllowLibraryEditing = false;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual FName GetContainerName() const override { return TEXT("Editor"); }
	virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};
