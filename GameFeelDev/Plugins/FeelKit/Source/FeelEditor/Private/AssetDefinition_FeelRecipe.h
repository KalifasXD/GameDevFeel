// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "AssetDefinition_FeelRecipe.generated.h"

/** Registers Feel Recipe assets and opens them in the recipe editor. */
UCLASS()
class UAssetDefinition_FeelRecipe : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	/**
	 * Recipes open for viewing as well as editing. The Content Browser opens assets in folders that do not allow edits
	 * (the FeelKit library) only for viewing; without this a library recipe would not open at all.
	 */
	virtual FAssetOpenSupport GetAssetOpenSupport(const FAssetOpenSupportArgs& OpenSupportArgs) const override;
	virtual EAssetCommandResult OpenAssets(const FAssetOpenArgs& OpenArgs) const override;
};
