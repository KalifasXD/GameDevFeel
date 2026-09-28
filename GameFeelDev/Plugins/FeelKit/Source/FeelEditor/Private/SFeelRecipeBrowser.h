// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortTypes.h"
#include "FeelRecipeFilter.h"
#include "Widgets/SCompoundWidget.h"

class FFeelRecipeEditorState;
class SBox;
class SEditableTextBox;
class SFeelPreviewViewport;
class UFeelRecipe;
template <typename ItemType> class SListView;

DECLARE_DELEGATE_OneParam(FOnFeelRecipePicked, UFeelRecipe*);

/**
 * Recipe browser: every recipe of the FeelKit library and of the project, filtered by feeling, genre, channel
 * and text, with a live preview of the recipe under the cursor and a one-click copy of a library recipe into the project.
 * In picker mode it lists only library recipes and reports the chosen one (Recipe from Template).
 */
class SFeelRecipeBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelRecipeBrowser)
		: _PickerMode(false)
	{}
		/** Lists only library recipes and reports the pick instead of copying it. */
		SLATE_ARGUMENT(bool, PickerMode)
		SLATE_EVENT(FOnFeelRecipePicked, OnRecipePicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFeelRecipeBrowser() override;

	/** Tab in Tools > FeelKit Recipe Browser. */
	static void RegisterTab();
	static void UnregisterTab();
	static const FName TabName;

	/** Selects a recipe in the list (clearing filters that hide it) and previews it. */
	void SelectRecipe(const FSoftObjectPath& RecipePath);

	/** Opens the browser as a modal picker and copies the chosen library recipe into TargetFolder. */
	static void OpenTemplatePicker(const FString& TargetFolder);

private:
	TSharedRef<SWidget> MakeFilterColumn();
	TSharedRef<SWidget> MakePreviewColumn();
	TSharedRef<class ITableRow> MakeTile(TSharedPtr<FFeelRecipeEntry> Entry, const TSharedRef<class STableViewBase>& OwnerTable);

	void RefreshEntries();
	void RefreshList();
	void SetPreviewRecipe(UFeelRecipe* Recipe, bool bLoop);
	void OnSelectionChanged(TSharedPtr<FFeelRecipeEntry> Entry, ESelectInfo::Type SelectInfo);
	TSharedPtr<FFeelRecipeEntry> FindHoveredEntry() const;
	void OnRowDoubleClicked(TSharedPtr<FFeelRecipeEntry> Entry);
	void UseSelected();
	void OpenSelected();
	void ShowSelectedInContentBrowser();
	UFeelRecipe* GetSelectedRecipe() const;
	TSharedPtr<FFeelRecipeEntry> GetSelectedEntry() const;
	void RebuildParameterSliders();
	TSharedRef<SWidget> MakeComfortMenu();
	FText GetComfortText() const;
	EActiveTimerReturnType TickPreview(double InCurrentTime, float InDeltaTime);
	void HandleAssetRegistryChange(const FAssetData& Asset);

	/** All recipes, and the ones the filter lets through. */
	TArray<FFeelRecipeEntry> AllEntries;
	TArray<TSharedPtr<FFeelRecipeEntry>> VisibleEntries;
	FFeelRecipeFilterState Filter;
	TMap<FName, int32> FeelingCounts;

	TSharedPtr<SListView<TSharedPtr<FFeelRecipeEntry>>> ListView;
	TSharedPtr<SBox> FilterBox;
	TSharedPtr<SBox> PreviewBox;
	TSharedPtr<SBox> ParameterSliderBox;
	TSharedPtr<SEditableTextBox> SearchBox;

	/** Thumbnails of the tiles, drawn by the recipe thumbnail renderer like the Content Browser's. */
	TSharedPtr<class FAssetThumbnailPool> ThumbnailPool;
	TMap<FName, TSharedPtr<class FAssetThumbnail>> Thumbnails;

	TSharedPtr<FFeelRecipeEditorState> PreviewState;
	TSharedPtr<SFeelPreviewViewport> PreviewViewport;
	TWeakObjectPtr<UFeelRecipe> PreviewRecipe;

	/** How long the cursor has rested on a row, so a passing cursor does not start playing. */
	float HoverSeconds = 0.0f;
	bool bMuted = false;
	bool bPickerMode = false;
	FOnFeelRecipePicked OnRecipePicked;

	/** Set while the preview should loop (a selected recipe) instead of playing once (a hovered one). */
	bool bLoopPreview = false;

	/** Row the cursor is on, so leaving it stops the hover preview. */
	TSharedPtr<FFeelRecipeEntry> HoveredEntry;

	/** Comfort preset the preview uses; unset means no comfort limits. */
	TOptional<EFeelBuiltInComfortPreset> ComfortPreset;

	FDelegateHandle AssetAddedHandle;
	FDelegateHandle AssetRemovedHandle;
	FDelegateHandle AssetUpdatedHandle;
};
