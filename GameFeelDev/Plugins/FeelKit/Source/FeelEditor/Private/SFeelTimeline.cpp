// Copyright 2026 Billo. All Rights Reserved.

#include "SFeelTimeline.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
// FEELKIT_PRO_BEGIN
#include "FeelAudioAnalysis.h"
// FEELKIT_PRO_END
#include "FeelEditorColors.h"
// FEELKIT_PRO_BEGIN
#include "FeelPlayCapture.h"
// FEELKIT_PRO_END
#include "Steps/FeelStep_PlaySound.h"
#include "FeelEditorSettings.h"
#include "FeelEvaluator.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelStep.h"
#include "FeelTrack.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Modules/ModuleManager.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "SPositiveActionButton.h"
#include "Styling/StyleColors.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

namespace FeelTimeline
{
	constexpr float HeaderWidth = 220.0f;
	constexpr float RulerHeight = 28.0f;
	constexpr float RowHeight = 28.0f;
	constexpr float EdgeHandleWidth = 6.0f;
	constexpr float ToggleSize = 16.0f;
	constexpr float DragThreshold = 3.0f;
	constexpr float MinPixelsPerSecond = 20.0f;
	constexpr float MaxPixelsPerSecond = 5000.0f;
	constexpr float CurveLaneHeight = 84.0f;
	constexpr float CurvePadding = 10.0f;
	constexpr float KeySize = 8.0f;
	constexpr float KeyHitRadius = 7.0f;

	using FeelEditorColors::GetChannelColor;
	using FeelEditorColors::GetChannelAccent;

	/** Colors measured from Sequencer in UE 5.6, so the timeline sits in the editor the same way Sequencer does. */
	namespace Palette
	{
		inline FLinearColor Srgb(uint8 R, uint8 G, uint8 B, float Alpha = 1.0f) { return FLinearColor(FColor(R, G, B)).CopyWithNewOpacity(Alpha); }
		inline FLinearColor OutsideRange() { return Srgb(25, 25, 25); }
		inline FLinearColor InsideRange() { return Srgb(36, 36, 36); }
		inline FLinearColor GridLine() { return Srgb(25, 25, 25); }
		inline FLinearColor Ruler() { return Srgb(23, 23, 23); }
		inline FLinearColor RulerCorner() { return Srgb(15, 15, 15); }
		inline FLinearColor RulerText() { return Srgb(123, 123, 123); }
		inline FLinearColor OutlinerEmpty() { return Srgb(26, 26, 26); }
		inline FLinearColor OutlinerRow() { return Srgb(47, 47, 47); }
		inline FLinearColor OutlinerLane() { return Srgb(36, 36, 36); }
		inline FLinearColor Separator() { return Srgb(0, 0, 0); }
		inline FLinearColor Text() { return Srgb(192, 192, 192); }
		inline FLinearColor SubduedText() { return Srgb(123, 123, 123); }
		inline FLinearColor Playhead() { return Srgb(255, 64, 64); }
		inline FLinearColor RangeEnd() { return Srgb(128, 32, 32); }
		inline FLinearColor Selection() { return FAppStyle::GetSlateColor(TEXT("SelectionColor")).GetColor(FWidgetStyle()); }
	}

	FString GetTrackLabel(const FFeelTrack& Track)
	{
		return Track.Step ? Track.Step->GetClass()->GetDisplayNameText().ToString() : LOCTEXT("NoStep", "(No Step)").ToString();
	}

	/** Lists concrete step classes, including Blueprint steps that are not loaded yet. */
	class FStepClassFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass && InClass->IsChildOf(UFeelStep::StaticClass()) && !InClass->HasAnyClassFlags(GetDisallowedFlags());
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InUnloadedClassData->IsChildOf(UFeelStep::StaticClass()) && !InUnloadedClassData->HasAnyClassFlags(GetDisallowedFlags());
		}

	private:
		static EClassFlags GetDisallowedFlags()
		{
			return CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
		}
	};
}

/** Custom-painted track rows with ruler, playhead and direct manipulation. */
class SFeelTimelineTrackArea : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFeelTimelineTrackArea) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState)
	{
		State = InState;
		SetClipping(EWidgetClipping::ClipToBounds);
	}

	/** Fits the whole recipe into the visible width. */
	void ZoomToFit()
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		const float Duration = Recipe ? FMath::Max(Recipe->GetDuration(), 0.5f) : 1.0f;
		const float Width = FMath::Max(LastWidth - FeelTimeline::HeaderWidth - 20.0f, 100.0f);
		PixelsPerSecond = FMath::Clamp(Width / (Duration * 1.1f), FeelTimeline::MinPixelsPerSecond, FeelTimeline::MaxPixelsPerSecond);
		ViewStartTime = -0.05f * Duration;
	}

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
	{
		return FVector2D(400.0, GetContentHeight());
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
		// A recipe opens with all of it in view, and stays fitted while the panel is resized until the user zooms or scrolls.
		const float Width = AllottedGeometry.GetLocalSize().X;
		if (!bUserMovedView && Width > FeelTimeline::HeaderWidth + 120.0f && !FMath::IsNearlyEqual(Width, FittedWidth, 1.0f))
		{
			LastWidth = Width;
			ZoomToFit();
			FittedWidth = Width;
		}
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;

private:
	enum class EDragMode : uint8
	{
		None,
		Scrub,
		MoveTrack,
		ResizeStart,
		ResizeEnd,
		Pan,
		MoveKey,
	};

	enum class EHitZone : uint8
	{
		None,
		Ruler,
		Header,
		MuteToggle,
		SoloToggle,
		Body,
		StartEdge,
		EndEdge,
		EmptyRow,
		CurveLane,
		CurveKey,
	};

	struct FHit
	{
		EHitZone Zone = EHitZone::None;
		int32 Track = INDEX_NONE;
		TOptional<FKeyHandle> Key;
	};

	float TimeToX(float Time) const { return FeelTimeline::HeaderWidth + (Time - ViewStartTime) * PixelsPerSecond; }
	float XToTime(float X) const { return ViewStartTime + (X - FeelTimeline::HeaderWidth) / PixelsPerSecond; }

	/** The selected track shows its intensity curve in a lane under its row. */
	bool HasCurveLane(int32 TrackIndex) const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		return Recipe && TrackIndex == State->GetSelectedTrack() && Recipe->Tracks.IsValidIndex(TrackIndex) && Recipe->Tracks[TrackIndex].Duration > 0.0f;
	}

	float GetRowHeight(int32 TrackIndex) const
	{
		return FeelTimeline::RowHeight + (HasCurveLane(TrackIndex) ? FeelTimeline::CurveLaneHeight : 0.0f);
	}

	float RowTop(int32 TrackIndex) const
	{
		const int32 Selected = State->GetSelectedTrack();
		const float LaneAbove = Selected < TrackIndex && HasCurveLane(Selected) ? FeelTimeline::CurveLaneHeight : 0.0f;
		return FeelTimeline::RulerHeight + TrackIndex * FeelTimeline::RowHeight + LaneAbove - VerticalOffset;
	}

	float GetContentHeight() const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		const int32 NumTracks = Recipe ? Recipe->Tracks.Num() : 0;
		return FeelTimeline::RulerHeight + (NumTracks + GetNumReleaseRows()) * FeelTimeline::RowHeight + (HasCurveLane(State->GetSelectedTrack()) ? FeelTimeline::CurveLaneHeight : 0.0f);
	}

	/** Release recipes of a sustained recipe get a row each under the tracks: On Full Release, then On Early Release. */
	int32 GetNumReleaseRows() const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		if (!Recipe || !Recipe->bSustain)
		{
			return 0;
		}
		return (Recipe->FullReleaseRecipe ? 1 : 0) + (Recipe->EarlyReleaseRecipe ? 1 : 0);
	}

	/** Recipe shown in a release row, and whether the row is On Full Release. */
	const UFeelRecipe* GetReleaseRowRecipe(int32 Row, bool& bOutFullRelease) const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		bOutFullRelease = Row == 0 && Recipe && Recipe->FullReleaseRecipe;
		if (!Recipe || Row < 0 || Row >= GetNumReleaseRows())
		{
			return nullptr;
		}
		return bOutFullRelease ? Recipe->FullReleaseRecipe.Get() : Recipe->EarlyReleaseRecipe.Get();
	}

	float ReleaseRowTop(int32 Row) const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		const int32 NumTracks = Recipe ? Recipe->Tracks.Num() : 0;
		const float Lane = HasCurveLane(State->GetSelectedTrack()) ? FeelTimeline::CurveLaneHeight : 0.0f;
		return FeelTimeline::RulerHeight + (NumTracks + Row) * FeelTimeline::RowHeight + Lane - VerticalOffset;
	}

	/** Release row under LocalY, or INDEX_NONE. */
	int32 ReleaseRowAt(float LocalY) const
	{
		for (int32 Row = 0; Row < GetNumReleaseRows(); ++Row)
		{
			const float Top = ReleaseRowTop(Row);
			if (LocalY >= Top && LocalY < Top + FeelTimeline::RowHeight)
			{
				return Row;
			}
		}
		return INDEX_NONE;
	}

	int32 RowAt(float LocalY) const
	{
		const UFeelRecipe* Recipe = State->GetRecipe();
		for (int32 TrackIndex = 0; Recipe && TrackIndex < Recipe->Tracks.Num(); ++TrackIndex)
		{
			const float Top = RowTop(TrackIndex);
			if (LocalY >= Top && LocalY < Top + GetRowHeight(TrackIndex))
			{
				return TrackIndex;
			}
		}
		return INDEX_NONE;
	}

	/** Curve time 0 to 1 spans the track; value 0 to 1 spans the lane height. */
	FVector2f CurveToLocal(const FFeelTrack& Track, float Top, float Alpha, float Value) const
	{
		const float PlotTop = Top + FeelTimeline::RowHeight + FeelTimeline::CurvePadding;
		const float PlotHeight = FeelTimeline::CurveLaneHeight - 2.0f * FeelTimeline::CurvePadding;
		return FVector2f(TimeToX(Track.StartTime + Alpha * Track.Duration), PlotTop + (1.0f - Value) * PlotHeight);
	}

	/** Inverse of CurveToLocal. With snapping, the key's absolute time lands on a frame, like track edits. */
	void LocalToCurve(const FFeelTrack& Track, float Top, const FVector2f& Local, bool bSnap, float& OutAlpha, float& OutValue) const
	{
		const float PlotTop = Top + FeelTimeline::RowHeight + FeelTimeline::CurvePadding;
		const float PlotHeight = FeelTimeline::CurveLaneHeight - 2.0f * FeelTimeline::CurvePadding;
		float KeyTime = XToTime(Local.X);
		if (bSnap)
		{
			KeyTime = State->SnapTime(KeyTime);
		}
		OutAlpha = Track.Duration > 0.0f ? (KeyTime - Track.StartTime) / Track.Duration : 0.0f;
		OutValue = 1.0f - (Local.Y - PlotTop) / PlotHeight;
	}

	FSlateRect GetMuteRect(float Top) const
	{
		const float Left = 8.0f;
		const float ToggleTop = Top + (FeelTimeline::RowHeight - FeelTimeline::ToggleSize) * 0.5f;
		return FSlateRect(Left, ToggleTop, Left + FeelTimeline::ToggleSize, ToggleTop + FeelTimeline::ToggleSize);
	}

	FSlateRect GetSoloRect(float Top) const
	{
		const float Left = 8.0f + FeelTimeline::ToggleSize + 6.0f;
		const float ToggleTop = Top + (FeelTimeline::RowHeight - FeelTimeline::ToggleSize) * 0.5f;
		return FSlateRect(Left, ToggleTop, Left + FeelTimeline::ToggleSize, ToggleTop + FeelTimeline::ToggleSize);
	}

	float ClampVerticalOffset(float Offset) const
	{
		return FMath::Clamp(Offset, 0.0f, FMath::Max(0.0f, GetContentHeight() - LastHeight));
	}

	FHit HitTest(const FVector2f& Local) const;
	void UpdateTrackDrag(const FVector2f& Local, bool bDisableSnap);
	void UpdateKeyDrag(const FVector2f& Local, bool bDisableSnap);
	void EndDrag();
	void ShowContextMenu(const FPointerEvent& MouseEvent, int32 TrackIndex);
	void ShowKeyMenu(const FPointerEvent& MouseEvent, int32 TrackIndex, FKeyHandle Key);

	TSharedPtr<FFeelRecipeEditorState> State;

	float ViewStartTime = -0.05f;
	float PixelsPerSecond = 400.0f;
	float VerticalOffset = 0.0f;
	mutable float LastWidth = 800.0f;
	mutable float LastHeight = 200.0f;
	/** Width the view was last fitted to; the view refits on resize until the user zooms or scrolls. */
	float FittedWidth = 0.0f;
	bool bUserMovedView = false;

	EDragMode DragMode = EDragMode::None;
	bool bDragStarted = false;
	FVector2f DragStartPosition = FVector2f::ZeroVector;
	int32 DragTrack = INDEX_NONE;
	TOptional<FKeyHandle> DragKey;
	float DragOriginalStart = 0.0f;
	float DragOriginalDuration = 0.0f;
	float DragStartViewTime = 0.0f;
	float DragStartVerticalOffset = 0.0f;
	TUniquePtr<FScopedTransaction> DragTransaction;
};

