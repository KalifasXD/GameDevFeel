// Copyright 2026 Billo. All Rights Reserved.

#include "FeelMapAssetTypes.h"

#include "FeelMap.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelMapAssetTypes)

#define LOCTEXT_NAMESPACE "FeelEditor"

UFeelMapFactory::UFeelMapFactory()
{
	SupportedClass = UFeelMap::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UFeelMapFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFeelMap>(InParent, InClass, InName, Flags | RF_Transactional);
}

FText UAssetDefinition_FeelMap::GetAssetDisplayName() const
{
	return LOCTEXT("FeelMapAssetName", "Feel Map");
}

FLinearColor UAssetDefinition_FeelMap::GetAssetColor() const
{
	return FLinearColor(0.95f, 0.7f, 0.2f);
}

TSoftClassPtr<UObject> UAssetDefinition_FeelMap::GetAssetClass() const
{
	return UFeelMap::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_FeelMap::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { FAssetCategoryPath(LOCTEXT("FeelKitAssetCategory", "FeelKit")) };
	return Categories;
}

#undef LOCTEXT_NAMESPACE
