// Copyright 2026 Billo. All Rights Reserved.

#include "FeelNumberPopLayer.h"

#include "FeelSubsystem.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void SFeelNumberPopLayer::Construct(const FArguments& InArgs, UFeelSubsystem* InSubsystem)
{
	Subsystem = InSubsystem;
	SetVisibility(EVisibility::HitTestInvisible);
}

int32 SFeelNumberPopLayer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const UFeelSubsystem* CurrentSubsystem = Subsystem.Get();
	if (!CurrentSubsystem || CurrentSubsystem->GetNumberPops().Num() == 0 || !FSlateApplication::IsInitialized())
	{
		return LayerId;
	}

	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const float GeometryScale = FMath::Max(AllottedGeometry.Scale, 0.01f);
	const double Now = CurrentSubsystem->GetNumberPopTime();

	for (const FFeelNumberPop& Pop : CurrentSubsystem->GetNumberPops())
	{
		const APlayerController* PlayerController = Pop.PlayerController.Get();
		FVector2D ScreenPosition;
		if (!PlayerController || !PlayerController->ProjectWorldLocationToScreen(Pop.WorldLocation, ScreenPosition, true))
		{
			continue;
		}

		const float Age = FMath::Clamp(static_cast<float>((Now - Pop.StartTime) / FMath::Max(Pop.Lifetime, 0.01f)), 0.0f, 1.0f);

		// Pops in large, settles quickly, floats up while easing out, and fades over the last third.
		const float Settle = FMath::Clamp(Age / 0.15f, 0.0f, 1.0f);
		const float Scale = FMath::Lerp(Pop.PopScale, 1.0f, 1.0f - FMath::Square(1.0f - Settle));
		const float Rise = Pop.RiseDistance * (1.0f - FMath::Square(1.0f - Age));
		const float Opacity = Age < 0.66f ? 1.0f : 1.0f - (Age - 0.66f) / 0.34f;

		FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), FMath::Max(FMath::RoundToInt32(Pop.FontSize * Scale), 4));
		Font.OutlineSettings.OutlineSize = 2;
		Font.OutlineSettings.OutlineColor = FLinearColor(0.0f, 0.0f, 0.0f, Opacity);

		const FString String = Pop.Text.ToString();
		const FVector2D TextSize = FontMeasure->Measure(String, Font);
		const FVector2D Center = ScreenPosition / GeometryScale + Pop.ScreenOffset - FVector2D(0.0, Rise);
		const FVector2D TopLeft = Center - TextSize * 0.5;

		FLinearColor Color = Pop.Color;
		Color.A *= Opacity;
		FSlateDrawElement::MakeText(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TopLeft)), String, Font, ESlateDrawEffect::None, Color);
	}
	return LayerId + 1;
}