SFeelTimelineTrackArea::FHit SFeelTimelineTrackArea::HitTest(const FVector2f& Local) const
{
	FHit Hit;
	const UFeelRecipe* Recipe = State->GetRecipe();
	if (!Recipe)
	{
		return Hit;
	}

	if (Local.Y < FeelTimeline::RulerHeight)
	{
		Hit.Zone = Local.X >= FeelTimeline::HeaderWidth ? EHitZone::Ruler : EHitZone::None;
		return Hit;
	}

	const int32 Row = RowAt(Local.Y);
	if (!Recipe->Tracks.IsValidIndex(Row))
	{
		return Hit;
	}

	Hit.Track = Row;
	const float Top = RowTop(Row);
	const FFeelTrack& Track = Recipe->Tracks[Row];

	if (Local.Y >= Top + FeelTimeline::RowHeight)
	{
		Hit.Zone = Local.X < FeelTimeline::HeaderWidth ? EHitZone::Header : EHitZone::CurveLane;
		const FRichCurve* Curve = Hit.Zone == EHitZone::CurveLane ? State->GetEditableCurve(Row) : nullptr;
		for (FKeyHandle Key = Curve ? Curve->GetFirstKeyHandle() : FKeyHandle::Invalid(); Curve && Curve->IsKeyHandleValid(Key); Key = Curve->GetNextKey(Key))
		{
			const FVector2f KeyPosition = CurveToLocal(Track, Top, Curve->GetKeyTime(Key), FMath::Clamp(Curve->GetKeyValue(Key), 0.0f, 1.0f));
			if (FVector2f::Distance(KeyPosition, Local) <= FeelTimeline::KeyHitRadius)
			{
				Hit.Zone = EHitZone::CurveKey;
				Hit.Key = Key;
				break;
			}
		}
		return Hit;
	}

	if (Local.X < FeelTimeline::HeaderWidth)
	{
		if (GetMuteRect(Top).ContainsPoint(Local))
		{
			Hit.Zone = EHitZone::MuteToggle;
		}
		else if (GetSoloRect(Top).ContainsPoint(Local))
		{
			Hit.Zone = EHitZone::SoloToggle;
		}
		else
		{
			Hit.Zone = EHitZone::Header;
		}
		return Hit;
	}

	const float StartX = TimeToX(Track.StartTime);
	Hit.Zone = EHitZone::EmptyRow;

	if (Track.Duration > 0.0f)
	{
		const float EndX = TimeToX(Track.GetEndTime());
		const float Edge = FMath::Min(FeelTimeline::EdgeHandleWidth, (EndX - StartX) * 0.25f);
		if (Local.X >= StartX - 2.0f && Local.X <= StartX + Edge)
		{
			Hit.Zone = EHitZone::StartEdge;
		}
		else if (Local.X >= EndX - Edge && Local.X <= EndX + 2.0f)
		{
			Hit.Zone = EHitZone::EndEdge;
		}
		else if (Local.X > StartX && Local.X < EndX)
		{
			Hit.Zone = EHitZone::Body;
		}
	}
	else if (FMath::Abs(Local.X - StartX) <= FeelTimeline::RowHeight * 0.35f)
	{
		Hit.Zone = EHitZone::Body;
	}
	return Hit;
}

