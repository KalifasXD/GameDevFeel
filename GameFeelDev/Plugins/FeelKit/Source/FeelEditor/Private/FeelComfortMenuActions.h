// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWidgetBlueprint;

/**
 * Copy to Project for the comfort menu that ships with FeelKit. FeelKit's own files live in the engine folder and are
 * replaced by the next update, so a menu to restyle is copied into the project first.
 */
namespace FeelComfortMenuActions
{
	/** Adds Copy to Project to the Content Browser menu of comfort menus inside FeelKit. */
	void RegisterMenus();

	/** Asks where to save a copy, makes it, offers to use it as the project's comfort menu and opens it. */
	UWidgetBlueprint* CopyToProjectWithDialog(UWidgetBlueprint& Source);
}
