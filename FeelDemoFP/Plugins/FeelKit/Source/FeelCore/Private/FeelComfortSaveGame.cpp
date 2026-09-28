// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortSaveGame.h"

#include "Engine/LocalPlayer.h"
#include "FeelSettings.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelComfortSaveGame)

bool UFeelSaveGameComfortStorage::LoadComfortScales_Implementation(ULocalPlayer* LocalPlayer, FFeelComfortScales& OutScales)
{
	return LoadFromSlot(GetSlotName(LocalPlayer), GetUserIndex(LocalPlayer), OutScales);
}

bool UFeelSaveGameComfortStorage::SaveComfortScales_Implementation(ULocalPlayer* LocalPlayer, const FFeelComfortScales& Scales)
{
	return SaveToSlot(GetSlotName(LocalPlayer), GetUserIndex(LocalPlayer), Scales);
}

FString UFeelSaveGameComfortStorage::GetSlotName(const ULocalPlayer* LocalPlayer)
{
	return FString::Printf(TEXT("%s_%d"), *GetDefault<UFeelSettings>()->ComfortSaveSlotPrefix, LocalPlayer ? LocalPlayer->GetLocalPlayerIndex() : 0);
}

int32 UFeelSaveGameComfortStorage::GetUserIndex(const ULocalPlayer* LocalPlayer)
{
	return LocalPlayer ? LocalPlayer->GetPlatformUserIndex() : 0;
}

bool UFeelSaveGameComfortStorage::SaveToSlot(const FString& SlotName, int32 UserIndex, const FFeelComfortScales& Scales)
{
	UFeelComfortSaveGame* SaveGame = Cast<UFeelComfortSaveGame>(UGameplayStatics::CreateSaveGameObject(UFeelComfortSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return false;
	}
	SaveGame->Scales = Scales;
	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

bool UFeelSaveGameComfortStorage::LoadFromSlot(const FString& SlotName, int32 UserIndex, FFeelComfortScales& OutScales)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		return false;
	}

	const UFeelComfortSaveGame* SaveGame = Cast<UFeelComfortSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
	if (!SaveGame)
	{
		return false;
	}
	OutScales = SaveGame->Scales;
	return true;
}