int32 SFeelTimelineTrackArea::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	using namespace FeelTimeline;

	const FVector2f Size = AllottedGeometry.GetLocalSize();
	LastWidth = Size.X;
	LastHeight = Size.Y;

	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SmallFont = FAppStyle::GetFontStyle(TEXT("SmallFont"));
	const FSlateFontInfo NormalFont = FAppStyle::GetFontStyle(TEXT("NormalFont"));
	const ESlateDrawEffect Effect = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	auto DrawBox = [&](int32 Layer, float X, float Y, float W, float H, const FLinearColor& Color)
	{
		if (W > 0.0f && H > 0.0f)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(W, H), FSlateLayoutTransform(FVector2f(X, Y))), WhiteBrush, Effect, Color);
		}
	};

	auto DrawText = [&](int32 Layer, float X, float Y, const FString& Text, const FSlateFontInfo& Font, const FLinearColor& Color)
	{
		FSlateDrawElement::MakeText(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(FMath::Max(Size.X - X, 1.0f), RowHeight), FSlateLayoutTransform(FVector2f(X, Y))), Text, Font, Effect, Color);
	};

	auto DrawLine = [&](int32 Layer, const FVector2f& A, const FVector2f& B, const FLinearColor& Color, float Thickness)
	{
		TArray<FVector2f> Points;
		Points.Add(A);
		Points.Add(B);
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(), MoveTemp(Points), Effect, Color, true, Thickness);
	};

	const int32 BackgroundLayer = LayerId;
	const int32 RowLayer = LayerId + 1;
	const int32 BarLayer = LayerId + 2;
	const int32 BarDetailLayer = LayerId + 3;
	const int32 HeaderLayer = LayerId + 4;
	const int32 HeaderDetailLayer = LayerId + 5;
	const int32 RulerLayer = LayerId + 6;
	const int32 RulerDetailLayer = LayerId + 7;
	const int32 PlayheadLayer = LayerId + 8;

	DrawBox(BackgroundLayer, 0.0f, 0.0f, Size.X, Size.Y, Palette::OutsideRange());

	const UFeelRecipe* Recipe = State->GetRecipe();
	if (!Recipe)
	{
		return PlayheadLayer + 1;
	}

	const int32 SelectedTrack = State->GetSelectedTrack();
	const bool bAnySolo = Recipe->HasSoloTracks();

	// Like Sequencer, the part of the time line the recipe plays in is lighter than the time around it.
	{
		const float RangeStartX = FMath::Max(TimeToX(0.0f), HeaderWidth);
		const float RangeEndX = FMath::Min(TimeToX(State->GetPlaybackLength()), Size.X);
		DrawBox(BackgroundLayer, RangeStartX, RulerHeight, RangeEndX - RangeStartX, Size.Y - RulerHeight, Palette::InsideRange());
	}
	DrawBox(HeaderLayer, 0.0f, RulerHeight, HeaderWidth, Size.Y - RulerHeight, Palette::OutlinerEmpty());
	const FSlateBrush* SectionBrush = FAppStyle::GetBrush(TEXT("Sequencer.Section.Background_Collapsed"));
	const FSlateBrush* SelectedSectionBrush = FAppStyle::GetBrush(TEXT("Sequencer.Section.CollapsedSelectedSectionOverlay"));
	// Sequencer's mute column: an open eye while the track plays, a closed eye when it is muted. (The circle with a
	// slash, Sequencer.Column.Mute, is Sequencer's Deactivate column, not mute.)
	const FSlateBrush* PlayingBrush = FAppStyle::GetBrush(TEXT("Level.VisibleIcon16x"));
	const FSlateBrush* MutedBrush = FAppStyle::GetBrush(TEXT("Level.NotVisibleIcon16x"));
	const FSlateBrush* SoloBrush = FAppStyle::GetBrush(TEXT("Sequencer.Column.Solo"));
	auto DrawBrush = [&](int32 Layer, float X, float Y, float W, float H, const FSlateBrush* Brush, const FLinearColor& Color)
	{
		if (W > 0.0f && H > 0.0f)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(W, H), FSlateLayoutTransform(FVector2f(X, Y))), Brush, Effect, Color);
		}
	};

	for (int32 TrackIndex = 0; TrackIndex < Recipe->Tracks.Num(); ++TrackIndex)
	{
		const float Top = RowTop(TrackIndex);
		if (Top + GetRowHeight(TrackIndex) < RulerHeight || Top > Size.Y)
		{
			continue;
		}

		const FFeelTrack& Track = Recipe->Tracks[TrackIndex];
		const bool bSelected = TrackIndex == SelectedTrack;
		const bool bAudible = Track.bEnabled && (!bAnySolo || Track.bSolo);
		const FString Label = GetTrackLabel(Track);

		FLinearColor ChannelColor = GetChannelColor(Track.Channel);
		FLinearColor AccentColor = GetChannelAccent(Track.Channel);
		if (!bAudible)
		{
			// Muted tracks keep their shape but lose their color, as Sequencer shows inactive sections.
			ChannelColor = Palette::Srgb(71, 71, 71);
			AccentColor = Palette::Srgb(90, 90, 90);
		}

		// Section, drawn with Sequencer's section brush and selection overlay.
		const float StartX = TimeToX(Track.StartTime);
		const float BarTop = Top + 2.0f;
		const float BarHeight = RowHeight - 4.0f;
		if (Track.Duration > 0.0f)
		{
			const float EndX = TimeToX(Track.GetEndTime());
			const float BarWidth = EndX - StartX;
			DrawBrush(BarLayer, StartX, BarTop, BarWidth, BarHeight, SectionBrush, ChannelColor);

			if (bSelected)
			{
				DrawBrush(BarDetailLayer, StartX + 1.0f, BarTop + 1.0f, BarWidth - 2.0f, BarHeight - 2.0f, SelectedSectionBrush, Palette::Selection().CopyWithNewOpacity(0.8f));
			}

			// Intensity curve sparkline.
			if (BarWidth > 8.0f)
			{
				const int32 NumSamples = FMath::Clamp(FMath::FloorToInt32(BarWidth / 4.0f), 2, 64);
				TArray<FVector2f> Points;
				Points.Reserve(NumSamples);
				for (int32 Sample = 0; Sample < NumSamples; ++Sample)
				{
					const float Alpha = static_cast<float>(Sample) / static_cast<float>(NumSamples - 1);
					const float Value = FMath::Clamp(Track.EvaluateIntensityCurve(Alpha), 0.0f, 1.0f);
					Points.Add(FVector2f(StartX + Alpha * BarWidth, BarTop + BarHeight - 1.0f - Value * (BarHeight - 2.0f)));
				}
				FSlateDrawElement::MakeLines(OutDrawElements, BarDetailLayer, AllottedGeometry.ToPaintGeometry(), MoveTemp(Points), Effect, FLinearColor(1.0f, 1.0f, 1.0f, 0.3f), true, 1.0f);
			}


			// Random duration: show where the track ends in this preview play.
			if (Track.RandomDurationScale.Min != 1.0f || Track.RandomDurationScale.Max != 1.0f)
			{
				const float PlayEndX = TimeToX(Track.StartTime + FFeelEvaluator::GetTrackDuration(*Recipe, TrackIndex, State->GetPreviewParams()));
				if (PlayEndX > EndX)
				{
					DrawBox(BarLayer, EndX, BarTop + BarHeight * 0.35f, PlayEndX - EndX, BarHeight * 0.3f, ChannelColor.CopyWithNewOpacity(0.6f));
				}
				DrawBox(BarDetailLayer, PlayEndX - 1.0f, BarTop, 2.0f, BarHeight, Palette::Text());
			}

			// FEELKIT_PRO_BEGIN
			// Sound waveform, so other tracks can be lined up with what the player hears.
			if (const UFeelStep_PlaySound* SoundStep = Cast<UFeelStep_PlaySound>(Track.Step))
			{
				if (const FFeelAudioEnvelope* Envelope = FFeelAudioAnalysis::GetEnvelope(SoundStep->Sound))
				{
					const float MidY = BarTop + BarHeight * 0.5f;
					const float VisibleStart = FMath::Max(StartX, HeaderWidth);
					for (float X = VisibleStart; X < FMath::Min(EndX, Size.X); X += 2.0f)
					{
						const float SoundTime = SoundStep->SoundStartTime + XToTime(X) - Track.StartTime;
						const float Level = Envelope->SampleLoudness(SoundTime, 2.0f / PixelsPerSecond);
						const float HalfHeight = Level * (BarHeight * 0.5f - 1.0f);
						if (HalfHeight > 0.25f)
						{
							DrawBox(BarDetailLayer, X, MidY - HalfHeight, 1.0f, HalfHeight * 2.0f, FLinearColor(1.0f, 1.0f, 1.0f, 0.18f));
						}
					}
				}
				else if (SoundStep->Sound && BarWidth > 220.0f)
				{
					// Say why there is no waveform, instead of silently showing none.
					DrawText(BarDetailLayer, FMath::Max(StartX, HeaderWidth) + 90.0f, BarTop + 2.0f, LOCTEXT("NoWaveform", "No waveform: this sound's audio cannot be read").ToString(), SmallFont, Palette::SubduedText());
				}
			}
			// FEELKIT_PRO_END

			// Labels. One line on the left (channel, and No preview for steps the preview cannot simulate), and what this
			// preview play does with the track on the right. Both are clipped to the bar and never drawn over each other.
			const bool bNoPreview = Track.Step && !Track.Step->SupportsPreview();
			if (bNoPreview)
			{
				// Steps the preview cannot simulate are hatched.
				for (float X = StartX; X + BarHeight < EndX; X += 8.0f)
				{
					DrawLine(BarDetailLayer, FVector2f(X, BarTop + BarHeight), FVector2f(X + BarHeight, BarTop), FLinearColor(0.0f, 0.0f, 0.0f, 0.25f), 1.0f);
				}
			}

			const float LabelLeft = FMath::Max(StartX, HeaderWidth) + 5.0f;
			const float LabelRight = FMath::Min(EndX, Size.X) - 5.0f;
			if (LabelRight - LabelLeft > 12.0f)
			{
				const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
				OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry.GetLayoutBoundingRect(FSlateRect(FMath::Max(StartX, HeaderWidth), BarTop, FMath::Min(EndX, Size.X), BarTop + BarHeight))));

				bool bSilenced = false;
				const FText Decision = State->GetTrackDecision(TrackIndex, &bSilenced);
				const FString DecisionString = Decision.ToString();
				if (bSilenced && !DecisionString.IsEmpty())
				{
					// The track does not play: the bar is dimmed and says why.
					DrawBox(BarDetailLayer + 1, FMath::Max(StartX, HeaderWidth), BarTop, EndX - FMath::Max(StartX, HeaderWidth), BarHeight, Palette::OutsideRange().CopyWithNewOpacity(0.35f));
					DrawText(BarDetailLayer + 2, LabelLeft, BarTop + 3.0f, DecisionString, SmallFont, Palette::Text());
				}
				else
				{
					FString LeftLabel = Track.Channel.IsValid() ? Track.Channel.ToString() : FString();
					if (bNoPreview)
					{
						LeftLabel = LeftLabel.IsEmpty() ? LOCTEXT("NoPreview", "No preview").ToString() : FString::Printf(TEXT("%s  -  %s"), *LeftLabel, *LOCTEXT("NoPreview", "No preview").ToString());
					}
					const float LeftWidth = FontMeasure->Measure(LeftLabel, SmallFont).X;
					const float DecisionWidth = DecisionString.IsEmpty() ? 0.0f : FontMeasure->Measure(DecisionString, SmallFont).X;
					const float DecisionX = LabelRight - DecisionWidth;
					const bool bDecisionFitsBeside = DecisionString.IsEmpty() || DecisionX >= LabelLeft + LeftWidth + 12.0f;

					if (bDecisionFitsBeside)
					{
						DrawText(BarDetailLayer + 2, LabelLeft, BarTop + 3.0f, LeftLabel, SmallFont, Palette::Text().CopyWithNewOpacity(bAudible ? 0.95f : 0.5f));
					}
					if (!DecisionString.IsEmpty())
					{
						// When both do not fit, what happens in this play is the more useful of the two.
						const float TextX = bDecisionFitsBeside ? DecisionX : LabelLeft;
						DrawBox(BarDetailLayer + 1, TextX - 3.0f, BarTop + 2.0f, DecisionWidth + 6.0f, 14.0f, Palette::RulerCorner().CopyWithNewOpacity(0.8f));
						DrawText(BarDetailLayer + 2, TextX, BarTop + 3.0f, DecisionString, SmallFont, Palette::Text());
					}
				}

				OutDrawElements.PopClip();
			}
		}
		else
		{
			// Instant step marker.
			DrawBrush(BarLayer, StartX - 5.0f, Top + RowHeight * 0.5f - 5.0f, 10.0f, 10.0f, SectionBrush, bSelected ? Palette::Selection() : AccentColor);
		}

		// Header.
		// Outliner row, as in Sequencer: toggles on the left, the name, a strip of the track color on the right edge.
		DrawBox(HeaderLayer, 0.0f, Top, HeaderWidth, RowHeight - 1.0f, Palette::OutlinerRow());
		if (bSelected)
		{
			DrawBox(HeaderLayer, 0.0f, Top, HeaderWidth, RowHeight - 1.0f, Palette::Selection().CopyWithNewOpacity(0.25f));
		}
		DrawBox(HeaderLayer, 0.0f, Top + RowHeight - 1.0f, HeaderWidth, 1.0f, Palette::Separator());
		DrawBox(HeaderDetailLayer, HeaderWidth - 4.0f, Top, 4.0f, RowHeight - 1.0f, AccentColor);

		const FSlateRect MuteRect = GetMuteRect(Top);
		DrawBrush(HeaderDetailLayer, MuteRect.Left, MuteRect.Top, ToggleSize, ToggleSize, Track.bEnabled ? PlayingBrush : MutedBrush, Track.bEnabled ? Palette::Text().CopyWithNewOpacity(0.35f) : Palette::Text());
		const FSlateRect SoloRect = GetSoloRect(Top);
		DrawBrush(HeaderDetailLayer, SoloRect.Left, SoloRect.Top, ToggleSize, ToggleSize, SoloBrush, Track.bSolo ? Palette::Text() : Palette::Text().CopyWithNewOpacity(0.25f));

		DrawText(HeaderDetailLayer, SoloRect.Right + 10.0f, Top + 6.0f, Label, NormalFont, bAudible ? Palette::Text() : Palette::SubduedText());

		// Intensity curve lane of the selected track.
		if (HasCurveLane(TrackIndex))
		{
			const float LaneTop = Top + RowHeight;
			const float EndX = TimeToX(Track.GetEndTime());
			const FLinearColor HintColor = Palette::SubduedText();

			DrawBox(RowLayer, HeaderWidth, LaneTop, Size.X - HeaderWidth, CurveLaneHeight - 1.0f, Palette::OutsideRange());
			DrawBox(RowLayer, StartX, LaneTop, EndX - StartX, CurveLaneHeight - 1.0f, ChannelColor.CopyWithNewOpacity(0.18f));

			for (float GuideValue : { 0.0f, 0.5f, 1.0f })
			{
				const float GuideY = CurveToLocal(Track, Top, 0.0f, GuideValue).Y;
				DrawLine(BarLayer, FVector2f(StartX, GuideY), FVector2f(EndX, GuideY), FLinearColor(1.0f, 1.0f, 1.0f, GuideValue == 0.5f ? 0.06f : 0.15f), 1.0f);
			}

			const int32 NumCurveSamples = FMath::Clamp(FMath::FloorToInt32((EndX - StartX) / 3.0f), 2, 256);
			TArray<FVector2f> CurvePoints;
			CurvePoints.Reserve(NumCurveSamples);
			for (int32 Sample = 0; Sample < NumCurveSamples; ++Sample)
			{
				const float Alpha = static_cast<float>(Sample) / static_cast<float>(NumCurveSamples - 1);
				CurvePoints.Add(CurveToLocal(Track, Top, Alpha, FMath::Clamp(Track.EvaluateIntensityCurve(Alpha), 0.0f, 1.0f)));
			}
			FSlateDrawElement::MakeLines(OutDrawElements, BarLayer, AllottedGeometry.ToPaintGeometry(), MoveTemp(CurvePoints), Effect, AccentColor, true, 2.0f);

			const float HintX = FMath::Max(StartX, HeaderWidth) + 6.0f;
			if (const FRichCurve* Curve = State->GetEditableCurve(TrackIndex))
			{
				for (FKeyHandle Key = Curve->GetFirstKeyHandle(); Curve->IsKeyHandleValid(Key); Key = Curve->GetNextKey(Key))
				{
					const FVector2f KeyPosition = CurveToLocal(Track, Top, Curve->GetKeyTime(Key), FMath::Clamp(Curve->GetKeyValue(Key), 0.0f, 1.0f));
					const bool bDraggedKey = DragMode == EDragMode::MoveKey && DragTrack == TrackIndex && DragKey.IsSet() && DragKey.GetValue() == Key;
					const float HalfKey = KeySize * 0.5f;
					DrawBox(BarDetailLayer, KeyPosition.X - HalfKey - 1.0f, KeyPosition.Y - HalfKey - 1.0f, KeySize + 2.0f, KeySize + 2.0f, FLinearColor::Black);
					DrawBox(BarDetailLayer + 1, KeyPosition.X - HalfKey, KeyPosition.Y - HalfKey, KeySize, KeySize, bDraggedKey ? Palette::Selection() : Palette::Text());
				}
				if (Curve->GetNumKeys() == 0)
				{
					DrawText(BarDetailLayer, HintX, LaneTop + 4.0f, LOCTEXT("CurveEmptyHint", "No keys: full intensity. Double-click to add a key.").ToString(), SmallFont, HintColor);
				}
			}
			else
			{
				DrawText(BarDetailLayer, HintX, LaneTop + 4.0f, LOCTEXT("CurveAssetHint", "Uses a curve asset. Edit that asset, or clear it in Details to key the curve here.").ToString(), SmallFont, HintColor);
			}

			DrawBox(HeaderLayer, 0.0f, LaneTop, HeaderWidth, CurveLaneHeight - 1.0f, Palette::OutlinerLane());
			DrawBox(HeaderLayer, 0.0f, LaneTop + CurveLaneHeight - 1.0f, HeaderWidth, 1.0f, Palette::Separator());
			DrawBox(HeaderDetailLayer, HeaderWidth - 4.0f, LaneTop, 4.0f, CurveLaneHeight - 1.0f, AccentColor);
			DrawText(HeaderDetailLayer, 30.0f, LaneTop + 4.0f, LOCTEXT("CurveLaneLabel", "Intensity").ToString(), NormalFont, Palette::Text());
			DrawText(HeaderDetailLayer, 30.0f, LaneTop + 26.0f, LOCTEXT("CurveLaneHintAdd", "Double-click: add or delete key").ToString(), SmallFont, HintColor);
			DrawText(HeaderDetailLayer, 30.0f, LaneTop + 42.0f, LOCTEXT("CurveLaneHintDrag", "Drag key: move (Shift: no snap)").ToString(), SmallFont, HintColor);
			DrawText(HeaderDetailLayer, 30.0f, LaneTop + 58.0f, LOCTEXT("CurveLaneHintMenu", "Right-click key: interpolation").ToString(), SmallFont, HintColor);
		}
	}

	// Release recipes: a row each under the tracks, a block from Sustain End as long as the release recipe's tracks.
	for (int32 Row = 0; Row < GetNumReleaseRows(); ++Row)
	{
		const float Top = ReleaseRowTop(Row);
		bool bFullRelease = false;
		const UFeelRecipe* ReleaseRecipe = GetReleaseRowRecipe(Row, bFullRelease);
		if (!ReleaseRecipe || Top + RowHeight < RulerHeight || Top > Size.Y)
		{
			continue;
		}

		const FLinearColor ReleaseAccent(0.35f, 0.75f, 1.0f);
		const FLinearColor ReleaseColor = Palette::Srgb(52, 96, 122);
		bool bSilenced = false;
		const FString Decision = State->GetReleaseDecision(bFullRelease, &bSilenced).ToString();

		const float StartX = TimeToX(Recipe->SustainEnd);
		const float EndX = FMath::Max(TimeToX(Recipe->SustainEnd + ReleaseRecipe->GetTracksLength()), StartX + 6.0f);
		const float BarTop = Top + 2.0f;
		const float BarHeight = RowHeight - 4.0f;
		DrawBrush(BarLayer, StartX, BarTop, EndX - StartX, BarHeight, SectionBrush, bSilenced ? Palette::Srgb(71, 71, 71) : ReleaseColor);

		const float LabelLeft = FMath::Max(StartX, HeaderWidth) + 5.0f;
		const float LabelRight = FMath::Min(EndX, Size.X) - 5.0f;
		if (LabelRight - LabelLeft > 12.0f)
		{
			OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry.GetLayoutBoundingRect(FSlateRect(FMath::Max(StartX, HeaderWidth), BarTop, FMath::Min(EndX, Size.X), BarTop + BarHeight))));
			const FString Label = Decision.IsEmpty() ? ReleaseRecipe->GetName() : FString::Printf(TEXT("%s  -  %s"), *ReleaseRecipe->GetName(), *Decision);
			DrawText(BarDetailLayer + 2, LabelLeft, BarTop + 3.0f, Label, SmallFont, Palette::Text().CopyWithNewOpacity(bSilenced ? 0.6f : 0.95f));
			OutDrawElements.PopClip();
		}

		DrawBox(HeaderLayer, 0.0f, Top, HeaderWidth, RowHeight - 1.0f, Palette::OutlinerRow());
		DrawBox(HeaderLayer, 0.0f, Top + RowHeight - 1.0f, HeaderWidth, 1.0f, Palette::Separator());
		DrawBox(HeaderDetailLayer, HeaderWidth - 4.0f, Top, 4.0f, RowHeight - 1.0f, ReleaseAccent);
		const FText RowLabel = bFullRelease ? LOCTEXT("FullReleaseRow", "On Full Release") : LOCTEXT("EarlyReleaseRow", "On Early Release");
		DrawText(HeaderDetailLayer, GetSoloRect(Top).Right + 10.0f, Top + 6.0f, RowLabel.ToString(), NormalFont, Palette::SubduedText());
	}

	if (Recipe->Tracks.Num() == 0)
	{
		DrawText(BarDetailLayer, HeaderWidth + 12.0f, RulerHeight + GetNumReleaseRows() * RowHeight + 10.0f, LOCTEXT("EmptyHint", "Add a track with + Track.").ToString(), NormalFont, Palette::SubduedText());
	}

	// Ruler and grid.
	DrawBox(RulerLayer, 0.0f, 0.0f, Size.X, RulerHeight, Palette::Ruler());

	static const float TickSteps[] = { 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 30.0f, 60.0f };
	float MajorStep = 60.0f;
	for (float Step : TickSteps)
	{
		if (Step * PixelsPerSecond >= 70.0f)
		{
			MajorStep = Step;
			break;
		}
	}

	// Frame grid: shows where edits snap, once frames are far enough apart to see.
	const UFeelEditorSettings* Settings = GetDefault<UFeelEditorSettings>();
	if (Settings->bSnapToFrames && Settings->SnapFrameRate > 0)
	{
		const float FrameLength = 1.0f / static_cast<float>(Settings->SnapFrameRate);
		if (FrameLength * PixelsPerSecond >= 4.0f)
		{
			const int32 FirstFrame = FMath::FloorToInt32(FMath::Max(ViewStartTime, 0.0f) / FrameLength);
			for (int32 Frame = FirstFrame; Frame < FirstFrame + 4000; ++Frame)
			{
				const float X = TimeToX(Frame * FrameLength);
				if (X > Size.X)
				{
					break;
				}
				if (X < HeaderWidth)
				{
					continue;
				}
				DrawLine(RowLayer, FVector2f(X, RulerHeight), FVector2f(X, Size.Y), Palette::GridLine().CopyWithNewOpacity(0.35f), 1.0f);
				DrawLine(RulerDetailLayer, FVector2f(X, RulerHeight - 4.0f), FVector2f(X, RulerHeight), Palette::RulerText(), 1.0f);
			}
		}
	}

	const float FirstTick = FMath::FloorToFloat(ViewStartTime / MajorStep) * MajorStep;
	for (int32 TickIndex = 0; TickIndex < 1000; ++TickIndex)
	{
		const float TickTime = FirstTick + TickIndex * MajorStep;
		const float X = TimeToX(TickTime);
		if (X > Size.X)
		{
			break;
		}
		if (X < HeaderWidth)
		{
			continue;
		}
		DrawLine(RowLayer, FVector2f(X, RulerHeight), FVector2f(X, Size.Y), Palette::GridLine(), 1.0f);
		DrawLine(RulerDetailLayer, FVector2f(X, RulerHeight - 12.0f), FVector2f(X, RulerHeight), Palette::Text(), 1.0f);
		DrawText(RulerDetailLayer, X + 3.0f, RulerHeight - 16.0f, FString::Printf(TEXT("%.2fs"), TickTime), SmallFont, Palette::RulerText());
	}

	// Corner above the outliner; the + Track button sits on top of it.
	DrawBox(RulerDetailLayer, 0.0f, 0.0f, HeaderWidth, RulerHeight, Palette::RulerCorner());

	// Sustain region: shaded in the ruler and marked across the tracks.
	if (Recipe->bSustain && Recipe->SustainEnd > Recipe->SustainStart)
	{
		const float SustainStartX = FMath::Max(TimeToX(Recipe->SustainStart), HeaderWidth);
		const float SustainEndX = TimeToX(Recipe->SustainEnd);
		const FLinearColor SustainColor(0.35f, 0.75f, 1.0f);
		if (SustainEndX > SustainStartX)
		{
			DrawBox(RulerDetailLayer, SustainStartX, RulerHeight - 6.0f, SustainEndX - SustainStartX, 6.0f, FLinearColor(SustainColor.R, SustainColor.G, SustainColor.B, 0.6f));
			DrawBox(RowLayer, SustainStartX, RulerHeight, SustainEndX - SustainStartX, Size.Y - RulerHeight, FLinearColor(SustainColor.R, SustainColor.G, SustainColor.B, 0.04f));
			DrawText(RulerDetailLayer + 1, SustainStartX + 3.0f, RulerHeight - 20.0f, LOCTEXT("SustainLabel", "sustain").ToString(), SmallFont, SustainColor);
		}
	}

	// Recipe end.
	const float EndMarkerX = TimeToX(State->GetPlaybackLength());
	if (EndMarkerX >= HeaderWidth)
	{
		DrawLine(BarDetailLayer, FVector2f(EndMarkerX, 0.0f), FVector2f(EndMarkerX, Size.Y), Palette::RangeEnd(), 1.0f);
	}

	// Playhead.
	const float PlayheadX = TimeToX(State->GetTime());
	if (PlayheadX >= HeaderWidth)
	{
		// Sequencer's scrub handle: a red marker in the ruler and a thin white line through the tracks.
		DrawLine(PlayheadLayer, FVector2f(PlayheadX, RulerHeight), FVector2f(PlayheadX, Size.Y), Palette::Text(), 1.0f);
		DrawBox(PlayheadLayer, PlayheadX - 6.0f, RulerHeight - 22.0f, 12.0f, 18.0f, Palette::Playhead());
		DrawLine(PlayheadLayer, FVector2f(PlayheadX, RulerHeight - 4.0f), FVector2f(PlayheadX, RulerHeight), Palette::Playhead(), 2.0f);
	}

	// Timing readout while dragging, in seconds and frames.
	if (bDragStarted && Recipe->Tracks.IsValidIndex(DragTrack))
	{
		const FFeelTrack& Track = Recipe->Tracks[DragTrack];
		const float FrameRate = static_cast<float>(FMath::Max(Settings->SnapFrameRate, 1));
		FString Readout = FString::Printf(TEXT("Start %.3f s (frame %d)   Length %.3f s (%d frames)"),
			Track.StartTime, FMath::RoundToInt32(Track.StartTime * FrameRate),
			Track.Duration, FMath::RoundToInt32(Track.Duration * FrameRate));

		const FRichCurve* DragCurve = DragMode == EDragMode::MoveKey ? State->GetEditableCurve(DragTrack) : nullptr;
		if (DragCurve && DragKey.IsSet() && DragCurve->IsKeyHandleValid(DragKey.GetValue()))
		{
			const float KeyTime = Track.StartTime + DragCurve->GetKeyTime(DragKey.GetValue()) * Track.Duration;
			Readout = FString::Printf(TEXT("Key at %.3f s (frame %d)   Intensity %.2f"),
				KeyTime, FMath::RoundToInt32(KeyTime * FrameRate), DragCurve->GetKeyValue(DragKey.GetValue()));
		}
		const float ReadoutX = FMath::Max(TimeToX(Track.StartTime), HeaderWidth) + 4.0f;
		const float ReadoutY = FMath::Max(RowTop(DragTrack) - 18.0f, RulerHeight);
		DrawBox(PlayheadLayer, ReadoutX - 3.0f, ReadoutY, 310.0f, 16.0f, Palette::RulerCorner().CopyWithNewOpacity(0.9f));
		DrawText(PlayheadLayer + 1, ReadoutX, ReadoutY + 1.0f, Readout, SmallFont, Palette::Text());
	}

	return PlayheadLayer + 2;
}

