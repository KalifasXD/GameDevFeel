// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UFeelRecipe;
struct FAssetData;

/**
 * The recipe library that ships with FeelKit: ready-made recipes under /FeelKit/Library, grouped by feeling.
 * Library recipes are read-only, because the plugin lives in the engine folder and is shared by every project: a buyer
 * copies one into their project (Recipe from Template) and edits the copy, so a plugin update never changes their game.
 * The Allow Library Editing editor preference lifts the protection for authoring the library itself.
 */
namespace FeelLibrary
{
	/** Content path every library recipe lives under. */
	extern const TCHAR* LibraryRoot;

	/** Whether a package path is inside the library, for example /FeelKit/Library/Impact/FR_Impact_HeavyHit. */
	bool IsLibraryPath(FStringView PackageName);

	bool IsLibraryRecipe(const UFeelRecipe* Recipe);
	bool IsLibraryAsset(const FAssetData& Asset);

	/** Whether the Allow Library Editing preference is on. */
	bool IsLibraryEditingAllowed();

	/** Whether this recipe must not be changed: a library recipe while library editing is off. */
	bool IsReadOnly(const UFeelRecipe* Recipe);

	/** Makes the library folder read-only for saving, renaming and deleting, or writable while the preference is on. */
	void ApplyWritePermission();

	/** Called by the editor module. */
	void Startup();
	void Shutdown();

	/** Asset name a copy gets by default: FR_Impact_HeavyHit becomes HeavyHit. Other names are kept. */
	FString SuggestCopyName(const FString& LibraryAssetName);

	/** Copies a recipe into PackagePath/AssetName and records where it came from. Returns the copy, or null on failure. */
	UFeelRecipe* DuplicateRecipe(const UFeelRecipe& Source, const FString& PackagePath, const FString& AssetName);

	/** Folder a copy is offered in by default: the Content Browser's current folder, else /Game. */
	FString GetDefaultCopyFolder();

	/** Asks for a name and folder, copies the recipe there and opens it. Returns null when canceled. */
	UFeelRecipe* CopyToProjectWithDialog(const UFeelRecipe& Source, const FString& DefaultFolder);
}
