// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortStorage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelComfortStorage)

namespace FeelComfort
{
	static bool ImplementsStorage(const UObject* Storage)
	{
		return Storage && Storage->GetClass()->ImplementsInterface(UFeelComfortStorage::StaticClass());
	}

	bool LoadScales(UObject* Storage, ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales)
	{
		return ImplementsStorage(Storage) && IFeelComfortStorage::Execute_LoadComfortScales(Storage, LocalPlayer, OutScales);
	}

	bool SaveScales(UObject* Storage, ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales)
	{
		return ImplementsStorage(Storage) && IFeelComfortStorage::Execute_SaveComfortScales(Storage, LocalPlayer, Scales);
	}
}