FReply SFeelTimelineTrackArea::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2f Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FHit Hit = HitTest(Local);
	const FKey Button = MouseEvent.GetEffectingButton();

	if (Button == EKeys::MiddleMouseButton)
	{
		DragMode = EDragMode::Pan;
		DragStartPosition = Local;
		DragStartViewTime = ViewStartTime;
		DragStartVerticalOffset = VerticalOffset;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	if (Button == EKeys::RightMouseButton)
	{
		if (Hit.Zone == EHitZone::CurveKey && Hit.Key.IsSet())
		{
			ShowKeyMenu(MouseEvent, Hit.Track, Hit.Key.GetValue());
			return FReply::Handled();
		}
		if (Hit.Track != INDEX_NONE)
		{
			State->SetSelectedTrack(Hit.Track);
		}
		ShowContextMenu(MouseEvent, Hit.Track);
		return FReply::Handled();
	}

	if (Button != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	FReply Reply = FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::Mouse);

	switch (Hit.Zone)
	{
	case EHitZone::Ruler:
		DragMode = EDragMode::Scrub;
		State->SetTime(XToTime(Local.X));
		return Reply.CaptureMouse(SharedThis(this));

	case EHitZone::MuteToggle:
		State->ToggleMute(Hit.Track);
		return Reply;

	case EHitZone::SoloToggle:
		State->ToggleSolo(Hit.Track);
		return Reply;

	case EHitZone::CurveKey:
		DragMode = EDragMode::MoveKey;
		DragTrack = Hit.Track;
		DragKey = Hit.Key;
		DragStartPosition = Local;
		bDragStarted = false;
		return Reply.CaptureMouse(SharedThis(this));

	case EHitZone::CurveLane:
		return Reply;

	case EHitZone::Header:
	case EHitZone::EmptyRow:
		State->SetSelectedTrack(Hit.Track);
		return Reply;

	case EHitZone::Body:
	case EHitZone::StartEdge:
	case EHitZone::EndEdge:
	{
		State->SetSelectedTrack(Hit.Track);
		const FFeelTrack& Track = State->GetRecipe()->Tracks[Hit.Track];
		DragMode = Hit.Zone == EHitZone::Body ? EDragMode::MoveTrack : (Hit.Zone == EHitZone::StartEdge ? EDragMode::ResizeStart : EDragMode::ResizeEnd);
		DragTrack = Hit.Track;
		DragOriginalStart = Track.StartTime;
		DragOriginalDuration = Track.Duration;
		DragStartPosition = Local;
		bDragStarted = false;
		return Reply.CaptureMouse(SharedThis(this));
	}

	case EHitZone::None:
	default:
		State->SetSelectedTrack(INDEX_NONE);
		return Reply;
	}
}

