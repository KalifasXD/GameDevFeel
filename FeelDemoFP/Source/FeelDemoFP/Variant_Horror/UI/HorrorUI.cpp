// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorUI.h"
#include "HorrorCharacter.h"
#include "FeelBlueprintLibrary.h" // FeelKit
#include "FeelTypes.h" // FeelKit

void UHorrorUI::SetupCharacter(AHorrorCharacter* HorrorCharacter)
{
	HorrorCharacter->OnSprintMeterUpdated.AddDynamic(this, &UHorrorUI::OnSprintMeterUpdated);
	HorrorCharacter->OnSprintStateChanged.AddDynamic(this, &UHorrorUI::OnSprintStateChanged);
}

void UHorrorUI::OnSprintMeterUpdated(float Percent)
{
	// call the BP handler
	BP_SprintMeterUpdated(Percent);

	// FeelKit: the meter pulses from the moment it runs empty until it is full again
	UWidget* Meter = GetWidgetFromName(MeterWidgetName);
	if (!Meter)
	{
		return;
	}
	if (Percent <= 0.0f && !BreathlessPlay.IsValid() && BreathlessFeel)
	{
		BreathlessPlay = UFeelBlueprintLibrary::PlayFeel(this, BreathlessFeel, FFeelTarget::FromWidget(Meter));
	}
	else if (Percent >= 1.0f && BreathlessPlay.IsValid())
	{
		UFeelBlueprintLibrary::ReleaseFeel(this, BreathlessPlay);
		BreathlessPlay = FFeelHandle();
		if (RecoveredFeel)
		{
			UFeelBlueprintLibrary::PlayFeel(this, RecoveredFeel, FFeelTarget::FromWidget(Meter));
		}
	}
}

void UHorrorUI::OnSprintStateChanged(bool bSprinting)
{
	// call the BP handler
	BP_SprintStateChanged(bSprinting);
}
