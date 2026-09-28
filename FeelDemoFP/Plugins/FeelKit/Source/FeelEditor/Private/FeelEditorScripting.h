// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FeelEditorScripting.generated.h"

// FEELKIT_PRO_BEGIN
class UFeelMap;
// FEELKIT_PRO_END
class UFeelRecipe;

/**
 // FEELKIT_PRO_BEGIN
 * Editor scripting for FeelKit content, callable from Python and Editor Utility Blueprints: create recipes from JSON files,
 * refresh them after the JSON changes, and register Feel Maps in the project settings. Used to build the demo kits.
 // FEELKIT_PRO_END
 // FEELKIT_LITE: * Editor scripting for FeelKit content, callable from Python and Editor Utility Blueprints.
 */
UCLASS()
class UFeelEditorScripting : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// FEELKIT_PRO_BEGIN
	/**
	 * Replaces a recipe's content with a recipe JSON file (the same as Import from JSON in the Content Browser) and saves it.
	 * @param Recipe		Recipe to fill.
	 * @param FilePath		Path of the .json file.
	 * @param OutMessage	What happened, including asset references that are not in this project.
	 * @return				True when the file was imported.
	 */
	UFUNCTION(BlueprintCallable, Category = "FeelKit|Editor Scripting")
	static bool ImportRecipeFromJsonFile(UFeelRecipe* Recipe, const FString& FilePath, FString& OutMessage);

	/**
	 * Creates a recipe asset (or reuses the one already there) and fills it from a recipe JSON file, then saves it.
	 * @param PackagePath	Folder, for example /FeelKit/Demos/ActionRPG.
	 * @param AssetName		Name of the recipe asset.
	 * @param FilePath		Path of the .json file.
	 * @param OutMessage	What happened.
	 * @return				The recipe, or none when it could not be created or imported.
	 */
	UFUNCTION(BlueprintCallable, Category = "FeelKit|Editor Scripting")
	static UFeelRecipe* CreateRecipeFromJsonFile(const FString& PackagePath, const FString& AssetName, const FString& FilePath, FString& OutMessage);

	/**
	 * Adds a Feel Map to Project Settings > Plugins > FeelKit > Feel Maps (if it is not there yet) and saves the project settings.
	 * @param FeelMap	The Feel Map asset to add.
	 * @return	True when the map is in the list afterwards.
	 */
	UFUNCTION(BlueprintCallable, Category = "FeelKit|Editor Scripting")
	static bool AddFeelMapToProjectSettings(UFeelMap* FeelMap);

	/**
	 * Adds an accumulator to Project Settings > Plugins > FeelKit > Accumulators, or updates the one with the same name, and saves
	 * the project settings.
	 * @param Name				Accumulator name, as used by Add To Feel Accumulator and recipe parameters.
	 * @param MaxValue			Highest value it can reach.
	 * @param DecayPerSecond	How much it loses per second once it starts decaying.
	 * @param DecayDelay		Seconds after the last add before it starts decaying.
	 * @return					True when the accumulator is in the list afterwards.
	 */
	UFUNCTION(BlueprintCallable, Category = "FeelKit|Editor Scripting")
	static bool SetAccumulatorInProjectSettings(FName Name, float MaxValue, float DecayPerSecond, float DecayDelay);
	// FEELKIT_PRO_END

	/**
	 * Builds the comfort menu Widget Blueprint, or rebuilds it from scratch when it exists: the layout, the named controls
	 * the comfort menu binds to, the look, and the preview recipes (FR_Comfort_*) from PreviewFolder. Then compiles and
	 * saves it.
	 * @param PackagePath		Folder, for example /FeelKit/UI.
	 * @param AssetName			Name of the Widget Blueprint.
	 * @param PreviewFolder		Folder with the FR_Comfort_* preview recipes.
	 * @param OutMessage		What happened.
	 * @return					True when it was built, compiled without errors and saved.
	 */
	UFUNCTION(BlueprintCallable, Category = "FeelKit|Editor Scripting")
	static bool BuildComfortMenuWidget(const FString& PackagePath, const FString& AssetName, const FString& PreviewFolder, FString& OutMessage);
};
