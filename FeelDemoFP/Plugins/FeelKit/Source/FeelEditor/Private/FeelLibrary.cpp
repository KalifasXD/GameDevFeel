// Copyright 2026 Billo. All Rights Reserved.

#include "FeelLibrary.h"

#include "AssetRegistry/AssetData.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "FeelEditorSettings.h"
#include "FeelRecipe.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Misc/NamePermissionList.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/AssetEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "FeelLibrary"

namespace FeelLibrary
{
	const TCHAR* LibraryRoot = TEXT("/FeelKit/Library/");

	/** Owner of the write permission entry, so only FeelKit's entry is removed again. */
	static const FName PermissionOwner(TEXT("FeelKitLibrary"));

	bool IsLibraryPath(FStringView PackageName)
	{
		return PackageName.StartsWith(LibraryRoot);
	}

	bool IsLibraryRecipe(const UFeelRecipe* Recipe)
	{
		return Recipe && IsLibraryPath(Recipe->GetPackage()->GetName());
	}

	bool IsLibraryAsset(const FAssetData& Asset)
	{
		return IsLibraryPath(Asset.PackageName.ToString());
	}

	bool IsLibraryEditingAllowed()
	{
		return GetDefault<UFeelEditorSettings>()->bAllowLibraryEditing;
	}

	bool IsReadOnly(const UFeelRecipe* Recipe)
	{
		return IsLibraryRecipe(Recipe) && !IsLibraryEditingAllowed();
	}

	void ApplyWritePermission()
	{
		FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		const TSharedRef<FPathPermissionList>& Writable = AssetTools.Get().GetWritableFolderPermissionList();
		Writable->UnregisterOwner(PermissionOwner);
		if (!IsLibraryEditingAllowed())
		{
			// The editor refuses to save packages here, and the Content Browser hides rename, delete and move.
			Writable->AddDenyListItem(PermissionOwner, LibraryRoot);
		}
	}

	void Startup()
	{
		ApplyWritePermission();
	}

	void Shutdown()
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
		{
			FAssetToolsModule& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
			AssetTools.Get().GetWritableFolderPermissionList()->UnregisterOwner(PermissionOwner);
		}
	}

	FString SuggestCopyName(const FString& LibraryAssetName)
	{
		// FR_<Feeling>_<Name> becomes <Name>.
		if (!LibraryAssetName.StartsWith(TEXT("FR_")))
		{
			return LibraryAssetName;
		}
		FString Rest = LibraryAssetName.RightChop(3);
		int32 Underscore = INDEX_NONE;
		if (!Rest.FindChar(TEXT('_'), Underscore) || Underscore + 1 >= Rest.Len())
		{
			return LibraryAssetName;
		}
		return Rest.RightChop(Underscore + 1);
	}

	UFeelRecipe* DuplicateRecipe(const UFeelRecipe& Source, const FString& PackagePath, const FString& AssetName)
	{
		if (PackagePath.IsEmpty() || AssetName.IsEmpty())
		{
			return nullptr;
		}

		FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		UFeelRecipe* Copy = Cast<UFeelRecipe>(AssetTools.Get().DuplicateAsset(AssetName, PackagePath, const_cast<UFeelRecipe*>(&Source)));
		if (!Copy)
		{
			return nullptr;
		}

#if WITH_EDITORONLY_DATA
		Copy->BasedOn = FSoftObjectPath(&Source);
#endif
		Copy->MarkPackageDirty();
		return Copy;
	}

	FString GetDefaultCopyFolder()
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("ContentBrowser")))
		{
			const FString Current = IContentBrowserSingleton::Get().GetCurrentPath().GetInternalPathString();
			if (!Current.IsEmpty() && !IsLibraryPath(Current) && FPackageName::IsValidPath(Current))
			{
				return Current;
			}
		}
		return TEXT("/Game");
	}

	UFeelRecipe* CopyToProjectWithDialog(const UFeelRecipe& Source, const FString& DefaultFolder)
	{
		FSaveAssetDialogConfig Config;
		Config.DialogTitleOverride = LOCTEXT("CopyDialogTitle", "Create a recipe from this template");
		Config.DefaultPath = DefaultFolder.IsEmpty() || IsLibraryPath(DefaultFolder) ? GetDefaultCopyFolder() : DefaultFolder;
		Config.DefaultAssetName = SuggestCopyName(Source.GetName());
		Config.AssetClassNames.Add(UFeelRecipe::StaticClass()->GetClassPathName());
		Config.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::Disallow;

		const FString ChosenPath = IContentBrowserSingleton::Get().CreateModalSaveAssetDialog(Config);
		if (ChosenPath.IsEmpty())
		{
			return nullptr;
		}

		const FString PackageName = FPackageName::ObjectPathToPackageName(ChosenPath);
		UFeelRecipe* Copy = DuplicateRecipe(Source, FPackageName::GetLongPackagePath(PackageName), FPackageName::GetLongPackageAssetName(PackageName));
		if (Copy)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Copy);
		}
		return Copy;
	}
}

#undef LOCTEXT_NAMESPACE