FReply SFeelTimelineTrackArea::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (DragMode != EDragMode::None)
	{
		EndDrag();
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SFeelTimelineTrackArea::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (DragMode == EDragMode::None || !HasMouseCapture())
	{
		return FReply::Unhandled();
	}

	const FVector2f Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	switch (DragMode)
	{
	case EDragMode::Scrub:
		State->SetTime(XToTime(Local.X));
		break;

	case EDragMode::Pan:
		bUserMovedView = true;
		ViewStartTime = FMath::Max(DragStartViewTime - (Local.X - DragStartPosition.X) / PixelsPerSecond, -1.0f);
		VerticalOffset = ClampVerticalOffset(DragStartVerticalOffset - (Local.Y - DragStartPosition.Y));
		break;

	case EDragMode::MoveKey:
		UpdateKeyDrag(Local, MouseEvent.IsShiftDown());
		break;

	default:
		UpdateTrackDrag(Local, MouseEvent.IsShiftDown());
		break;
	}
	return FReply::Handled();
}

void SFeelTimelineTrackArea::UpdateTrackDrag(const FVector2f& Local, bool bDisableSnap)
{
	UFeelRecipe* Recipe = State->GetRecipe();
	if (!Recipe || !Recipe->Tracks.IsValidIndex(DragTrack))
	{
		return;
	}

	if (!bDragStarted)
	{
		if (FVector2f::Distance(Local, DragStartPosition) < FeelTimeline::DragThreshold)
		{
			return;
		}
		bDragStarted = true;
		DragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("EditTrackTiming", "Edit Feel Track Timing"));
		Recipe->Modify();
	}

	const UFeelEditorSettings* Settings = GetDefault<UFeelEditorSettings>();
	const float FrameLength = Settings->SnapFrameRate > 0 ? 1.0f / static_cast<float>(Settings->SnapFrameRate) : 0.01f;
	const float MinDuration = FMath::Max(FrameLength, 0.001f);
	const float Delta = (Local.X - DragStartPosition.X) / PixelsPerSecond;
	const float OriginalEnd = DragOriginalStart + DragOriginalDuration;

	// Hits in other tracks' sounds attract edges within a few pixels, ahead of frame snapping.
	TArray<float, TInlineAllocator<32>> SoundOnsets;
	// FEELKIT_PRO_BEGIN
	if (!bDisableSnap)
	{
		for (int32 TrackIndex = 0; TrackIndex < Recipe->Tracks.Num(); ++TrackIndex)
		{
			const FFeelTrack& SoundTrack = Recipe->Tracks[TrackIndex];
			const UFeelStep_PlaySound* SoundStep = TrackIndex != DragTrack ? Cast<UFeelStep_PlaySound>(SoundTrack.Step) : nullptr;
			const FFeelAudioEnvelope* Envelope = SoundStep ? FFeelAudioAnalysis::GetEnvelope(SoundStep->Sound) : nullptr;
			if (Envelope)
			{
				for (float Onset : Envelope->Onsets)
				{
					if (Onset >= SoundStep->SoundStartTime)
					{
						SoundOnsets.Add(SoundTrack.StartTime + Onset - SoundStep->SoundStartTime);
					}
				}
			}
		}
	}
	// FEELKIT_PRO_END

	auto Snap = [this, bDisableSnap, &SoundOnsets](float Time)
	{
		if (bDisableSnap)
		{
			return FMath::Max(Time, 0.0f);
		}
		const float MaxDistance = 8.0f / PixelsPerSecond;
		float BestDistance = MaxDistance;
		TOptional<float> BestOnset;
		for (float Onset : SoundOnsets)
		{
			if (FMath::Abs(Onset - Time) <= BestDistance)
			{
				BestDistance = FMath::Abs(Onset - Time);
				BestOnset = Onset;
			}
		}
		return BestOnset.IsSet() ? FMath::Max(BestOnset.GetValue(), 0.0f) : State->SnapTime(Time);
	};

	FFeelTrack& Track = Recipe->Tracks[DragTrack];
	switch (DragMode)
	{
	case EDragMode::MoveTrack:
		Track.StartTime = Snap(DragOriginalStart + Delta);
		break;

	case EDragMode::ResizeStart:
		Track.StartTime = FMath::Max(FMath::Min(Snap(DragOriginalStart + Delta), OriginalEnd - MinDuration), 0.0f);
		Track.Duration = OriginalEnd - Track.StartTime;
		break;

	case EDragMode::ResizeEnd:
		Track.Duration = FMath::Max(Snap(OriginalEnd + Delta), DragOriginalStart + MinDuration) - DragOriginalStart;
		break;

	default:
		break;
	}
}

void SFeelTimelineTrackArea::UpdateKeyDrag(const FVector2f& Local, bool bDisableSnap)
{
	UFeelRecipe* Recipe = State->GetRecipe();
	const FRichCurve* Curve = State->GetEditableCurve(DragTrack);
	if (!Recipe || !Curve || !DragKey.IsSet() || !Curve->IsKeyHandleValid(DragKey.GetValue()))
	{
		return;
	}

	if (!bDragStarted)
	{
		if (FVector2f::Distance(Local, DragStartPosition) < FeelTimeline::DragThreshold)
		{
			return;
		}
		bDragStarted = true;
		DragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("MoveCurveKey", "Move Intensity Key"));
		Recipe->Modify();
	}

	float Alpha = 0.0f;
	float Value = 0.0f;
	LocalToCurve(Recipe->Tracks[DragTrack], RowTop(DragTrack), Local, !bDisableSnap, Alpha, Value);
	State->MoveCurveKey(DragTrack, DragKey.GetValue(), Alpha, Value);
}

