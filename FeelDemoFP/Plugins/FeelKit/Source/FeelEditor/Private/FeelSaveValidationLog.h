// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UPackage;

/**
 * Keeps save validation messages for recipes current.
 *
 * On every save, Unreal validates the asset and appends the results to the Asset Check log page "Asset Save: <asset>",
 * which it reuses across saves. It opens the Message Log whenever that page holds any warning or error, including
 * messages from earlier saves, so a fixed recipe kept reopening the log. Before a recipe saves, its page is cleared,
 * so the log opens only when the recipe still has a problem.
 */
namespace FeelSaveValidationLog
{
	/** Subscribes to package saves. */
	void Startup();
	void Shutdown();

	/** Title Unreal's save validation gives the Asset Check page of one saved asset. */
	FText GetSavePageTitle(FName AssetName);

	/** Removes the messages on the Asset Check save page of this asset, if the page exists. Returns true when a page was cleared. */
	bool ClearSavePage(FName AssetName);
}
