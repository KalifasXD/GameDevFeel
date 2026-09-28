// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/SCompoundWidget.h"

/** Rows of the controls panel: what the input does, then its keys. */
using FFeelSwitchControlRows = TArray<TPair<FText, FText>>;

/**
 * On-screen display of the Feel Switch: a start card in the middle of the screen that explains the switch, a small
 * ON/OFF badge in the top right corner, and optionally the level's controls in a panel under the badge. Switching
 * confirms on the badge itself (it briefly grows and brightens), never in the middle of the screen. Never takes input.
 * The owning Feel Switch pushes the state every frame.
 */
class SFeelSwitchOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelSwitchOverlay) {}
		SLATE_ARGUMENT(FText, Title)
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(FText, Hint)
		SLATE_ARGUMENT(FText, BadgeHint)
		/** Rows of the controls panel under the badge: what the input does, then its keys. Empty: no panel. */
		SLATE_ARGUMENT(FFeelSwitchControlRows, Controls)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Feel on or off, as shown on the badge. */
	void SetFeelEnabled(bool bInEnabled) { bEnabled = bInEnabled; }

	/** 0 = start card hidden, 1 = fully visible. */
	void SetCardOpacity(float InOpacity) { CardOpacity = InOpacity; }

	/** 0 = badge at rest, 1 = badge at the peak of its switch confirmation. */
	void SetBadgePulse(float InPulse) { BadgePulse = InPulse; }

	bool IsFeelEnabledShown() const { return bEnabled; }
	int32 GetControlRowCount() const { return ControlRowCount; }
	float GetCardOpacity() const { return CardOpacity; }
	float GetBadgePulse() const { return BadgePulse; }

private:
	TSharedRef<SWidget> MakeControls(const TArray<TPair<FText, FText>>& Controls);
	FText GetBadgeText() const;
	FSlateColor GetDotColor() const;
	FSlateColor GetBadgeBackground() const;
	TOptional<FSlateRenderTransform> GetBadgeTransform() const;
	FLinearColor GetCardColor() const;
	FSlateColor GetCardBackground() const;
	EVisibility GetCardVisibility() const;

	FSlateRoundedBoxBrush PanelBrush = FSlateRoundedBoxBrush(FLinearColor::White, 6.0f);
	FSlateRoundedBoxBrush DotBrush = FSlateRoundedBoxBrush(FLinearColor::White, 4.0f);
	bool bEnabled = true;
	int32 ControlRowCount = 0;
	float CardOpacity = 0.0f;
	float BadgePulse = 0.0f;
};
