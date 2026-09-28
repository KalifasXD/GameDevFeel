// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortTypes.h"
#include "UObject/Interface.h"
#include "FeelComfortStorage.generated.h"

class ULocalPlayer;

UINTERFACE(BlueprintType, Blueprintable)
class FEELCORE_API UFeelComfortStorage : public UInterface
{
	GENERATED_BODY()
};

/**
 * Routes comfort settings to your own save system.
 * Pick the class in Project Settings > Plugins > FeelKit, or call Set Comfort Storage on a player's comfort subsystem.
 */
class FEELCORE_API IFeelComfortStorage
{
	GENERATED_BODY()

public:
	/**
	 * Loads a player's comfort scales.
	 * @param LocalPlayer	The player whose settings to load. Can be empty outside a game.
	 * @param OutScales		Receives the stored scales.
	 * @return				False when nothing is stored.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Feel|Comfort")
	bool LoadComfortScales(ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales);

	/**
	 * Stores a player's comfort scales.
	 * @param LocalPlayer	The player whose settings to store. Can be empty outside a game.
	 * @param Scales		The scales to store.
	 * @return				False when storing failed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Feel|Comfort")
	bool SaveComfortScales(ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales);
};

namespace FeelComfort
{
	/** Loads through any object implementing IFeelComfortStorage, in C++ or Blueprint. False if it does not implement it. */
	FEELCORE_API bool LoadScales(UObject* Storage, ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales);

	/** Saves through any object implementing IFeelComfortStorage, in C++ or Blueprint. False if it does not implement it. */
	FEELCORE_API bool SaveScales(UObject* Storage, ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales);
}
