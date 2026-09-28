// Copyright 2026 Billo. All Rights Reserved.

#include "AssetDefinition_FeelRecipe.h"

#include "FeelRecipe.h"
#include "FeelRecipeEditorToolkit.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_FeelRecipe)

#define LOCTEXT_NAMESPACE "FeelEditor"

FText UAssetDefinition_FeelRecipe::GetAssetDisplayName() const
{
	return LOCTEXT("FeelRecipeAssetName", "Feel Recipe");
}

FLinearColor UAssetDefinition_FeelRecipe::GetAssetColor() const
{
	return FLinearColor(1.0f, 0.45f, 0.1f);
}

TSoftClassPtr<UObject> UAssetDefinition_FeelRecipe::GetAssetClass() const
{
	return UFeelRecipe::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_FeelRecipe::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { FAssetCategoryPath(LOCTEXT("FeelKitAssetCategory", "FeelKit")) };
	return Categories;
}

FAssetOpenSupport UAssetDefinition_FeelRecipe::GetAssetOpenSupport(const FAssetOpenSupportArgs& OpenSupportArgs) const
{
	// The recipe editor shows library recipes read-only by itself, so the same editor serves both methods.
	return FAssetOpenSupport(OpenSupportArgs.OpenMethod, true);
}

EAssetCommandResult UAssetDefinition_FeelRecipe::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	for (UFeelRecipe* Recipe : OpenArgs.LoadObjects<UFeelRecipe>())
	{
		const TSharedRef<FFeelRecipeEditorToolkit> Toolkit = MakeShared<FFeelRecipeEditorToolkit>();
		Toolkit->InitRecipeEditor(OpenArgs.GetToolkitMode(), OpenArgs.ToolkitHost, Recipe);
	}
	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