FReply SFeelTimelineTrackArea::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FVector2f Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const FHit Hit = HitTest(Local);

		if (Hit.Zone == EHitZone::CurveKey && Hit.Key.IsSet())
		{
			EndDrag();
			State->DeleteCurveKey(Hit.Track, Hit.Key.GetValue());
			return FReply::Handled();
		}

		if (Hit.Zone == EHitZone::CurveLane)
		{
			float Alpha = 0.0f;
			float Value = 0.0f;
			LocalToCurve(State->GetRecipe()->Tracks[Hit.Track], RowTop(Hit.Track), Local, !MouseEvent.IsShiftDown(), Alpha, Value);
			State->AddCurveKey(Hit.Track, Alpha, Value);
			return FReply::Handled();
		}
	}

	// A release row opens its recipe, as double-clicking a Play Recipe step's asset would.
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FVector2f Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		bool bFullRelease = false;
		const UFeelRecipe* ReleaseRecipe = Local.Y >= FeelTimeline::RulerHeight ? GetReleaseRowRecipe(ReleaseRowAt(Local.Y), bFullRelease) : nullptr;
		if (ReleaseRecipe && GEditor)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(const_cast<UFeelRecipe*>(ReleaseRecipe));
			return FReply::Handled();
		}
	}

	// Elsewhere a fast second click behaves like a normal click.
	return OnMouseButtonDown(MyGeometry, MouseEvent);
}

void SFeelTimelineTrackArea::ShowKeyMenu(const FPointerEvent& MouseEvent, int32 TrackIndex, FKeyHandle Key)
{
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	FMenuBuilder MenuBuilder(true, nullptr);

	auto AddInterpEntry = [&MenuBuilder, WeakState, TrackIndex, Key](const FText& Label, const FText& Tooltip, ERichCurveInterpMode InterpMode)
	{
		MenuBuilder.AddMenuEntry(Label, Tooltip, FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([WeakState, TrackIndex, Key, InterpMode]()
				{
					if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
					{
						PinnedState->SetCurveKeyInterpMode(TrackIndex, Key, InterpMode);
					}
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([WeakState, TrackIndex, Key, InterpMode]()
				{
					const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
					const FRichCurve* Curve = PinnedState.IsValid() ? PinnedState->GetEditableCurve(TrackIndex) : nullptr;
					return Curve && Curve->IsKeyHandleValid(Key) && Curve->GetKeyInterpMode(Key) == InterpMode;
				})),
			NAME_None,
			EUserInterfaceActionType::RadioButton);
	};

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("KeyInterpSection", "Interpolation to next key"));
	AddInterpEntry(LOCTEXT("KeyLinear", "Linear"), LOCTEXT("KeyLinearTip", "Straight line to the next key."), RCIM_Linear);
	AddInterpEntry(LOCTEXT("KeySmooth", "Smooth"), LOCTEXT("KeySmoothTip", "Eased curve to the next key."), RCIM_Cubic);
	AddInterpEntry(LOCTEXT("KeyConstant", "Constant"), LOCTEXT("KeyConstantTip", "Hold this value until the next key."), RCIM_Constant);
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("KeyEditSection", "Key"));
	MenuBuilder.AddMenuEntry(LOCTEXT("DeleteKeyMenu", "Delete Key"), LOCTEXT("DeleteKeyTip", "Delete this key (or double-click it)."), FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([WeakState, TrackIndex, Key]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->DeleteCurveKey(TrackIndex, Key);
			}
		})));
	MenuBuilder.EndSection();

	const FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

void SFeelTimelineTrackArea::EndDrag()
{
	DragMode = EDragMode::None;
	bDragStarted = false;
	DragTrack = INDEX_NONE;
	DragKey.Reset();
	DragTransaction.Reset();
}

void SFeelTimelineTrackArea::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	EndDrag();
}

FReply SFeelTimelineTrackArea::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2f Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float WheelDelta = MouseEvent.GetWheelDelta();

	if (MouseEvent.IsControlDown() || MouseEvent.IsShiftDown())
	{
		bUserMovedView = true;
	}
	if (MouseEvent.IsControlDown())
	{
		// Zoom around the cursor.
		const float AnchorX = FMath::Max(Local.X, FeelTimeline::HeaderWidth);
		const float TimeAtAnchor = XToTime(AnchorX);
		PixelsPerSecond = FMath::Clamp(PixelsPerSecond * FMath::Pow(1.2f, WheelDelta), FeelTimeline::MinPixelsPerSecond, FeelTimeline::MaxPixelsPerSecond);
		ViewStartTime = TimeAtAnchor - (AnchorX - FeelTimeline::HeaderWidth) / PixelsPerSecond;
	}
	else if (MouseEvent.IsShiftDown())
	{
		ViewStartTime -= WheelDelta * 60.0f / PixelsPerSecond;
	}
	else
	{
		VerticalOffset = ClampVerticalOffset(VerticalOffset - WheelDelta * FeelTimeline::RowHeight);
	}

	ViewStartTime = FMath::Max(ViewStartTime, -1.0f);
	return FReply::Handled();
}

FReply SFeelTimelineTrackArea::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const int32 SelectedTrack = State->GetSelectedTrack();

	if (Key == EKeys::Delete && SelectedTrack != INDEX_NONE)
	{
		State->DeleteTrack(SelectedTrack);
		return FReply::Handled();
	}
	if (Key == EKeys::D && InKeyEvent.IsControlDown() && SelectedTrack != INDEX_NONE)
	{
		State->DuplicateTrack(SelectedTrack);
		return FReply::Handled();
	}
	if (Key == EKeys::C && InKeyEvent.IsControlDown() && SelectedTrack != INDEX_NONE)
	{
		State->CopySelectedTrack();
		return FReply::Handled();
	}
	if (Key == EKeys::V && InKeyEvent.IsControlDown())
	{
		State->PasteTracks();
		return FReply::Handled();
	}
	if (Key == EKeys::SpaceBar)
	{
		State->TogglePlay();
		return FReply::Handled();
	}
	if (Key == EKeys::F)
	{
		ZoomToFit();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FCursorReply SFeelTimelineTrackArea::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	switch (DragMode)
	{
	case EDragMode::ResizeStart:
	case EDragMode::ResizeEnd:
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	case EDragMode::MoveTrack:
	case EDragMode::MoveKey:
	case EDragMode::Pan:
		return FCursorReply::Cursor(EMouseCursor::GrabHandClosed);
	default:
		break;
	}

	const FHit Hit = HitTest(MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition()));
	switch (Hit.Zone)
	{
	case EHitZone::StartEdge:
	case EHitZone::EndEdge:
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	case EHitZone::Body:
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	case EHitZone::CurveKey:
		return FCursorReply::Cursor(EMouseCursor::CardinalCross);
	case EHitZone::MuteToggle:
	case EHitZone::SoloToggle:
		return FCursorReply::Cursor(EMouseCursor::Hand);
	default:
		return FCursorReply::Unhandled();
	}
}

void SFeelTimelineTrackArea::ShowContextMenu(const FPointerEvent& MouseEvent, int32 TrackIndex)
{
	const UFeelRecipe* Recipe = State->GetRecipe();
	if (!Recipe)
	{
		return;
	}

	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;

	// Copy and paste, also available on empty space.
	auto AddClipboardEntries = [WeakState](FMenuBuilder& Builder, bool bWithTrack)
	{
		if (bWithTrack)
		{
			Builder.AddMenuEntry(LOCTEXT("CopyTrackMenu", "Copy"), LOCTEXT("CopyTrackTip", "Copy this track (Ctrl+C) to paste into any recipe."), FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([WeakState]()
				{
					if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
					{
						PinnedState->CopySelectedTrack();
					}
				})));
		}

		Builder.AddMenuEntry(LOCTEXT("CopyAllTracksMenu", "Copy All Tracks"), LOCTEXT("CopyAllTracksTip", "Copy every track of this recipe."), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([WeakState]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->CopyAllTracks();
				}
			})));

		Builder.AddMenuEntry(LOCTEXT("PasteTracksMenu", "Paste"), LOCTEXT("PasteTracksTip", "Paste copied tracks at the playhead (Ctrl+V), keeping their relative timing."), FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([WeakState]()
				{
					if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
					{
						PinnedState->PasteTracks();
					}
				}),
				FCanExecuteAction::CreateLambda([WeakState]()
				{
					const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
					return PinnedState.IsValid() && PinnedState->CanPaste();
				})));
	};

	if (!Recipe->Tracks.IsValidIndex(TrackIndex))
	{
		FMenuBuilder EmptySpaceMenu(true, nullptr);
		AddClipboardEntries(EmptySpaceMenu, false);
		const FWidgetPath EmptySpacePath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
		FSlateApplication::Get().PushMenu(AsShared(), EmptySpacePath, EmptySpaceMenu.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
		return;
	}

	const FFeelTrack& Track = Recipe->Tracks[TrackIndex];

	auto MakeAction = [WeakState, TrackIndex](void (FFeelRecipeEditorState::*Function)(int32))
	{
		return FUIAction(FExecuteAction::CreateLambda([WeakState, TrackIndex, Function]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				(PinnedState.Get()->*Function)(TrackIndex);
			}
		}));
	};

	FMenuBuilder MenuBuilder(true, nullptr);
	AddClipboardEntries(MenuBuilder, true);
	MenuBuilder.AddMenuSeparator();
	MenuBuilder.AddMenuEntry(LOCTEXT("DuplicateTrackMenu", "Duplicate"), LOCTEXT("DuplicateTrackTip", "Duplicate this track (Ctrl+D)."), FSlateIcon(), MakeAction(&FFeelRecipeEditorState::DuplicateTrack));
	MenuBuilder.AddMenuEntry(LOCTEXT("DeleteTrackMenu", "Delete"), LOCTEXT("DeleteTrackTip", "Delete this track (Delete)."), FSlateIcon(), MakeAction(&FFeelRecipeEditorState::DeleteTrack));
	MenuBuilder.AddMenuSeparator();
	MenuBuilder.AddMenuEntry(
		FUIAction(FExecuteAction::CreateLambda([WeakState, TrackIndex]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->MoveTrack(TrackIndex, -1);
			}
		})),
		SNew(STextBlock).Text(LOCTEXT("MoveUpMenu", "Move Up")));
	MenuBuilder.AddMenuEntry(
		FUIAction(FExecuteAction::CreateLambda([WeakState, TrackIndex]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->MoveTrack(TrackIndex, 1);
			}
		})),
		SNew(STextBlock).Text(LOCTEXT("MoveDownMenu", "Move Down")));
	MenuBuilder.AddMenuSeparator();
	MenuBuilder.AddMenuEntry(Track.bEnabled ? LOCTEXT("MuteMenu", "Mute") : LOCTEXT("UnmuteMenu", "Unmute"), LOCTEXT("MuteTip", "Toggle whether this track plays."), FSlateIcon(), MakeAction(&FFeelRecipeEditorState::ToggleMute));
	MenuBuilder.AddMenuEntry(Track.bSolo ? LOCTEXT("UnsoloMenu", "Unsolo") : LOCTEXT("SoloMenu", "Solo"), LOCTEXT("SoloTip", "Preview only soloed tracks."), FSlateIcon(), MakeAction(&FFeelRecipeEditorState::ToggleSolo));

	// FEELKIT_PRO_BEGIN
	// Tracks that follow a sound: its loudness over time becomes their intensity curve.
	if (const UFeelStep_PlaySound* SoundStep = Cast<UFeelStep_PlaySound>(Track.Step))
	{
		const bool bHasAudio = FFeelAudioAnalysis::GetEnvelope(SoundStep->Sound) != nullptr;
		auto AddFromSound = [&MenuBuilder, WeakState, TrackIndex, bHasAudio](const FText& Label, const FText& Tooltip, bool bForceFeedback)
		{
			MenuBuilder.AddMenuEntry(Label, bHasAudio ? Tooltip : LOCTEXT("FromSoundNoAudioTip", "Needs a Sound Wave, or a Sound Cue that plays one, with imported audio. MetaSounds and procedural sounds cannot be read."), FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateLambda([WeakState, TrackIndex, bForceFeedback]()
					{
						if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
						{
							PinnedState->CreateTrackFromSound(TrackIndex, bForceFeedback);
						}
					}),
					FCanExecuteAction::CreateLambda([bHasAudio]() { return bHasAudio; })));
		};
		MenuBuilder.AddMenuSeparator();
		AddFromSound(LOCTEXT("ForceFeedbackFromSound", "Create Force Feedback Track From Sound"), LOCTEXT("ForceFeedbackFromSoundTip", "Adds a Force Feedback Curve track below this one whose rumble follows the sound's loudness."), true);
		AddFromSound(LOCTEXT("ShakeFromSound", "Create Shake Track From Sound"), LOCTEXT("ShakeFromSoundTip", "Adds a Procedural Shake track below this one whose strength follows the sound's loudness."), false);
	}
	// FEELKIT_PRO_END

	const FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

