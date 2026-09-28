// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeFactory.h"

#include "FeelRecipe.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelRecipeFactory)

UFeelRecipeFactory::UFeelRecipeFactory()
{
	SupportedClass = UFeelRecipe::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UFeelRecipeFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UFeelRecipe>(InParent, InClass, InName, Flags | RF_Transactional);
}
