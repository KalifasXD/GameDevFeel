// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate/WidgetTransform.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"

class UWidget;

/**
 * Applies widget contributions (render transform offsets and a color tint) and restores the original values once nothing
 * requests them. Each frame: BeginFrame, then Add calls, then EndFrame.
 * Colors apply to user widgets, images and text blocks (color and opacity) and borders (brush color); other widgets only
 * take the transform.
 */
class FEELCORE_API FFeelWidgetDelivery
{
public:
	void BeginFrame();

	/** Requests transform offsets (added) and a color tint (strongest weight wins) for a widget. */
	void AddWidget(UWidget* Widget, const FVector2D& Translation, const FVector2D& ScaleDelta, float AngleDegrees, const FLinearColor& Color, float ColorWeight);

	void EndFrame();
	void RestoreAll();

	bool IsModifyingAny() const { return Entries.Num() > 0; }

private:
	struct FEntry
	{
		TWeakObjectPtr<UWidget> Widget;
		FWidgetTransform BaseTransform;
		FLinearColor BaseColor = FLinearColor::White;
		bool bHasColor = false;
		FVector2D Translation = FVector2D::ZeroVector;
		FVector2D ScaleDelta = FVector2D::ZeroVector;
		float Angle = 0.0f;
		FLinearColor Color = FLinearColor::White;
		float ColorWeight = 0.0f;
		bool bRequested = false;
	};

	static bool ReadColor(const UWidget& Widget, FLinearColor& OutColor);
	static void WriteColor(UWidget& Widget, const FLinearColor& Color);
	static void Apply(UWidget& Widget, const FEntry& Entry, bool bRestore);

	TMap<FObjectKey, FEntry> Entries;
};
