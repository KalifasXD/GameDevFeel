// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "TickableEditorObject.h"
#include "Toolkits/AssetEditorToolkit.h"

class FFeelRecipeEditorState;
class IDetailsView;
class SFeelIntensityGraph;
class SFeelPreviewViewport;
class SFeelTimeline;
class UFeelRecipe;

/** Asset editor for Feel Recipes: preview viewport, timeline, intensity graph and details panel. */
class FFeelRecipeEditorToolkit : public FAssetEditorToolkit, public FSelfRegisteringEditorUndoClient, public FTickableEditorObject
{
public:
	void InitRecipeEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UFeelRecipe* Recipe);

	//~ Begin FAssetEditorToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	//~ End FAssetEditorToolkit interface

	//~ Begin FEditorUndoClient interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient interface

	//~ Begin FTickableEditorObject interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableEditorObject interface

private:
	TSharedRef<SDockTab> SpawnViewportTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTimelineTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnIntensityTab(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);

	/** Banner shown above the timeline while the recipe is a read-only library recipe. */
	TSharedRef<SWidget> MakeReadOnlyBanner();

	void RefreshDetails();

	TSharedPtr<FFeelRecipeEditorState> State;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SFeelPreviewViewport> Viewport;
	TSharedPtr<SFeelTimeline> Timeline;
	TSharedPtr<SFeelIntensityGraph> IntensityGraph;
};
