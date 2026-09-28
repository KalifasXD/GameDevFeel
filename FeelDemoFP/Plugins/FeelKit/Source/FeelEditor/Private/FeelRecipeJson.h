// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UFeelRecipe;

/**
 * Recipe JSON import and export (Strategy 5E): a recipe as readable text, for review in version control, sharing on
 * forums, bulk edits in scripts, and moving recipes between projects. Everything a recipe stores is included: its
 * settings, parameters and tracks with their steps (including steps nested inside other steps). Asset references (sounds,
 * curves, materials) are written as asset paths; references the importing project does not have are left empty and
 * reported, so the rest of the recipe still imports.
 */
class FFeelRecipeJson
{
public:
	/** Value of the "format" field that identifies FeelKit recipe files. */
	static const TCHAR* FormatName;

	/** Writes a recipe as pretty-printed JSON. */
	static FString Export(const UFeelRecipe& Recipe);

	/**
	 * Replaces a recipe's contents with JSON written by Export. The recipe is only changed when the text is a valid FeelKit
	 * recipe. Records an undo step when called inside a transaction.
	 */
	static bool Import(UFeelRecipe& Recipe, const FString& Json, FText& OutError, TArray<FString>* OutMissingAssets = nullptr);

	/** Adds Export to JSON and Import from JSON to the recipe asset context menu. */
	static void RegisterMenus();
};
