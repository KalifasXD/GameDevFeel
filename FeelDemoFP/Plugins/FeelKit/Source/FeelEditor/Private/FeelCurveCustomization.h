// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Curves/CurveOwnerInterface.h"
#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class SWindow;
struct FRuntimeFloatCurve;

/**
 * Curve rows in the recipe editor's Details panel: a small preview of the curve and an Edit button that opens the curve
 * editor in its own window. Unreal's inline curve editor rebuilds the whole Details panel after any value change, which
 * throws the keyboard focus out of the field being edited (Tab and Enter stopped working); this row avoids that.
 */
class FFeelCurveCustomization : public IPropertyTypeCustomization, public FCurveOwnerInterface
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	virtual ~FFeelCurveCustomization() override;

	//~ IPropertyTypeCustomization
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	//~ FCurveOwnerInterface
	virtual TArray<FRichCurveEditInfoConst> GetCurves() const override;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 7
	virtual void GetCurves(TAdderReserverRef<FRichCurveEditInfoConst> Curves) const override;
#endif
	virtual TArray<FRichCurveEditInfo> GetCurves() override;
	virtual void ModifyOwner() override;
	virtual TArray<const UObject*> GetOwners() const override;
	virtual void MakeTransactional() override;
	virtual void OnCurveChanged(const TArray<FRichCurveEditInfo>& ChangedCurveEditInfos) override;
	virtual bool IsValidCurve(FRichCurveEditInfo CurveInfo) override;

private:
	FReply OpenCurveWindow();
	void CloseCurveWindow();

	TSharedPtr<IPropertyHandle> StructHandle;
	FRuntimeFloatCurve* RuntimeCurve = nullptr;
	TWeakObjectPtr<UObject> Owner;
	TWeakPtr<SWindow> CurveWindow;
	TSharedPtr<class SCurveEditor> CurveEditor;
};
