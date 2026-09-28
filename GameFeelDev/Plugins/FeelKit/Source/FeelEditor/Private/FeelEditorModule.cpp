// Copyright 2026 Billo. All Rights Reserved.

// FEELKIT_PRO_BEGIN
#include "FeelComfortAudit.h"
// FEELKIT_PRO_END
#include "FeelComfortMenuActions.h"
// FEELKIT_PRO_BEGIN
#include "FeelGraphPinFactory.h"
// FEELKIT_PRO_END
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelRecipeThumbnailRenderer.h"
// FEELKIT_PRO_BEGIN
#include "FeelRecipeJson.h"
// FEELKIT_PRO_END
#include "FeelSaveValidationLog.h"
#include "FeelTrackClipboard.h"
#include "Modules/ModuleManager.h"
// FEELKIT_PRO_BEGIN
#include "SFeelDebugger.h"
#include "SFeelRecipeBrowser.h"
// FEELKIT_PRO_END
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FeelEditorModule"

/** FeelKit editor module: owns editor-wide services such as the track clipboard, save validation messages and tools. */
class FFeelEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FFeelTrackClipboard::Startup();
		FeelSaveValidationLog::Startup();
		// FEELKIT_PRO_BEGIN
		FFeelComfortAudit::Startup();
		// FEELKIT_PRO_END
		FeelLibrary::Startup();
		// FEELKIT_PRO_BEGIN
		SFeelDebugger::RegisterTab();
		SFeelRecipeBrowser::RegisterTab();
		// FEELKIT_PRO_END
		UThumbnailManager::Get().RegisterCustomRenderer(UFeelRecipe::StaticClass(), UFeelRecipeThumbnailRenderer::StaticClass());
		// FEELKIT_PRO_BEGIN
		FFeelGraphPinFactory::Register();
		// FEELKIT_PRO_END
		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFeelEditorModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		UToolMenus::UnregisterOwner(this);
		// FEELKIT_PRO_BEGIN
		FFeelGraphPinFactory::Unregister();
		SFeelRecipeBrowser::UnregisterTab();
		SFeelDebugger::UnregisterTab();
		// FEELKIT_PRO_END
		if (UObjectInitialized())
		{
			UThumbnailManager::Get().UnregisterCustomRenderer(UFeelRecipe::StaticClass());
		}
		FeelLibrary::Shutdown();
		// FEELKIT_PRO_BEGIN
		FFeelComfortAudit::Shutdown();
		// FEELKIT_PRO_END
		FeelSaveValidationLog::Shutdown();
		FFeelTrackClipboard::Shutdown();
	}

private:
	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);
		// FEELKIT_PRO_BEGIN
		FFeelRecipeJson::RegisterMenus();
		// FEELKIT_PRO_END
		FeelComfortMenuActions::RegisterMenus();

		// FEELKIT_PRO_BEGIN
		// Content Browser: right-click empty space in a folder.
		for (const TCHAR* MenuName : { TEXT("ContentBrowser.AddNewContextMenu"), TEXT("ContentBrowser.FolderContextMenu") })
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(FName(MenuName));
			FToolMenuSection& FeelSection = Menu->FindOrAddSection(TEXT("FeelKit"), LOCTEXT("FeelKitContentSection", "FeelKit"));
			FeelSection.AddMenuEntry(
				TEXT("FeelKitRecipeFromTemplate"),
				LOCTEXT("RecipeFromTemplate", "Recipe from Template..."),
				LOCTEXT("RecipeFromTemplateTip", "Pick a recipe that ships with FeelKit and create an editable copy of it in this folder."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					SFeelRecipeBrowser::OpenTemplatePicker(FeelLibrary::GetDefaultCopyFolder());
				})));
			FeelSection.AddMenuEntry(
				TEXT("FeelKitBrowseRecipes"),
				LOCTEXT("BrowseRecipes", "Browse Recipes..."),
				LOCTEXT("BrowseRecipesTip", "Open the FeelKit Recipe Browser: every recipe of the library and of this project, with a live preview."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([]()
				{
					FGlobalTabmanager::Get()->TryInvokeTab(SFeelRecipeBrowser::TabName);
				})));
		}

		UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
		FToolMenuSection& Section = ToolsMenu->FindOrAddSection(TEXT("FeelKit"), LOCTEXT("FeelKitSection", "FeelKit"));
		Section.AddMenuEntry(
			TEXT("FeelKitComfortAudit"),
			LOCTEXT("ComfortAuditMenu", "FeelKit Comfort Audit"),
			LOCTEXT("ComfortAuditMenuTip", "Check every recipe and the comfort settings for likely comfort problems, such as more than three flashes in a second or saturated red flashes. Results appear in the Message Log. A readiness helper, not a certification."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateStatic(&FFeelComfortAudit::RunProjectAudit)));

		Section.AddMenuEntry(
			TEXT("FeelKitRecipeBrowser"),
			LOCTEXT("RecipeBrowserMenu", "FeelKit Recipe Browser"),
			LOCTEXT("RecipeBrowserMenuTip", "Browse the recipes that ship with FeelKit and the recipes of this project, see and hear them in a preview, and copy one into your project."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(SFeelRecipeBrowser::TabName);
			})));
		// FEELKIT_PRO_END
	}
};

IMPLEMENT_MODULE(FFeelEditorModule, FeelEditor);

#undef LOCTEXT_NAMESPACE
