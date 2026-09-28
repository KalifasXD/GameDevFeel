// Tool (filter DiagFeel): renders the thumbnail of every FeelKit recipe under /FeelKit and saves it into the recipe's
// package, so tiles that are not loaded yet show the current look. Library recipes are read-only, so Allow Library
// Editing is switched on for the run and restored afterwards. Each thumbnail is then read back from the saved file and
// one of them is written as a picture to Saved/FeelKit/ThumbnailFromDisk.png. Needs a rendering session (no -nullrhi).
// Back up Plugins/FeelKit/Content first. Editor builds only.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "FeelRecipe.h"
#include "FileHelpers.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/ObjectThumbnail.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"

namespace FeelThumbnailRefresh
{
	// Sets Allow Library Editing through reflection (the settings class is private to FeelEditor) and lets the settings
	// object apply the folder permission, as the Editor Preferences checkbox does.
	bool SetLibraryEditing(bool bAllow)
	{
		UClass* SettingsClass = FindObject<UClass>(nullptr, TEXT("/Script/FeelEditor.FeelEditorSettings"));
		UObject* Settings = SettingsClass ? SettingsClass->GetDefaultObject() : nullptr;
		FBoolProperty* Property = SettingsClass ? FindFProperty<FBoolProperty>(SettingsClass, TEXT("bAllowLibraryEditing")) : nullptr;
		if (!Settings || !Property)
		{
			return false;
		}
		const bool bWas = Property->GetPropertyValue_InContainer(Settings);
		Property->SetPropertyValue_InContainer(Settings, bAllow);
		FPropertyChangedEvent Event(Property);
		Settings->PostEditChangeProperty(Event);
		return bWas;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeThumbnailRefresh, "DiagFeel.RefreshRecipeThumbnails", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFeelRecipeThumbnailRefresh::RunTest(const FString& Parameters)
{
	IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	Registry.SearchAllAssets(true);
	FARFilter Filter;
	Filter.PackagePaths.Add(TEXT("/FeelKit"));
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UFeelRecipe::StaticClass()->GetClassPathName());
	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);
	AddInfo(FString::Printf(TEXT("THUMBS recipes under /FeelKit: %d"), Assets.Num()));

	const bool bWasAllowed = FeelThumbnailRefresh::SetLibraryEditing(true);

	TArray<UPackage*> Packages;
	int32 Generated = 0;
	for (const FAssetData& Asset : Assets)
	{
		UObject* Recipe = Asset.GetAsset();
		if (!Recipe)
		{
			AddError(FString::Printf(TEXT("THUMBS could not load %s"), *Asset.GetObjectPathString()));
			continue;
		}
		if (ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(Recipe))
		{
			++Generated;
		}
		Recipe->GetPackage()->MarkPackageDirty();
		Packages.Add(Recipe->GetPackage());
	}
	const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(Packages, false);
	FeelThumbnailRefresh::SetLibraryEditing(bWasAllowed);
	AddInfo(FString::Printf(TEXT("THUMBS rendered %d, saved %d packages: %s, Allow Library Editing back to %s"),
		Generated, Packages.Num(), bSaved ? TEXT("yes") : TEXT("no"), bWasAllowed ? TEXT("on") : TEXT("off")));

	// Read every thumbnail back from the saved files.
	int32 OnDisk = 0;
	bool bPictureWritten = false;
	for (const FAssetData& Asset : Assets)
	{
		FString File;
		if (!FPackageName::DoesPackageExist(Asset.PackageName.ToString(), &File))
		{
			continue;
		}
		const FName FullName(*Asset.GetFullName());
		FThumbnailMap Thumbnails;
		ThumbnailTools::LoadThumbnailsFromPackage(File, { FullName }, Thumbnails);
		const FObjectThumbnail* Thumbnail = Thumbnails.Find(FullName);
		if (!Thumbnail || Thumbnail->IsEmpty() || Thumbnail->GetImageWidth() <= 0)
		{
			AddError(FString::Printf(TEXT("THUMBS no thumbnail on disk for %s"), *Asset.AssetName.ToString()));
			continue;
		}
		++OnDisk;
		if (!bPictureWritten && Asset.PackagePath.ToString().StartsWith(TEXT("/FeelKit/Library")))
		{
			const TArray<uint8>& Bytes = const_cast<FObjectThumbnail*>(Thumbnail)->GetUncompressedImageData();
			const int32 Width = Thumbnail->GetImageWidth();
			const int32 Height = Thumbnail->GetImageHeight();
			if (Bytes.Num() == Width * Height * 4)
			{
				TArray<FColor> Pixels;
				Pixels.SetNumUninitialized(Width * Height);
				FMemory::Memcpy(Pixels.GetData(), Bytes.GetData(), Bytes.Num());
				for (FColor& Pixel : Pixels)
				{
					Pixel.A = 255;
				}
				TArray64<uint8> Png;
				FImageUtils::PNGCompressImageArray(Width, Height, TArrayView64<const FColor>(Pixels.GetData(), Pixels.Num()), Png);
				const FString Picture = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("ThumbnailFromDisk.png");
				bPictureWritten = FFileHelper::SaveArrayToFile(Png, *Picture);
				AddInfo(FString::Printf(TEXT("THUMBS picture of %s (%dx%d) read from disk: %s"), *Asset.AssetName.ToString(), Width, Height, *Picture));
			}
		}
	}
	AddInfo(FString::Printf(TEXT("THUMBS thumbnails found in the saved files: %d of %d"), OnDisk, Assets.Num()));
	TestEqual(TEXT("every recipe has a thumbnail on disk"), OnDisk, Assets.Num());
	return true;
}

#endif
