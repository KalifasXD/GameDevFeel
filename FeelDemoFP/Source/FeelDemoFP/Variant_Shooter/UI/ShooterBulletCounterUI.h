// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

class UFeelRecipe;

/**
 *  Simple bullet counter UI widget for a first person shooter game
 */
UCLASS(abstract)
class FEELDEMOFP_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Allows Blueprint to update sub-widgets with the new bullet count */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** Allows Blueprint to update sub-widgets with the new life total and play a damage effect on the HUD */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);

	// FeelKit: updates the counter through Blueprint as before, then lets the HUD react to what changed
	void UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	// FeelKit: updates the life bar through Blueprint as before, then lets the HUD react to a loss of life
	void Damaged(float LifePercent);

protected:

	// FeelKit: one shot fired. Parameter Ammo: share of the magazine left, 0 to 1. Plays on the bullets widget.
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> ShotFeel;

	// FeelKit: the magazine ran dry. Plays on the bullets widget.
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> EmptyFeel;

	// FeelKit: the magazine was refilled. Plays on the bullets widget.
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> ReloadFeel;

	// FeelKit: life was lost. Plays on the life widget.
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	TObjectPtr<UFeelRecipe> HurtFeel;

	// FeelKit: the widget showing the bullets
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	FName BulletsWidgetName = TEXT("Image_33");

	// FeelKit: the widget holding the life bar
	UPROPERTY(EditDefaultsOnly, Category="Feel")
	FName LifeWidgetName = TEXT("Border_0");

private:

	// FeelKit
	void PlayOn(UFeelRecipe* Recipe, FName WidgetName, float Ammo = -1.0f);

	// FeelKit: what the counter showed last, to tell a shot from a reload or a weapon change
	int32 LastMagazineSize = 0;
	int32 LastBulletCount = 0;
	float LastLifePercent = 1.0f;
};
