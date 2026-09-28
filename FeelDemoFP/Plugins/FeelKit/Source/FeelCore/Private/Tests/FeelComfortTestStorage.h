// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortStorage.h"
#include "FeelComfortTypes.h"
#include "FeelComfortTestStorage.generated.h"

/** In-memory comfort storage used by automation tests to check storage routing (CMF-011). */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UFeelComfortTestStorage : public UObject, public IFeelComfortStorage
{
	GENERATED_BODY()

public:
	FFeelComfortScales StoredScales;
	bool bHasStoredScales = false;
	int32 SaveCount = 0;

	virtual bool LoadComfortScales_Implementation(ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales) override
	{
		if (!bHasStoredScales)
		{
			return false;
		}
		OutScales = StoredScales;
		return true;
	}

	virtual bool SaveComfortScales_Implementation(ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales) override
	{
		StoredScales = Scales;
		bHasStoredScales = true;
		++SaveCount;
		return true;
	}
};
