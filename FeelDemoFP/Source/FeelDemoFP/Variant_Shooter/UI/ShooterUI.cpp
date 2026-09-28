// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterUI.h"
#include "FeelBlueprintLibrary.h" // FeelKit
#include "FeelParameters.h" // FeelKit
#include "FeelTypes.h" // FeelKit

// FeelKit
void UShooterUI::UpdateScore(uint8 TeamByte, int32 Score)
{
	BP_UpdateScore(TeamByte, Score);

	UWidget* Widget = TeamScoreWidgetNames.IsValidIndex(TeamByte) ? GetWidgetFromName(TeamScoreWidgetNames[TeamByte]) : nullptr;
	if (!ScoreFeel || !Widget)
	{
		return;
	}
	FFeelPlayContext Context;
	Context.Parameters.Add(TEXT("PlayerScored"), TeamByte != PlayerTeam ? 1.0f : 0.0f);
	UFeelBlueprintLibrary::PlayFeelWithContext(this, ScoreFeel, FFeelTarget::FromWidget(Widget), Context);
}
