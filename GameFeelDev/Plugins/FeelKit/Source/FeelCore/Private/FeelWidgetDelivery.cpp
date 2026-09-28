// Copyright 2026 Billo. All Rights Reserved.

#include "FeelWidgetDelivery.h"

#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void FFeelWidgetDelivery::BeginFrame()
{
	for (TPair<FObjectKey, FEntry>& Pair : Entries)
	{
		Pair.Value.Translation = FVector2D::ZeroVector;
		Pair.Value.ScaleDelta = FVector2D::ZeroVector;
		Pair.Value.Angle = 0.0f;
		Pair.Value.ColorWeight = 0.0f;
		Pair.Value.bRequested = false;
	}
}

void FFeelWidgetDelivery::AddWidget(UWidget* Widget, const FVector2D& Translation, const FVector2D& ScaleDelta, float AngleDegrees, const FLinearColor& Color, float ColorWeight)
{
	const bool bHasTransform = !Translation.IsZero() || !ScaleDelta.IsZero() || !FMath::IsNearlyZero(AngleDegrees);
	if (!Widget || (!bHasTransform && ColorWeight <= 0.0f))
	{
		return;
	}

	const FObjectKey Key(Widget);
	FEntry* Entry = Entries.Find(Key);
	if (!Entry)
	{
		Entry = &Entries.Add(Key);
		Entry->Widget = Widget;
		Entry->BaseTransform = Widget->GetRenderTransform();
		Entry->bHasColor = ReadColor(*Widget, Entry->BaseColor);
	}
	Entry->Translation += Translation;
	Entry->ScaleDelta += ScaleDelta;
	Entry->Angle += AngleDegrees;
	const float ClampedWeight = FMath::Clamp(ColorWeight, 0.0f, 1.0f);
	if (ClampedWeight > Entry->ColorWeight)
	{
		Entry->ColorWeight = ClampedWeight;
		Entry->Color = Color;
	}
	Entry->bRequested = true;
}

void FFeelWidgetDelivery::EndFrame()
{
	for (auto It = Entries.CreateIterator(); It; ++It)
	{
		UWidget* Widget = It.Value().Widget.Get();
		if (!Widget)
		{
			It.RemoveCurrent();
			continue;
		}

		const bool bRestore = !It.Value().bRequested;
		Apply(*Widget, It.Value(), bRestore);
		if (bRestore)
		{
			It.RemoveCurrent();
		}
	}
}

void FFeelWidgetDelivery::RestoreAll()
{
	for (const TPair<FObjectKey, FEntry>& Pair : Entries)
	{
		if (UWidget* Widget = Pair.Value.Widget.Get())
		{
			Apply(*Widget, Pair.Value, true);
		}
	}
	Entries.Reset();
}

bool FFeelWidgetDelivery::ReadColor(const UWidget& Widget, FLinearColor& OutColor)
{
	if (const UUserWidget* UserWidget = Cast<UUserWidget>(&Widget))
	{
		OutColor = UserWidget->GetColorAndOpacity();
		return true;
	}
	if (const UImage* Image = Cast<UImage>(&Widget))
	{
		OutColor = Image->GetColorAndOpacity();
		return true;
	}
	if (const UTextBlock* Text = Cast<UTextBlock>(&Widget))
	{
		OutColor = Text->GetColorAndOpacity().GetSpecifiedColor();
		return true;
	}
	if (const UBorder* Border = Cast<UBorder>(&Widget))
	{
		OutColor = Border->GetBrushColor();
		return true;
	}
	return false;
}

void FFeelWidgetDelivery::WriteColor(UWidget& Widget, const FLinearColor& Color)
{
	if (UUserWidget* UserWidget = Cast<UUserWidget>(&Widget))
	{
		UserWidget->SetColorAndOpacity(Color);
	}
	else if (UImage* Image = Cast<UImage>(&Widget))
	{
		Image->SetColorAndOpacity(Color);
	}
	else if (UTextBlock* Text = Cast<UTextBlock>(&Widget))
	{
		Text->SetColorAndOpacity(FSlateColor(Color));
	}
	else if (UBorder* Border = Cast<UBorder>(&Widget))
	{
		Border->SetBrushColor(Color);
	}
}

void FFeelWidgetDelivery::Apply(UWidget& Widget, const FEntry& Entry, bool bRestore)
{
	FWidgetTransform Transform = Entry.BaseTransform;
	if (!bRestore)
	{
		Transform.Translation += Entry.Translation;
		Transform.Scale = Transform.Scale * (FVector2D(1.0, 1.0) + Entry.ScaleDelta);
		Transform.Angle += Entry.Angle;
	}
	Widget.SetRenderTransform(Transform);

	if (Entry.bHasColor)
	{
		const float Weight = bRestore ? 0.0f : Entry.ColorWeight;
		WriteColor(Widget, FMath::Lerp(Entry.BaseColor, Entry.Color, Weight));
	}
}
