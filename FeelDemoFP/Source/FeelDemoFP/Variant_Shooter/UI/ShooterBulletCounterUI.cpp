// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterBulletCounterUI.h"
#include "FeelBlueprintLibrary.h" // FeelKit
#include "FeelParameters.h" // FeelKit
#include "FeelTypes.h" // FeelKit

// FeelKit
void UShooterBulletCounterUI::UpdateBulletCounter(int32 MagazineSize, int32 BulletCount)
{
	BP_UpdateBulletCounter(MagazineSize, BulletCount);

	// the same magazine with fewer bullets is a shot, with more a reload; a different magazine is a weapon change
	if (MagazineSize > 0 && MagazineSize == LastMagazineSize)
	{
		if (BulletCount < LastBulletCount)
		{
			PlayOn(ShotFeel, BulletsWidgetName, static_cast<float>(BulletCount) / MagazineSize);
			if (BulletCount == 0)
			{
				PlayOn(EmptyFeel, BulletsWidgetName);
			}
		}
		else if (BulletCount > LastBulletCount)
		{
			PlayOn(ReloadFeel, BulletsWidgetName);
		}
	}

	LastMagazineSize = MagazineSize;
	LastBulletCount = BulletCount;
}

// FeelKit
void UShooterBulletCounterUI::Damaged(float LifePercent)
{
	BP_Damaged(LifePercent);

	if (LifePercent < LastLifePercent)
	{
		PlayOn(HurtFeel, LifeWidgetName);
	}
	LastLifePercent = LifePercent;
}

// FeelKit
void UShooterBulletCounterUI::PlayOn(UFeelRecipe* Recipe, FName WidgetName, float Ammo)
{
	UWidget* Widget = GetWidgetFromName(WidgetName);
	if (!Recipe || !Widget)
	{
		return;
	}
	FFeelPlayContext Context;
	if (Ammo >= 0.0f)
	{
		Context.Parameters.Add(TEXT("Ammo"), Ammo);
	}
	UFeelBlueprintLibrary::PlayFeelWithContext(this, Recipe, FFeelTarget::FromWidget(Widget), Context);
}
