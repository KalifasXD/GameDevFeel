// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FeelTypes.h" // FeelKit
#include "HorrorUI.generated.h"

class AHorrorCharacter;
class UFeelRecipe;

/**
 *  Simple UI for a first person horror game
 *  Manages character sprint meter display
 */
UCLASS(abstract)
class FEELDEMOFP_API UHorrorUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** Sets up delegate listeners for the passed character */
	void SetupCharacter(AHorrorCharacter* HorrorCharacter);

	/** Called when the character's sprint meter is updated */
	UFUNCTION()
	void OnSprintMeterUpdated(float Percent);

	/** Called when the character's sprint state changes */
	UFUNCTION()
	void OnSprintStateChanged(bool bSprinting);

protected:

	/** Passes control to Blueprint to update the sprint meter widgets */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint Meter Updated"))
	void BP_SprintMeterUpdated(float Percent);

	/** Passes control to Blueprint to update the sprint meter status */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint State Changed"))
	void BP_SprintStateChanged(bool bSprinting);

	// FeelKit: plays on the sprint meter from the moment it runs empty until it is full again (a sustained recipe, released then)
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> BreathlessFeel;

	// FeelKit: plays on the sprint meter when it is full again after running empty
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> RecoveredFeel;

	// FeelKit: the widget around the sprint meter
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	FName MeterWidgetName = TEXT("Border_0");

	// FeelKit: the sustained play while the meter recovers from empty
	FFeelHandle BreathlessPlay;
};