void SFeelTimeline::Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState)
{
	State = InState;
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			MakeToolbar()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f, 0.0f, 4.0f, 4.0f)
		[
			SAssignNew(ParameterSliderBox, SBox)
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(TrackArea, SFeelTimelineTrackArea, InState)
			]
			// Where Sequencer puts + Add: above the track names, in the corner next to the ruler.
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(4.0f, 2.0f)
			[
				SNew(SBox)
				.HeightOverride(FeelTimeline::RulerHeight - 4.0f)
				[
					SNew(SPositiveActionButton)
					.Text(LOCTEXT("AddTrackButton", "Track"))
					.ToolTipText(LOCTEXT("AddTrackTip", "Add a track with the chosen step, including Blueprint steps."))
					.Icon(FAppStyle::GetBrush(TEXT("Icons.Plus")))
					.OnGetMenuContent(this, &SFeelTimeline::MakeAddTrackMenu)
					.IsEnabled_Lambda([WeakState]()
					{
						const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
						return PinnedState.IsValid() && !PinnedState->IsReadOnly();
					})
				]
			]
		]
	];

	RefreshParameterSliders();
}

TSharedRef<SWidget> SFeelTimeline::MakeToolbar()
{
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;

	FSlimHorizontalToolBarBuilder Toolbar(nullptr, FMultiBoxCustomization::None);
	Toolbar.SetStyle(&FAppStyle::Get(), TEXT("SequencerToolBar"));
	Toolbar.SetLabelVisibility(EVisibility::Visible);

	Toolbar.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([WeakState]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->TogglePlay();
			}
		})),
		NAME_None,
		TAttribute<FText>::CreateLambda([WeakState]()
		{
			const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
			return PinnedState.IsValid() && PinnedState->IsPlaying() ? LOCTEXT("PauseButton", "Pause") : LOCTEXT("PlayButton", "Play");
		}),
		LOCTEXT("PlayTip", "Play or pause the preview (Space in the timeline)."),
		TAttribute<FSlateIcon>::CreateLambda([WeakState]()
		{
			const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
			return FSlateIcon(FAppStyle::GetAppStyleSetName(), PinnedState.IsValid() && PinnedState->IsPlaying() ? TEXT("Animation.Pause") : TEXT("Animation.Forward"));
		}));

	Toolbar.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([WeakState]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->Stop();
			}
		})),
		NAME_None,
		LOCTEXT("StopButton", "Stop"),
		LOCTEXT("StopTip", "Stop and return to the start."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Animation.Stop")));

	Toolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateLambda([WeakState]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->ReleaseSustain();
				}
			}),
			FCanExecuteAction::CreateLambda([WeakState]()
			{
				const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
				return PinnedState.IsValid() && PinnedState->CanReleaseSustain();
			})),
		NAME_None,
		LOCTEXT("ReleaseButton", "Release"),
		LOCTEXT("ReleaseTip", "End the sustain loop, as Release Feel does in game: the preview plays the rest of the recipe and On Early Release, if set. Enabled while a recipe with sustain is looping."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Animation.Forward_End")),
		EUserInterfaceActionType::Button,
		NAME_None,
		TAttribute<EVisibility>::CreateLambda([WeakState]()
		{
			const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
			const UFeelRecipe* Recipe = PinnedState.IsValid() ? PinnedState->GetRecipe() : nullptr;
			return Recipe && Recipe->bSustain ? EVisibility::Visible : EVisibility::Collapsed;
		}));

	Toolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateLambda([WeakState]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->SetLooping(!PinnedState->IsLooping());
				}
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([WeakState]()
			{
				const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
				return PinnedState.IsValid() && PinnedState->IsLooping();
			})),
		NAME_None,
		LOCTEXT("LoopLabel", "Loop"),
		LOCTEXT("LoopTip", "Loop the preview."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Animation.Loop.Enabled")),
		EUserInterfaceActionType::ToggleButton);

	Toolbar.AddSeparator();

	Toolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateLambda([]()
			{
				UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
				Settings->bSnapToFrames = !Settings->bSnapToFrames;
				Settings->SaveConfig();
			}),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([]() { return GetDefault<UFeelEditorSettings>()->bSnapToFrames; })),
		NAME_None,
		LOCTEXT("SnapLabel", "Snap"),
		LOCTEXT("SnapTip", "Snap track edits to frames. Hold Shift while dragging to ignore snapping."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Snap")),
		EUserInterfaceActionType::ToggleButton);

	Toolbar.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateSP(this, &SFeelTimeline::MakeFrameRateMenu),
		TAttribute<FText>::CreateSP(this, &SFeelTimeline::GetFrameRateText),
		LOCTEXT("FpsTip", "Frame rate used for snapping."),
		FSlateIcon());

	Toolbar.AddSeparator();

	Toolbar.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateSP(this, &SFeelTimeline::MakePreviewComfortMenu),
		TAttribute<FText>::CreateSP(this, &SFeelTimeline::GetPreviewComfortText),
		LOCTEXT("PreviewComfortTip", "Comfort settings applied to the preview, to check how the recipe feels for players who reduce motion or flashes."),
		FSlateIcon());

	// FEELKIT_PRO_BEGIN
	Toolbar.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateSP(this, &SFeelTimeline::MakeRecentPlaysMenu),
		TAttribute<FText>::CreateSP(this, &SFeelTimeline::GetRecentPlaysText),
		LOCTEXT("RecentPlaysTip", "Replay a play of this recipe recorded during Play In Editor, with the same random rolls, intensity, parameters and comfort, so the preview shows exactly what the player saw. Changes to the recipe apply to the replay."),
		FSlateIcon());
	// FEELKIT_PRO_END

	Toolbar.AddSeparator();

	Toolbar.AddToolBarButton(
		FUIAction(
			FExecuteAction::CreateLambda([WeakState]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->PlayInPIE();
				}
			}),
			FCanExecuteAction::CreateLambda([WeakState]()
			{
				const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
				return PinnedState.IsValid() && PinnedState->CanPlayInPIE();
			})),
		NAME_None,
		LOCTEXT("PlayInPIEButton", "Play in PIE"),
		LOCTEXT("PlayInPIETip", "Play this recipe in the running Play-In-Editor session, on the player pawn (its skeletal mesh when it has one). Start PIE first."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("PlayWorld.PlayInViewport")));

	// FEELKIT_PRO_BEGIN
	Toolbar.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([WeakState]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->OnCaptureGifRequested.Broadcast();
			}
		})),
		NAME_None,
		LOCTEXT("CaptureGifButton", "Capture GIF"),
		LOCTEXT("CaptureGifTip", "Record the preview twice, without and with the recipe, and save both side by side as an animated GIF in Saved/FeelKit/Captures. Useful for sharing a before and after."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("LevelViewport.HighResScreenshot")));
	// FEELKIT_PRO_END

	Toolbar.AddSeparator();

	Toolbar.AddToolBarButton(
		FUIAction(FExecuteAction::CreateLambda([this]()
		{
			if (TrackArea.IsValid())
			{
				TrackArea->ZoomToFit();
			}
		})),
		NAME_None,
		LOCTEXT("FitButton", "Fit"),
		LOCTEXT("FitTip", "Fit the whole recipe in view (F in the timeline). Ctrl+Wheel zooms, Shift+Wheel or middle-drag scrolls."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("GenericCurveEditor.ZoomToFit")));

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			Toolbar.MakeWidget()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(8.0f, 0.0f, 10.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text(this, &SFeelTimeline::GetTimeText)
			.ColorAndOpacity(FeelTimeline::Palette::Text())
		];
}

