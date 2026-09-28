// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/StyleColors.h"

/** Colors shared by the recipe editor widgets. */
namespace FeelEditorColors
{
	/**
	 * Sequencer's section color rule (FSequencerSectionPainter::BlendColor): a track tint blended over Unreal's section
	 * gray by the tint's alpha. Using the same rule keeps FeelKit's tracks in the muted palette Sequencer uses.
	 */
	inline FLinearColor BlendSequencerTint(const FColor& Tint)
	{
		const FLinearColor Base(FColor(71, 71, 71));
		const float Alpha = Tint.A / 255.0f;
		FLinearColor Color = Base * (1.0f - Alpha) + FLinearColor(FColor(Tint.R, Tint.G, Tint.B)) * Alpha;
		Color.A = 1.0f;
		return Color;
	}

	/**
	 * Color of a channel family, used for timeline bars, curves and the intensity graph. Tints come from the closest
	 * Sequencer track: camera from particle parameters (blue), actor from transform (teal), audio from audio, time from
	 * subsequences (red), UI from material (green), everything else from camera cuts (gray).
	 */
	inline FLinearColor GetChannelColor(const FGameplayTag& Channel)
	{
		const FString Name = Channel.GetTagName().ToString();
		if (Name.StartsWith(TEXT("Feel.Camera")))
		{
			return BlendSequencerTint(FColor(0, 170, 255, 65));
		}
		if (Name.StartsWith(TEXT("Feel.Screen")))
		{
			return BlendSequencerTint(FColor(255, 220, 0, 65));
		}
		if (Name.StartsWith(TEXT("Feel.Actor")))
		{
			return BlendSequencerTint(FColor(65, 173, 164, 65));
		}
		if (Name.StartsWith(TEXT("Feel.Time")))
		{
			return BlendSequencerTint(FColor(180, 0, 40, 65));
		}
		if (Name.StartsWith(TEXT("Feel.Audio")))
		{
			return BlendSequencerTint(FColor(93, 95, 136, 255));
		}
		if (Name.StartsWith(TEXT("Feel.Haptics")))
		{
			return BlendSequencerTint(FColor(124, 15, 124, 65));
		}
		if (Name.StartsWith(TEXT("Feel.UI")))
		{
			return BlendSequencerTint(FColor(64, 192, 64, 65));
		}
		return BlendSequencerTint(FColor(120, 120, 120, 65));
	}

	/** A channel color lifted for thin lines and small marks (outliner strip, curve line), which need more contrast. */
	inline FLinearColor GetChannelAccent(const FGameplayTag& Channel)
	{
		const FLinearColor Hsv = GetChannelColor(Channel).LinearRGBToHSV();
		return FLinearColor(Hsv.R, FMath::Min(Hsv.G * 1.6f, 1.0f), FMath::Min(Hsv.B * 1.9f, 1.0f)).HSVToLinearRGB();
	}

	/**
	 * Color of a feeling, used by the recipe browser, the Content Browser tiles and their filters. Taken from Unreal's own
	 * accent palette (FStyleColors), so it follows the editor theme. Unknown feelings are gray.
	 */
	inline FLinearColor GetFeelingColor(const FGameplayTag& Feeling)
	{
		const FString Name = Feeling.GetTagName().ToString();
		const FSlateColor* Accent = &FStyleColors::AccentGray;
		if (Name == TEXT("Feel.Feeling.Impact"))
		{
			Accent = &FStyleColors::AccentOrange;
		}
		else if (Name == TEXT("Feel.Feeling.Weight"))
		{
			Accent = &FStyleColors::AccentBrown;
		}
		else if (Name == TEXT("Feel.Feeling.Power"))
		{
			Accent = &FStyleColors::AccentYellow;
		}
		else if (Name == TEXT("Feel.Feeling.Speed"))
		{
			Accent = &FStyleColors::AccentBlue;
		}
		else if (Name == TEXT("Feel.Feeling.Reward"))
		{
			Accent = &FStyleColors::AccentGreen;
		}
		else if (Name == TEXT("Feel.Feeling.Danger"))
		{
			Accent = &FStyleColors::AccentRed;
		}
		else if (Name == TEXT("Feel.Feeling.Dread"))
		{
			Accent = &FStyleColors::AccentPurple;
		}
		else if (Name == TEXT("Feel.Feeling.Denial"))
		{
			Accent = &FStyleColors::AccentWhite;
		}
		else if (Name == TEXT("Feel.Feeling.Interface"))
		{
			Accent = &FStyleColors::AccentPink;
		}
		return Accent->GetColor(FWidgetStyle());
	}
}
