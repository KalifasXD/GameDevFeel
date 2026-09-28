// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

class UFeelRecipe;

/**
 *  Simple scoreboard UI for a first person shooter game
 */
UCLASS(abstract)
class FEELDEMOFP_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Allows Blueprint to update score sub-widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);

	// FeelKit: updates the score through Blueprint as before, then lets the changed score react
	void UpdateScore(uint8 TeamByte, int32 Score);

protected:

	// FeelKit: a team's score went up. Parameter PlayerScored: 1 when the point is the player's (another team lost someone), 0 when the player's team did. Plays on that team's score widget.
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> ScoreFeel;

	// FeelKit: the player's team
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	uint8 PlayerTeam = 0;

	// FeelKit: the score widget of each team, by team number
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TArray<FName> TeamScoreWidgetNames = { TEXT("Team1Score"), TEXT("Team2Score") };
};
