// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSwitchOverlay.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FeelSwitch"

namespace FeelSwitchLook
{
	// Quiet, like Unreal's own on-screen text: dark translucent panels, white text, color only for the ON/OFF dot.
	const FLinearColor Panel(0.0f, 0.0f, 0.0f, 0.55f);
	const FLinearColor Text(1.0f, 1.0f, 1.0f, 1.0f);
	const FLinearColor SubText(0.78f, 0.78f, 0.78f, 1.0f);
	const FLinearColor HintText(0.6f, 0.6f, 0.6f, 1.0f);
	const FLinearColor On(0.30f, 0.80f, 0.40f, 1.0f);
	const FLinearColor Off(0.45f, 0.45f, 0.45f, 1.0f);
	constexpr float BadgeGrowth = 0.18f;
	constexpr float BadgeExtraOpacity = 0.3f;
}

void SFeelSwitchOverlay::Construct(const FArguments& InArgs)
{
	using namespace FeelSwitchLook;
	SetVisibility(EVisibility::HitTestInvisible);

	TSharedRef<SVerticalBox> Card = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(InArgs._Title)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
			.ColorAndOpacity(Text)
			.AutoWrapText(true)
		];
	if (!InArgs._Text.IsEmpty())
	{
		Card->AddSlot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(InArgs._Text)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(SubText)
				.AutoWrapText(true)
			];
	}
	Card->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 16.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(InArgs._Hint)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			.ColorAndOpacity(Text)
			.AutoWrapText(true)
		];

	ChildSlot
	[
		SNew(SOverlay)

		// Start card, a little above the middle so the character stays visible below it.
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(24.0f, 0.0f, 24.0f, 160.0f)
		[
			SNew(SBox)
			.MaxDesiredWidth(640.0f)
			.Visibility(this, &SFeelSwitchOverlay::GetCardVisibility)
			[
				SNew(SBorder)
				.BorderImage(&PanelBrush)
				.BorderBackgroundColor(this, &SFeelSwitchOverlay::GetCardBackground)
				.ColorAndOpacity(this, &SFeelSwitchOverlay::GetCardColor)
				.Padding(FMargin(28.0f, 20.0f))
				[
					Card
				]
			]
		]

		// Badge in the top right corner, the controls panel under it.
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Top)
		.Padding(24.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.BorderBackgroundColor(this, &SFeelSwitchOverlay::GetBadgeBackground)
			.Padding(FMargin(10.0f, 6.0f))
			.RenderTransformPivot(FVector2D(1.0, 0.0))
			.RenderTransform(this, &SFeelSwitchOverlay::GetBadgeTransform)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(8.0f)
					.HeightOverride(8.0f)
					[
						SNew(SImage)
						.Image(&DotBrush)
						.ColorAndOpacity(this, &SFeelSwitchOverlay::GetDotColor)
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(this, &SFeelSwitchOverlay::GetBadgeText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(Text)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(10.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(InArgs._BadgeHint)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(HintText)
				]
			]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				MakeControls(InArgs._Controls)
			]
		]
	];
}

TSharedRef<SWidget> SFeelSwitchOverlay::MakeControls(const TArray<TPair<FText, FText>>& Controls)
{
	using namespace FeelSwitchLook;
	ControlRowCount = Controls.Num();
	if (Controls.Num() == 0)
	{
		return SNullWidget::NullWidget;
	}

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int32 Row = 0; Row < Controls.Num(); ++Row)
	{
		const FMargin RowPadding(0.0f, Row > 0 ? 3.0f : 0.0f, 0.0f, 0.0f);
		Grid->AddSlot(0, Row)
			.Padding(RowPadding + FMargin(0.0f, 0.0f, 16.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(Controls[Row].Key)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(SubText)
			];
		Grid->AddSlot(1, Row)
			.Padding(RowPadding)
			[
				SNew(STextBlock)
				.Text(Controls[Row].Value)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				.ColorAndOpacity(Text)
			];
	}

	return SNew(SBorder)
		.BorderImage(&PanelBrush)
		.BorderBackgroundColor(Panel)
		.Padding(FMargin(12.0f, 8.0f))
		[
			Grid
		];
}

FText SFeelSwitchOverlay::GetBadgeText() const
{
	return bEnabled ? LOCTEXT("BadgeOn", "FEEL: ON") : LOCTEXT("BadgeOff", "FEEL: OFF");
}

FSlateColor SFeelSwitchOverlay::GetDotColor() const
{
	return bEnabled ? FeelSwitchLook::On : FeelSwitchLook::Off;
}

FSlateColor SFeelSwitchOverlay::GetBadgeBackground() const
{
	FLinearColor Color = FeelSwitchLook::Panel;
	Color.A = FMath::Min(1.0f, Color.A + FeelSwitchLook::BadgeExtraOpacity * BadgePulse);
	return Color;
}

TOptional<FSlateRenderTransform> SFeelSwitchOverlay::GetBadgeTransform() const
{
	if (BadgePulse <= 0.0f)
	{
		return TOptional<FSlateRenderTransform>();
	}
	return FSlateRenderTransform(1.0f + FeelSwitchLook::BadgeGrowth * BadgePulse);
}

FLinearColor SFeelSwitchOverlay::GetCardColor() const
{
	return FLinearColor(1.0f, 1.0f, 1.0f, CardOpacity);
}

FSlateColor SFeelSwitchOverlay::GetCardBackground() const
{
	FLinearColor Color = FeelSwitchLook::Panel;
	Color.A *= CardOpacity;
	return Color;
}

EVisibility SFeelSwitchOverlay::GetCardVisibility() const
{
	return CardOpacity > 0.0f ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

#undef LOCTEXT_NAMESPACE
