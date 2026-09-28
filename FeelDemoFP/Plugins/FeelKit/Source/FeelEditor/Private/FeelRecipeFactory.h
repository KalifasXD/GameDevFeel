// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "FeelRecipeFactory.generated.h"

/** Creates Feel Recipe assets from the Content Browser. */
UCLASS()
class UFeelRecipeFactory : public UFactory
{
	GENERATED_BODY()

public:
	UFeelRecipeFactory();

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
