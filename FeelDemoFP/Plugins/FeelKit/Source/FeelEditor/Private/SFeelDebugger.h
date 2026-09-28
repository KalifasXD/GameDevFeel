// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

struct FFeelDebugItem;
class SSearchBox;

/**
 * FeelKit Debugger (Tools > FeelKit Debugger): what FeelKit is doing in running Play In Editor and game worlds (plays,
 * accumulators, each local player's comfort) and the recent plays that can be opened and replayed in the recipe editor.
 * Laid out like the Outliner: a tree with column headers, updated in place while play runs.
 */
class SFeelDebugger : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelDebugger) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SFeelDebugger() override;

	static const FName TabName;

	/** Registers the nomad tab under the Tools menu. */
	static void RegisterTab();
	static void UnregisterTab();

	/** Column ids of the tree. */
	static const FName ColumnName;
	static const FName ColumnTarget;
	static const FName ColumnTime;
	static const FName ColumnValue;
	static const FName ColumnDetails;
	static const FName ColumnReplay;

private:
	EActiveTimerReturnType RefreshTimer(double InCurrentTime, float InDeltaTime);
	/** Rebuilds the tree from the running worlds and the recent plays, reusing items so selection and expansion stay. */
	void Refresh();
	TSharedPtr<FFeelDebugItem> FindOrAddItem(const FString& Id);
	bool PassesSearch(const FFeelDebugItem& Item) const;

	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FFeelDebugItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void GetItemChildren(TSharedPtr<FFeelDebugItem> Item, TArray<TSharedPtr<FFeelDebugItem>>& OutChildren);
	void OnItemDoubleClicked(TSharedPtr<FFeelDebugItem> Item);
	TSharedPtr<SWidget> OnContextMenuOpening();

	TSharedPtr<STreeView<TSharedPtr<FFeelDebugItem>>> TreeView;
	TSharedPtr<SSearchBox> SearchBox;
	TArray<TSharedPtr<FFeelDebugItem>> RootItems;

	/** Every item by id, kept between refreshes. */
	TMap<FString, TSharedPtr<FFeelDebugItem>> ItemsById;
	/** Ids seen before, so new items open expanded once and the user's own expansion is kept afterwards. */
	TSet<FString> KnownIds;
	FString SearchText;
	FDelegateHandle CaptureStoreHandle;
};
