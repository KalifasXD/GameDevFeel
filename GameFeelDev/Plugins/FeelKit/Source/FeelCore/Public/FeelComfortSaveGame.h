// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortStorage.h"
#include "FeelComfortTypes.h"
#include "GameFramework/SaveGame.h"
#include "FeelComfortSaveGame.generated.h"

/** Save game holding one player's comfort scales. */
UCLASS()
class FEELCORE_API UFeelComfortSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Format version of this save. */
	UPROPERTY()
	int32 Version = 1;

	/** The player's comfort scales. */
	UPROPERTY()
	FFeelComfortScales Scales;
};

/** Default comfort storage: one save game slot per local player. */
UCLASS()
class FEELCORE_API UFeelSaveGameComfortStorage : public UObject, public IFeelComfortStorage
{
	GENERATED_BODY()

public:
	virtual bool LoadComfortScales_Implementation(ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales) override;
	virtual bool SaveComfortScales_Implementation(ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales) override;

	/** Slot name for a player: the project's slot prefix plus the local player index. */
	static FString GetSlotName(const ULocalPlayer* LocalPlayer);

	/** Platform user index used for the slot. */
	static int32 GetUserIndex(const ULocalPlayer* LocalPlayer);

	static bool SaveToSlot(const FString& SlotName, int32 UserIndex, const FFeelComfortScales& Scales);
	static bool LoadFromSlot(const FString& SlotName, int32 UserIndex, FFeelComfortScales& OutScales);
};
