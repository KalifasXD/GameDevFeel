// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "FeelManualCapture.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Misc/App.h"
#include "SFeelRecipeBrowser.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the Recipe Browser for the manual's library chapter, saved to
 * Saved/FeelKit/Manual/Library_Browser.png at twice the screen's pixels and 1.25x application scale, with
 * FR_Impact_HeavyHit selected and playing. Needs a rendering session.
 */
namespace FeelLibraryShots
{
	TSharedPtr<SWindow> BrowserWindow()
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(SFeelRecipeBrowser::TabName));
		return Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryShotsDiagnostic, "DiagFeel.ManualShotsLibrary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelLibraryShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<float> PreviousScale = MakeShared<float>(FSlateApplication::Get().GetApplicationScale());
	FSlateApplication::Get().SetApplicationScale(1.25f);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
	{
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId(SFeelRecipeBrowser::TabName));
		if (const TSharedPtr<SWindow> Window = FeelLibraryShots::BrowserWindow())
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		const TSharedPtr<SWindow> Window = FeelLibraryShots::BrowserWindow();
		const TSharedPtr<SWidget> Browser = Window.IsValid() ? FeelManualCapture::FindWidget(Window.ToSharedRef(), TEXT("SFeelRecipeBrowser")) : nullptr;
		if (!Browser.IsValid())
		{
			AddWarning(TEXT("MANUALSHOT Library_Browser: no browser widget"));
			return true;
		}
		// Source: Library, as a click on the button would set it, so the list shows only the recipes that ship.
		if (const TSharedPtr<SWidget> Label = FeelManualCapture::FindText(Browser.ToSharedRef(), TEXT("Library")))
		{
			const FGeometry& Geometry = Label->GetTickSpaceGeometry();
			const FVector2D Center = FVector2D(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f));
			FSlateApplication& Slate = FSlateApplication::Get();
			const FPointerEvent Down(0, Center, Center, TSet<FKey>({ EKeys::LeftMouseButton }), EKeys::LeftMouseButton, 0.0f, FModifierKeysState());
			const FPointerEvent Up(0, Center, Center, TSet<FKey>(), EKeys::LeftMouseButton, 0.0f, FModifierKeysState());
			Slate.ProcessMouseButtonDownEvent(nullptr, Down);
			Slate.ProcessMouseButtonUpEvent(Up);
		}
		StaticCastSharedPtr<SFeelRecipeBrowser>(Browser)->SelectRecipe(FSoftObjectPath(TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit")));
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!FeelManualCapture::SaveWindow(FeelLibraryShots::BrowserWindow(), TEXT("Library_Browser"), 2.0f, this))
		{
			return false;
		}
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(SFeelRecipeBrowser::TabName)))
		{
			Tab->RequestCloseTab();
		}
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([PreviousScale]()
	{
		FSlateApplication::Get().SetApplicationScale(*PreviousScale);
		return true;
	}));
	return true;
}

#endif
