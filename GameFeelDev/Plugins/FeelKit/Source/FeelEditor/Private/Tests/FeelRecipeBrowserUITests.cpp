// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "ImageUtils.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "IAssetTools.h"
#include "Misc/NamePermissionList.h"
#include "Modules/ModuleManager.h"
#include "FeelEditorSettings.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "SFeelPreviewViewport.h"
#include "SFeelRecipeBrowser.h"
#include "Sound/SoundWave.h"
#include "Widgets/Docking/SDockTab.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

/**
 * Opens the recipe browser tab and closes it again. Needs a rendering session, so it is skipped when the editor runs
 * headless (-nullrhi), like the details focus test.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeBrowserUITest, "FeelKit.Editor.RecipeBrowserUI", FEEL_TEST_FLAGS)
bool FFeelRecipeBrowserUITest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddInfo(TEXT("Skipped: this session cannot render, so Slate widgets cannot be built."));
		return true;
	}

	const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(SFeelRecipeBrowser::TabName);
	if (!TestValid(TEXT("The browser tab opens"), Tab))
	{
		return false;
	}

	const TSharedRef<SWidget> Content = Tab->GetContent();
	TestTrue(TEXT("The tab holds the recipe browser"), Content->GetTypeAsString().Contains(TEXT("FeelRecipeBrowser")));

	// Let the widget tick once, which also ticks its preview.
	FSlateApplication::Get().Tick();

	Tab->RequestCloseTab();
	FSlateApplication::Get().Tick();
	return true;
}

/** The preview's Mute: while muted, the preview world starts no sounds; unmuting allows them again. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPreviewMuteTest, "FeelKit.Editor.PreviewMute", FEEL_TEST_FLAGS)
bool FFeelPreviewMuteTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddInfo(TEXT("Skipped: this session cannot render, so Slate widgets cannot be built."));
		return true;
	}

	const TSharedRef<FFeelRecipeEditorState> State = MakeShared<FFeelRecipeEditorState>(nullptr);
	const TSharedRef<SFeelPreviewViewport> Viewport = SNew(SFeelPreviewViewport, State);
	UWorld* World = Viewport->GetPreviewWorld();
	if (!TestNotNull(TEXT("The preview has a world"), World))
	{
		return false;
	}
	TestTrue(TEXT("Audio is allowed before muting"), World->AllowAudioPlayback());

	Viewport->SetAudioMuted(true);
	TestFalse(TEXT("Muting stops audio in the preview world"), World->AllowAudioPlayback());
	USoundWave* Sound = NewObject<USoundWave>(GetTransientPackage());
	TestNull(TEXT("No sound starts while muted"), UGameplayStatics::SpawnSound2D(World, Sound));

	Viewport->SetAudioMuted(false);
	TestTrue(TEXT("Unmuting allows audio again"), World->AllowAudioPlayback());
	return true;
}

/**
 * Double-clicking a library recipe in the Content Browser opens the recipe editor. Repeats the Content Browser's
 * decision on the real library asset, with library editing off. Skipped when the library has not been imported.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryOpenFromContentBrowserTest, "FeelKit.Editor.LibraryOpensFromContentBrowser", FEEL_TEST_FLAGS)
bool FFeelLibraryOpenFromContentBrowserTest::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddInfo(TEXT("Skipped: this session cannot render, so the recipe editor cannot be built."));
		return true;
	}

	UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit"));
	if (!Recipe)
	{
		AddInfo(TEXT("Skipped: the library recipe FR_Impact_HeavyHit is not in this project."));
		return true;
	}

	UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
	const bool bPreviousAllow = Settings->bAllowLibraryEditing;
	Settings->bAllowLibraryEditing = false;
	FeelLibrary::ApplyWritePermission();
	UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	ON_SCOPE_EXIT
	{
		AssetEditors->CloseAllEditorsForAsset(Recipe);
		Settings->bAllowLibraryEditing = bPreviousAllow;
		FeelLibrary::ApplyWritePermission();
	};

	AssetEditors->CloseAllEditorsForAsset(Recipe);

	// What a double-click does (ContentBrowserAssetDataCore, EditOrPreviewItems): an asset in a folder that does not
	// allow edits is opened for viewing, and only if its type supports that.
	FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	TestFalse(TEXT("The library folder does not allow edits"),
		AssetTools.Get().GetWritableFolderPermissionList()->PassesStartsWithFilter(Recipe->GetPackage()->GetName()));
	const TSharedPtr<IAssetTypeActions> Actions = AssetTools.Get().GetAssetTypeActionsForClass(UFeelRecipe::StaticClass()).Pin();
	if (!TestTrue(TEXT("Recipes can be opened for viewing"), Actions.IsValid() && Actions->SupportsOpenedMethod(EAssetTypeActivationOpenedMethod::View)))
	{
		return false;
	}
	TestTrue(TEXT("Opening it for viewing succeeds"), AssetEditors->OpenEditorForAssets({ Recipe }, EAssetTypeActivationOpenedMethod::View));
	IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Recipe, false);
	if (!TestNotNull(TEXT("The recipe editor is open"), Editor))
	{
		return false;
	}

	// Unreal's view-only mode disables every panel (preview, Play, Copy to Project). The recipe editor must not use it.
	TestTrue(TEXT("The recipe editor opens in the normal mode"), Editor->GetOpenMethod() == EAssetOpenMethod::Edit);
	const TSharedPtr<FTabManager> EditorTabs = Editor->GetAssociatedTabManager();
	TestTrue(TEXT("The recipe editor's panels are not disabled"), EditorTabs.IsValid() && !EditorTabs->IsReadOnly());
	return true;
}

/**
 * Diagnostic (filter DiagFeel): opens FR_Impact_HeavyHit the way a double-click does, with library editing off, and saves
 * a picture of the recipe editor window to Saved/FeelKit/RecipeEditor.png. Needs a rendering session.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeEditorShotDiagnostic, "DiagFeel.RecipeEditorShot", FEEL_TEST_FLAGS)
bool FFeelRecipeEditorShotDiagnostic::RunTest(const FString& Parameters)
{
	UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit"));
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor || !Recipe)
	{
		AddError(TEXT("Needs a rendering session and the library recipe FR_Impact_HeavyHit."));
		return false;
	}

	UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
	const bool bPreviousAllow = Settings->bAllowLibraryEditing;
	Settings->bAllowLibraryEditing = false;
	FeelLibrary::ApplyWritePermission();
	UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	ON_SCOPE_EXIT
	{
		AssetEditors->CloseAllEditorsForAsset(Recipe);
		Settings->bAllowLibraryEditing = bPreviousAllow;
		FeelLibrary::ApplyWritePermission();
	};

	AssetEditors->CloseAllEditorsForAsset(Recipe);
	AssetEditors->OpenEditorForAssets({ Recipe }, EAssetTypeActivationOpenedMethod::View);
	IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Recipe, false);
	const TSharedPtr<FTabManager> EditorTabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
	const TSharedPtr<SDockTab> OwnerTab = EditorTabs.IsValid() ? EditorTabs->GetOwnerTab() : nullptr;
	const TSharedPtr<SWindow> Window = OwnerTab.IsValid() ? OwnerTab->GetParentWindow() : nullptr;
	if (!Window.IsValid())
	{
		AddError(TEXT("The recipe editor window was not found."));
		return false;
	}

	Window->Resize(FVector2D(1600.0, 1000.0));
	for (int32 Frame = 0; Frame < 30; ++Frame)
	{
		FSlateApplication::Get().Tick();
		FPlatformProcess::Sleep(0.02f);
	}

	TArray<FColor> Pixels;
	FIntVector Size;
	if (!FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) || Pixels.Num() == 0)
	{
		AddError(TEXT("Could not take the picture."));
		return false;
	}
	for (FColor& Pixel : Pixels)
	{
		Pixel.A = 255;
	}
	const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("RecipeEditor.png");
	FImageView Image(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
	if (!FImageUtils::SaveImageByExtension(*File, Image))
	{
		AddError(TEXT("Could not save the picture."));
		return false;
	}
	AddInfo(FString::Printf(TEXT("EDITORSHOT wrote %s (%dx%d), open method %s, panels disabled %s"), *File, Size.X, Size.Y,
		Editor->GetOpenMethod() == EAssetOpenMethod::Edit ? TEXT("Edit") : TEXT("View"), EditorTabs->IsReadOnly() ? TEXT("yes") : TEXT("no")));

	// Second picture: an editable recipe with a selected track, which shows the selection and the intensity lane.
	Settings->bAllowLibraryEditing = true;
	FeelLibrary::ApplyWritePermission();
	if (const TSharedPtr<FFeelRecipeEditorState> OpenState = FFeelRecipeEditorState::FindOpenState(Recipe))
	{
		// The user's case: one track soloed, so the others say why they are silent, plus a No preview step.
		OpenState->SetSelectedTrack(4);
		OpenState->ToggleSolo(4);
		OpenState->SetSelectedTrack(1);
	}
	for (int32 Frame = 0; Frame < 30; ++Frame)
	{
		FSlateApplication::Get().Tick();
		FPlatformProcess::Sleep(0.02f);
	}
	Pixels.Reset();
	if (FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size) && Pixels.Num() > 0)
	{
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString SelectedFile = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("RecipeEditorSelected.png");
		FImageView SelectedImage(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
		FImageUtils::SaveImageByExtension(*SelectedFile, SelectedImage);
		AddInfo(FString::Printf(TEXT("EDITORSHOT wrote %s"), *SelectedFile));
	}

	// Third picture: the recipe browser in the main window.
	AssetEditors->CloseAllEditorsForAsset(Recipe);
	const TSharedPtr<SDockTab> BrowserTab = FGlobalTabmanager::Get()->TryInvokeTab(SFeelRecipeBrowser::TabName);
	AddInfo(FString::Printf(TEXT("EDITORSHOT browser tab %s"), BrowserTab.IsValid() ? TEXT("opened") : TEXT("did not open")));
	if (BrowserTab.IsValid())
	{
		for (int32 Frame = 0; Frame < 40; ++Frame)
		{
			FSlateApplication::Get().Tick();
			FPlatformProcess::Sleep(0.02f);
		}
		TSharedPtr<SWindow> BrowserWindow = BrowserTab->GetParentWindow();
		if (!BrowserWindow.IsValid())
		{
			BrowserWindow = FSlateApplication::Get().FindWidgetWindow(BrowserTab->GetContent());
		}
		if (!BrowserWindow.IsValid() && FSlateApplication::Get().GetTopLevelWindows().Num() > 0)
		{
			BrowserWindow = FSlateApplication::Get().GetTopLevelWindows()[0];
		}
		AddInfo(FString::Printf(TEXT("EDITORSHOT browser window %s"), BrowserWindow.IsValid() ? *BrowserWindow->GetTitle().ToString() : TEXT("not found")));
		Pixels.Reset();
		if (BrowserWindow.IsValid() && FSlateApplication::Get().TakeScreenshot(BrowserWindow.ToSharedRef(), Pixels, Size) && Pixels.Num() > 0)
		{
			for (FColor& Pixel : Pixels)
			{
				Pixel.A = 255;
			}
			const FString BrowserFile = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("RecipeBrowser.png");
			FImageView BrowserImage(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
			FImageUtils::SaveImageByExtension(*BrowserFile, BrowserImage);
			AddInfo(FString::Printf(TEXT("EDITORSHOT wrote %s"), *BrowserFile));
		}
		BrowserTab->RequestCloseTab();
	}
	return true;
}

/**
 * Diagnostic (filter DiagFeel): saves a picture of every visible editor window to Saved/FeelKit/Window_<n>.png, after
 * letting the editor draw for a moment. Used to compare FeelKit's windows with their Unreal counterparts.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelWindowShotDiagnostic, "DiagFeel.WindowShot", FEEL_TEST_FLAGS)
bool FFeelWindowShotDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized())
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}

	for (int32 Frame = 0; Frame < 60; ++Frame)
	{
		FSlateApplication::Get().Tick();
		FPlatformProcess::Sleep(0.02f);
	}

	int32 Index = 0;
	for (const TSharedRef<SWindow>& Window : FSlateApplication::Get().GetTopLevelWindows())
	{
		if (!Window->IsVisible())
		{
			continue;
		}
		TArray<FColor> Pixels;
		FIntVector Size;
		if (!FSlateApplication::Get().TakeScreenshot(Window, Pixels, Size) || Pixels.Num() == 0)
		{
			continue;
		}
		for (FColor& Pixel : Pixels)
		{
			Pixel.A = 255;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / FString::Printf(TEXT("Window_%d.png"), Index++);
		FImageView Image(Pixels.GetData(), Size.X, Size.Y, ERawImageFormat::BGRA8);
		FImageUtils::SaveImageByExtension(*File, Image);
		AddInfo(FString::Printf(TEXT("WINDOWSHOT %s %s (%dx%d)"), *File, *Window->GetTitle().ToString(), Size.X, Size.Y));
	}
	return Index > 0;
}

#endif
