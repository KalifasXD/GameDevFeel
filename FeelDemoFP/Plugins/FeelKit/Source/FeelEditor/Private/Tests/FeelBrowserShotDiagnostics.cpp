// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "ImageUtils.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "SFeelRecipeBrowser.h"
#include "Tests/AutomationCommon.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): opens the recipe browser, selects FR_Impact_HeavyHit, lets the editor run for a few
 * seconds (thumbnails are drawn on engine ticks) and saves a picture to Saved/FeelKit/RecipeBrowser.png.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FFeelBrowserShotCommand, FAutomationTestBase*, Test);

bool FFeelBrowserShotCommand::Update()
{
	const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(SFeelRecipeBrowser::TabName);
	TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	if (!Window.IsValid() && Tab.IsValid())
	{
		Window = FSlateApplication::Get().FindWidgetWindow(Tab->GetContent());
	}
	if (!Window.IsValid() && FSlateApplication::Get().GetTopLevelWindows().Num() > 0)
	{
		Window = FSlateApplication::Get().GetTopLevelWindows()[0];
	}

	TArray<FColor> Pixels;
	FIntVector Size(0, 0, 0);
	if (Window.IsValid() && FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) && Pixels.Num() > 0)
	{
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("RecipeBrowser.png");
		FImageView Image(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
		FImageUtils::SaveImageByExtension(*File, Image);
		Test->AddInfo(FString::Printf(TEXT("BROWSERSHOT wrote %s (%dx%d)"), *File, Size.X, Size.Y));
	}
	else
	{
		Test->AddError(TEXT("BROWSERSHOT could not take the picture"));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelBrowserShotDiagnostic, "DiagFeel.BrowserShot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelBrowserShotDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}

	const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(SFeelRecipeBrowser::TabName);
	if (!Tab.IsValid())
	{
		AddError(TEXT("The browser did not open."));
		return false;
	}
	StaticCastSharedRef<SFeelRecipeBrowser>(Tab->GetContent())->SelectRecipe(FSoftObjectPath(TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit")));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(3.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFeelBrowserShotCommand(this));
	return true;
}

#endif
