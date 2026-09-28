// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "Factories/Factory.h"
#include "FeelMapAssetTypes.generated.h"

/** Creates Feel Map assets from the Content Browser. */
UCLASS()
class UFeelMapFactory : public UFactory
{
	GENERATED_BODY()

public:
	UFeelMapFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

/** Registers Feel Map assets in the FeelKit asset category. They open in the standard property editor. */
UCLASS()
class UAssetDefinition_FeelMap : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};
