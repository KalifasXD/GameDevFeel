// Copyright 2026 Billo. All Rights Reserved.

#include "FeelCurveCustomization.h"

#include "Curves/CurveFloat.h"
#include "DetailWidgetRow.h"
#include "Framework/Application/SlateApplication.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Rendering/DrawElements.h"
#include "SCurveEditor.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SWindow.h"

#define LOCTEXT_NAMESPACE "FeelCurveCustomization"

namespace FeelCurveCustomizationPrivate
{
	/** Read-only drawing of a curve over its key range. */
	class SCurvePreview : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SCurvePreview) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, TFunction<const FRuntimeFloatCurve*()> InGetCurve)
		{
			GetCurve = MoveTemp(InGetCurve);
		}

		virtual FVector2D ComputeDesiredSize(float) const override
		{
			return FVector2D(140.0, 28.0);
		}

		virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
		{
			const FVector2f Size = AllottedGeometry.GetLocalSize();
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), FAppStyle::GetBrush(TEXT("WhiteBrush")), ESlateDrawEffect::None, FLinearColor(0.02f, 0.02f, 0.02f));

			const FRuntimeFloatCurve* Curve = GetCurve ? GetCurve() : nullptr;
			const FRichCurve* Rich = Curve ? Curve->GetRichCurveConst() : nullptr;
			if (!Rich)
			{
				return LayerId + 1;
			}

			float MinTime = 0.0f;
			float MaxTime = 1.0f;
			if (Rich->GetNumKeys() > 1)
			{
				Rich->GetTimeRange(MinTime, MaxTime);
			}
			if (MaxTime <= MinTime)
			{
				MaxTime = MinTime + 1.0f;
			}

			constexpr int32 NumSamples = 48;
			TArray<float> Values;
			float MinValue = 0.0f;
			float MaxValue = 1.0f;
			for (int32 Sample = 0; Sample < NumSamples; ++Sample)
			{
				const float Time = FMath::Lerp(MinTime, MaxTime, static_cast<float>(Sample) / (NumSamples - 1));
				const float Value = Rich->GetNumKeys() > 0 ? Rich->Eval(Time) : Time;
				Values.Add(Value);
				MinValue = FMath::Min(MinValue, Value);
				MaxValue = FMath::Max(MaxValue, Value);
			}

			TArray<FVector2f> Points;
			for (int32 Sample = 0; Sample < NumSamples; ++Sample)
			{
				const float X = Size.X * Sample / (NumSamples - 1);
				const float Y = Size.Y - 2.0f - (Values[Sample] - MinValue) / FMath::Max(MaxValue - MinValue, UE_KINDA_SMALL_NUMBER) * (Size.Y - 4.0f);
				Points.Add(FVector2f(X, Y));
			}
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), MoveTemp(Points), ESlateDrawEffect::None, FLinearColor(1.0f, 0.55f, 0.15f), true, 1.5f);
			return LayerId + 2;
		}

	private:
		TFunction<const FRuntimeFloatCurve*()> GetCurve;
	};
}

TSharedRef<IPropertyTypeCustomization> FFeelCurveCustomization::MakeInstance()
{
	return MakeShared<FFeelCurveCustomization>();
}

FFeelCurveCustomization::~FFeelCurveCustomization()
{
	CloseCurveWindow();
}

void FFeelCurveCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructHandle = InStructPropertyHandle;

	TArray<void*> RawData;
	StructHandle->AccessRawData(RawData);
	RuntimeCurve = RawData.Num() == 1 ? static_cast<FRuntimeFloatCurve*>(RawData[0]) : nullptr;

	TArray<UObject*> Outers;
	StructHandle->GetOuterObjects(Outers);
	Owner = Outers.Num() == 1 ? Outers[0] : nullptr;

	const TWeakPtr<FFeelCurveCustomization> WeakSelf = StaticCastSharedRef<FFeelCurveCustomization>(AsShared());
	HeaderRow
	.NameContent()
	[
		StructHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(220.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(FeelCurveCustomizationPrivate::SCurvePreview, [WeakSelf]() -> const FRuntimeFloatCurve*
			{
				const TSharedPtr<FFeelCurveCustomization> Pinned = WeakSelf.Pin();
				return Pinned.IsValid() ? Pinned->RuntimeCurve : nullptr;
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("EditCurve", "Edit..."))
			.ToolTipText(LOCTEXT("EditCurveTip", "Open the curve in a curve editor window. Track intensity curves can also be edited directly on the timeline, in the lane under the selected track."))
			.IsEnabled_Lambda([WeakSelf]()
			{
				const TSharedPtr<FFeelCurveCustomization> Pinned = WeakSelf.Pin();
				return Pinned.IsValid() && Pinned->RuntimeCurve && !Pinned->RuntimeCurve->ExternalCurve;
			})
			.OnClicked_Lambda([WeakSelf]()
			{
				const TSharedPtr<FFeelCurveCustomization> Pinned = WeakSelf.Pin();
				return Pinned.IsValid() ? Pinned->OpenCurveWindow() : FReply::Handled();
			})
		]
	];
}

void FFeelCurveCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// A curve asset can replace the curve stored here.
	if (TSharedPtr<IPropertyHandle> External = InStructPropertyHandle->GetChildHandle(TEXT("ExternalCurve")))
	{
		ChildBuilder.AddProperty(External.ToSharedRef());
	}
}

FReply FFeelCurveCustomization::OpenCurveWindow()
{
	if (!RuntimeCurve)
	{
		return FReply::Handled();
	}
	if (const TSharedPtr<SWindow> Existing = CurveWindow.Pin())
	{
		Existing->BringToFront();
		return FReply::Handled();
	}

	const FText Title = StructHandle.IsValid() ? StructHandle->GetPropertyDisplayName() : LOCTEXT("CurveWindowTitle", "Curve");
	float MinTime = 0.0f;
	float MaxTime = 1.0f;
	if (RuntimeCurve->GetRichCurveConst()->GetNumKeys() > 1)
	{
		RuntimeCurve->GetRichCurveConst()->GetTimeRange(MinTime, MaxTime);
	}
	MaxTime = FMath::Max(MaxTime, MinTime + 1.0f);

	const TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(Title)
		.ClientSize(FVector2D(800.0, 450.0))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			[
				SAssignNew(CurveEditor, SCurveEditor)
				.ViewMinInput(MinTime)
				.ViewMaxInput(MaxTime)
				.TimelineLength(MaxTime - MinTime)
				.HideUI(false)
				.ShowCurveSelector(false)
				.DesiredSize(FVector2D(780.0, 420.0))
			]
		];
	CurveEditor->SetCurveOwner(this);
	const TWeakPtr<FFeelCurveCustomization> WeakSelf = StaticCastSharedRef<FFeelCurveCustomization>(AsShared());
	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([WeakSelf](const TSharedRef<SWindow>&)
	{
		const TSharedPtr<FFeelCurveCustomization> Pinned = WeakSelf.Pin();
		if (Pinned.IsValid() && Pinned->CurveEditor.IsValid())
		{
			Pinned->CurveEditor->SetCurveOwner(nullptr, false);
			Pinned->CurveEditor.Reset();
		}
	}));

	const TSharedPtr<SWindow> Parent = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (Parent.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window, Parent.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window);
	}
	CurveWindow = Window;
	return FReply::Handled();
}

void FFeelCurveCustomization::CloseCurveWindow()
{
	if (CurveEditor.IsValid())
	{
		CurveEditor->SetCurveOwner(nullptr, false);
		CurveEditor.Reset();
	}
	if (const TSharedPtr<SWindow> Window = CurveWindow.Pin())
	{
		Window->SetOnWindowClosed(FOnWindowClosed());
		Window->RequestDestroyWindow();
	}
	CurveWindow.Reset();
}

TArray<FRichCurveEditInfoConst> FFeelCurveCustomization::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	if (RuntimeCurve)
	{
		Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->EditorCurveData));
	}
	return Curves;
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
void FFeelCurveCustomization::GetCurves(TAdderReserverRef<FRichCurveEditInfoConst> Curves) const
{
	if (RuntimeCurve)
	{
		Curves.Add(FRichCurveEditInfoConst(&RuntimeCurve->EditorCurveData));
	}
}
#endif

TArray<FRichCurveEditInfo> FFeelCurveCustomization::GetCurves()
{
	TArray<FRichCurveEditInfo> Curves;
	if (RuntimeCurve)
	{
		Curves.Add(FRichCurveEditInfo(&RuntimeCurve->EditorCurveData));
	}
	return Curves;
}

void FFeelCurveCustomization::ModifyOwner()
{
	if (UObject* Object = Owner.Get())
	{
		Object->Modify(true);
	}
}

TArray<const UObject*> FFeelCurveCustomization::GetOwners() const
{
	TArray<const UObject*> Owners;
	if (const UObject* Object = Owner.Get())
	{
		Owners.Add(Object);
	}
	return Owners;
}

void FFeelCurveCustomization::MakeTransactional()
{
	if (UObject* Object = Owner.Get())
	{
		Object->SetFlags(Object->GetFlags() | RF_Transactional);
	}
}

void FFeelCurveCustomization::OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos)
{
	// The recipe editor reads curves live; the asset only needs saving.
	if (UObject* Object = Owner.Get())
	{
		Object->MarkPackageDirty();
	}
}

bool FFeelCurveCustomization::IsValidCurve(FRichCurveEditInfo CurveInfo)
{
	return RuntimeCurve && CurveInfo.CurveToEdit == &RuntimeCurve->EditorCurveData;
}

#undef LOCTEXT_NAMESPACE