TSharedRef<SWidget> SFeelTimeline::MakeFrameRateMenu()
{
	FMenuBuilder Menu(true, nullptr);
	Menu.BeginSection(NAME_None, LOCTEXT("FrameRateSection", "Snap Frame Rate"));
	for (const int32 Rate : { 24, 25, 30, 48, 50, 60, 120 })
	{
		Menu.AddMenuEntry(
			FText::Format(LOCTEXT("FrameRateEntry", "{0} fps"), FText::AsNumber(Rate)),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([Rate]()
				{
					UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
					Settings->SnapFrameRate = Rate;
					Settings->SaveConfig();
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([Rate]() { return GetDefault<UFeelEditorSettings>()->SnapFrameRate == Rate; })),
			NAME_None,
			EUserInterfaceActionType::RadioButton);
	}
	Menu.AddWidget(
		SNew(SBox)
		.WidthOverride(70.0f)
		[
			SNew(SSpinBox<int32>)
			.MinValue(1)
			.MaxValue(240)
			.Value_Lambda([]() { return GetDefault<UFeelEditorSettings>()->SnapFrameRate; })
			.OnValueCommitted_Lambda([](int32 NewValue, ETextCommit::Type CommitType)
			{
				UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
				Settings->SnapFrameRate = NewValue;
				Settings->SaveConfig();
			})
		],
		LOCTEXT("CustomFrameRate", "Custom"));
	Menu.EndSection();
	return Menu.MakeWidget();
}

FText SFeelTimeline::GetFrameRateText() const
{
	return FText::Format(LOCTEXT("FrameRateLabel", "{0} fps"), FText::AsNumber(GetDefault<UFeelEditorSettings>()->SnapFrameRate));
}

void SFeelTimeline::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	RefreshParameterSliders();
}

void SFeelTimeline::RefreshParameterSliders()
{
	const UFeelRecipe* Recipe = State->GetRecipe();

	FString Signature;
	if (Recipe)
	{
		for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
		{
			Signature += FString::Printf(TEXT("%s|%g|%g;"), *Parameter.Name.ToString(), Parameter.MinValue, Parameter.MaxValue);
		}
	}
	if (bParameterSlidersBuilt && Signature == ParameterSliderSignature)
	{
		return;
	}
	bParameterSlidersBuilt = true;
	ParameterSliderSignature = Signature;

	if (!Recipe || Recipe->Parameters.Num() == 0)
	{
		ParameterSliderBox->SetContent(SNullWidget::NullWidget);
		ParameterSliderBox->SetVisibility(EVisibility::Collapsed);
		return;
	}

	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	TSharedRef<SWrapBox> Sliders = SNew(SWrapBox).UseAllottedSize(true);

	Sliders->AddSlot()
	.Padding(0.0f, 2.0f, 10.0f, 2.0f)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("PreviewParametersLabel", "Preview parameters:"))
		.ToolTipText(LOCTEXT("PreviewParametersTip", "Values the preview plays with, as if a game passed them. They are not saved in the recipe."))
	];

	for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
	{
		if (Parameter.Name.IsNone() || Parameter.MaxValue <= Parameter.MinValue)
		{
			continue;
		}

		const FName ParameterName = Parameter.Name;
		const FText Tooltip = Parameter.Description.IsEmpty()
			? FText::Format(LOCTEXT("ParameterSliderTip", "{0}: {1} to {2}, default {3}."), FText::FromName(ParameterName), FText::AsNumber(Parameter.MinValue), FText::AsNumber(Parameter.MaxValue), FText::AsNumber(Parameter.DefaultValue))
			: Parameter.Description;

		Sliders->AddSlot()
		.Padding(0.0f, 2.0f, 12.0f, 2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			.ToolTipText(Tooltip)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(STextBlock).Text(FText::FromName(ParameterName))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				[
					SNew(SSpinBox<float>)
					.MinValue(Parameter.MinValue)
					.MaxValue(Parameter.MaxValue)
					.MinSliderValue(Parameter.MinValue)
					.MaxSliderValue(Parameter.MaxValue)
					.Value_Lambda([WeakState, ParameterName]()
					{
						const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
						return PinnedState.IsValid() ? PinnedState->GetPreviewParameterValue(ParameterName) : 0.0f;
					})
					.OnValueChanged_Lambda([WeakState, ParameterName](float NewValue)
					{
						if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
						{
							PinnedState->SetPreviewParameterValue(ParameterName, NewValue);
						}
					})
				]
			]
		];
	}

	Sliders->AddSlot()
	.Padding(0.0f, 2.0f)
	.VAlign(VAlign_Center)
	[
		SNew(SButton)
		.Text(LOCTEXT("ResetParametersButton", "Defaults"))
		.ToolTipText(LOCTEXT("ResetParametersTip", "Put every preview parameter back to its default value."))
		.OnClicked_Lambda([WeakState]()
		{
			if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
			{
				PinnedState->ResetPreviewParameters();
			}
			return FReply::Handled();
		})
	];

	ParameterSliderBox->SetContent(Sliders);
	ParameterSliderBox->SetVisibility(EVisibility::Visible);
}

TSharedRef<SWidget> SFeelTimeline::MakeAddTrackMenu()
{
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	Options.ClassFilters.Add(MakeShared<FeelTimeline::FStepClassFilter>());

	FClassViewerModule& ClassViewer = FModuleManager::LoadModuleChecked<FClassViewerModule>(TEXT("ClassViewer"));

	return SNew(SBox)
		.WidthOverride(280.0f)
		.HeightOverride(360.0f)
		[
			ClassViewer.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SFeelTimeline::OnStepClassPicked))
		];
}

void SFeelTimeline::OnStepClassPicked(UClass* StepClass)
{
	FSlateApplication::Get().DismissAllMenus();
	State->AddTrack(StepClass);
}

FText SFeelTimeline::GetTimeText() const
{
	return FText::FromString(FString::Printf(TEXT("%.2f / %.2f s"), State->GetTime(), State->GetPlaybackLength()));
}

TSharedRef<SWidget> SFeelTimeline::MakePreviewComfortMenu()
{
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	FMenuBuilder MenuBuilder(true, nullptr);

	auto AddPreset = [&MenuBuilder, WeakState](const FText& Label, const FText& Tooltip, TOptional<EFeelBuiltInComfortPreset> Preset)
	{
		MenuBuilder.AddMenuEntry(Label, Tooltip, FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([WeakState, Preset]()
				{
					if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
					{
						PinnedState->SetPreviewComfortPreset(Preset);
					}
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([WeakState, Preset]()
				{
					const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
					return PinnedState.IsValid() && PinnedState->GetPreviewComfortPreset() == Preset;
				})),
			NAME_None,
			EUserInterfaceActionType::RadioButton);
	};

	AddPreset(LOCTEXT("ComfortNeutral", "Neutral"), LOCTEXT("ComfortNeutralTip", "No comfort scaling: every effect at full strength."), TOptional<EFeelBuiltInComfortPreset>());
	AddPreset(LOCTEXT("ComfortProjectDefaults", "Project Defaults"), LOCTEXT("ComfortProjectDefaultsTip", "The comfort scales new players start with."), EFeelBuiltInComfortPreset::Default);
	AddPreset(LOCTEXT("ComfortReducedMotion", "Reduced Motion"), LOCTEXT("ComfortReducedMotionTip", "The Reduced Motion preset from the project settings."), EFeelBuiltInComfortPreset::ReducedMotion);
	AddPreset(LOCTEXT("ComfortReducedFlashing", "Reduced Flashing"), LOCTEXT("ComfortReducedFlashingTip", "The Reduced Flashing preset from the project settings."), EFeelBuiltInComfortPreset::ReducedFlashing);
	AddPreset(LOCTEXT("ComfortNoHaptics", "No Haptics"), LOCTEXT("ComfortNoHapticsTip", "The No Haptics preset from the project settings."), EFeelBuiltInComfortPreset::NoHaptics);

	return MenuBuilder.MakeWidget();
}

FText SFeelTimeline::GetPreviewComfortText() const
{
	FText PresetName = LOCTEXT("ComfortNeutralShort", "Neutral");
	const TOptional<EFeelBuiltInComfortPreset> Preset = State->GetPreviewComfortPreset();
	if (Preset.IsSet())
	{
		switch (Preset.GetValue())
		{
		case EFeelBuiltInComfortPreset::ReducedMotion:
			PresetName = LOCTEXT("ComfortReducedMotionShort", "Reduced Motion");
			break;
		case EFeelBuiltInComfortPreset::ReducedFlashing:
			PresetName = LOCTEXT("ComfortReducedFlashingShort", "Reduced Flashing");
			break;
		case EFeelBuiltInComfortPreset::NoHaptics:
			PresetName = LOCTEXT("ComfortNoHapticsShort", "No Haptics");
			break;
		case EFeelBuiltInComfortPreset::Default:
		default:
			PresetName = LOCTEXT("ComfortProjectDefaultsShort", "Project Defaults");
			break;
		}
	}
	return FText::Format(LOCTEXT("PreviewComfortLabel", "Comfort: {0}"), PresetName);
}

// FEELKIT_PRO_BEGIN
TSharedRef<SWidget> SFeelTimeline::MakeRecentPlaysMenu()
{
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(LOCTEXT("BackToPreview", "Back to Normal Preview"), LOCTEXT("BackToPreviewTip", "Stop replaying and use the preview's own settings again."), FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([WeakState]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->ClearCapture();
				}
			}),
			FCanExecuteAction::CreateLambda([WeakState]()
			{
				const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
				return PinnedState.IsValid() && PinnedState->GetActiveCapture() != nullptr;
			})));
	MenuBuilder.AddMenuSeparator();

	const UFeelRecipe* Recipe = State->GetRecipe();
	const TArray<FFeelPlayCapture>& Captures = FFeelPlayCaptureStore::Get().GetCaptures();
	int32 NumListed = 0;
	for (int32 CaptureIndex = Captures.Num() - 1; CaptureIndex >= 0; --CaptureIndex)
	{
		const FFeelPlayCapture& Capture = Captures[CaptureIndex];
		if (Capture.Recipe.Get() != Recipe)
		{
			continue;
		}

		FString Label = FString::Printf(TEXT("%s ago  -  intensity %.2f  -  %s"),
			*FText::AsTimespan(FDateTime::Now() - Capture.EndedAt).ToString(), Capture.Intensity, *Capture.TargetName);
		if (Capture.bInterrupted)
		{
			Label += TEXT("  (stopped early)");
		}

		FFeelPlayCapture CaptureCopy = Capture;
		MenuBuilder.AddMenuEntry(FText::FromString(Label), LOCTEXT("ReplayCaptureTip", "Replay this play in the preview. It starts playing right away."), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([WeakState, CaptureCopy]()
			{
				if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
				{
					PinnedState->LoadCapture(CaptureCopy);
					PinnedState->TogglePlay();
				}
			})));
		++NumListed;
	}

	if (NumListed == 0)
	{
		MenuBuilder.AddWidget(SNew(STextBlock).Text(LOCTEXT("NoRecentPlays", "No recorded plays yet. Play this recipe in Play In Editor, then open this menu again.")), FText::GetEmpty());
	}
	return MenuBuilder.MakeWidget();
}

FText SFeelTimeline::GetRecentPlaysText() const
{
	return State->GetActiveCapture() ? LOCTEXT("ReplayingCapture", "Replaying a Recorded Play") : LOCTEXT("RecentPlays", "Recent Plays");
}
// FEELKIT_PRO_END

#undef LOCTEXT_NAMESPACE
