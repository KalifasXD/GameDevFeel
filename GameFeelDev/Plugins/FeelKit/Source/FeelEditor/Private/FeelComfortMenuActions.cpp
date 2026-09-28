// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortMenuActions.h"

#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "FeelComfortMenu.h"
#include "FeelLibrary.h"
#include "FeelSettings.h"
#include "IContentBrowserSingleton.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ToolMenus.h"
#include "WidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "FeelComfortMenuActions"

namespace FeelComfortMenuActions
{
	namespace
	{
		bool IsFeelKitComfortMenu(const FAssetData& Asset)
		{
			if (!Asset.PackageName.ToString().StartsWith(TEXT("/FeelKit/")))
			{
				return false;
			}
			const FString ParentClass = Asset.GetTagValueRef<FString>(FBlueprintTags::ParentClassPath);
			return ParentClass.Contains(UFeelComfortMenu::StaticClass()->GetName());
		}
	}

	UWidgetBlueprint* CopyToProjectWithDialog(UWidgetBlueprint& Source)
	{
		FSaveAssetDialogConfig Config;
		Config.DialogTitleOverride = LOCTEXT("CopyDialogTitle", "Copy the comfort menu into your project");
		Config.DefaultPath = FeelLibrary::GetDefaultCopyFolder();
		Config.DefaultAssetName = TEXT("WBP_ComfortMenu");
		Config.AssetClassNames.Add(UWidgetBlueprint::StaticClass()->GetClassPathName());
		Config.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FString ChosenPath = IContentBrowserSingleton::Get().CreateModalSaveAssetDialog(Config);
		if (ChosenPath.IsEmpty())
		{
			return nullptr;
		}

		const FString PackageName = FPackageName::ObjectPathToPackageName(ChosenPath);
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		UWidgetBlueprint* Copy = Cast<UWidgetBlueprint>(AssetTools.DuplicateAsset(FPackageName::GetLongPackageAssetName(PackageName), FPackageName::GetLongPackagePath(PackageName), &Source));
		if (!Copy)
		{
			return nullptr;
		}

		const EAppReturnType::Type Answer = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(
			LOCTEXT("UseCopy", "Use {0} as this project's comfort menu?\n\nShow Feel Comfort Menu then opens your copy when it is given no menu class. You can change this later in Project Settings > Plugins > FeelKit > Comfort Menu Class."),
			FText::FromString(Copy->GetName())));
		if (Answer == EAppReturnType::Yes && Copy->GeneratedClass)
		{
			UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
			Settings->ComfortMenuClass = FSoftClassPath(Copy->GeneratedClass);
			Settings->TryUpdateDefaultConfigFile();
		}

		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Copy);
		return Copy;
	}

	void RegisterMenus()
	{
		UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UWidgetBlueprint::StaticClass());
		FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("GetAssetActions"));
		Section.AddDynamicEntry(TEXT("FeelComfortMenuCopy"), FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			const UContentBrowserAssetContextMenuContext* Context = InSection.FindContext<UContentBrowserAssetContextMenuContext>();
			if (!Context || Context->SelectedAssets.Num() != 1 || !IsFeelKitComfortMenu(Context->SelectedAssets[0]))
			{
				return;
			}
			InSection.AddMenuEntry(
				TEXT("FeelComfortMenuCopyToProject"),
				LOCTEXT("CopyToProject", "Copy to Project..."),
				LOCTEXT("CopyToProjectTip", "Make your own copy of the comfort menu to restyle. FeelKit's own copy lives in the engine folder, is shared by every project on this engine, and is replaced by the next FeelKit update."),
				FSlateIcon(),
				FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& MenuContext)
				{
					const UContentBrowserAssetContextMenuContext* ExecuteContext = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
					const TArray<UWidgetBlueprint*> Selected = ExecuteContext ? ExecuteContext->LoadSelectedObjects<UWidgetBlueprint>() : TArray<UWidgetBlueprint*>();
					if (Selected.Num() == 1 && Selected[0])
					{
						CopyToProjectWithDialog(*Selected[0]);
					}
				}));
		}));
	}
}

#undef LOCTEXT_NAMESPACE
