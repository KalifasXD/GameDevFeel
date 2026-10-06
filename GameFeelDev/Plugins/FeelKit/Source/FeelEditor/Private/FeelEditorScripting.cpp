// Copyright 2026 Billo. All Rights Reserved.

#include "FeelEditorScripting.h"

#include "AssetToolsModule.h"
// FEELKIT_PRO_BEGIN
#include "FeelLibrary.h"
#include "FeelMap.h"
#include "Misc/FeedbackContext.h"
// FEELKIT_PRO_END
#include "FeelRecipe.h"
#include "FeelRecipeFactory.h"
// FEELKIT_PRO_BEGIN
#include "FeelRecipeJson.h"
// FEELKIT_PRO_END
#include "FeelSettings.h"
#include "IAssetTools.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelEditorScripting)

// FEELKIT_PRO_BEGIN
namespace FeelEditorScriptingPrivate
{
	bool SaveAsset(UObject* Asset)
	{
		UPackage* Package = Asset ? Asset->GetPackage() : nullptr;
		if (!Package)
		{
			return false;
		}
		const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		// A refused save (read-only folder, file locked) is reported, not fatal.
		Args.Error = GWarn;
		return UPackage::SavePackage(Package, Asset, *Filename, Args);
	}
}

bool UFeelEditorScripting::ImportRecipeFromJsonFile(UFeelRecipe* Recipe, const FString& FilePath, FString& OutMessage)
{
	if (!Recipe)
	{
		OutMessage = TEXT("No recipe.");
		return false;
	}
	if (FeelLibrary::IsReadOnly(Recipe))
	{
		OutMessage = FString::Printf(TEXT("%s is in the FeelKit library, which is read-only. To change it, turn on Editor Preferences > Plugins > FeelKit > Allow Library Editing."), *Recipe->GetName());
		return false;
	}

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *FilePath))
	{
		OutMessage = FString::Printf(TEXT("Could not read %s."), *FilePath);
		return false;
	}

	FText Error;
	TArray<FString> MissingAssets;
	Recipe->Modify();
	if (!FFeelRecipeJson::Import(*Recipe, Json, Error, &MissingAssets))
	{
		OutMessage = Error.ToString();
		return false;
	}
	Recipe->MarkPackageDirty();
	const bool bSaved = FeelEditorScriptingPrivate::SaveAsset(Recipe);

	OutMessage = FString::Printf(TEXT("Imported %s into %s%s."), *FPaths::GetCleanFilename(FilePath), *Recipe->GetName(), bSaved ? TEXT(" and saved it") : TEXT(" (not saved)"));
	if (MissingAssets.Num() > 0)
	{
		OutMessage += FString::Printf(TEXT(" Asset references not in this project, left empty: %s"), *FString::Join(MissingAssets, TEXT(", ")));
	}
	return true;
}

UFeelRecipe* UFeelEditorScripting::CreateRecipeFromJsonFile(const FString& PackagePath, const FString& AssetName, const FString& FilePath, FString& OutMessage)
{
	const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), *PackagePath, *AssetName, *AssetName);
	UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet);
	if (!Recipe)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools")).Get();
		Recipe = Cast<UFeelRecipe>(AssetTools.CreateAsset(AssetName, PackagePath, UFeelRecipe::StaticClass(), NewObject<UFeelRecipeFactory>()));
	}
	if (!Recipe)
	{
		OutMessage = FString::Printf(TEXT("Could not create %s in %s."), *AssetName, *PackagePath);
		return nullptr;
	}
	return ImportRecipeFromJsonFile(Recipe, FilePath, OutMessage) ? Recipe : nullptr;
}

bool UFeelEditorScripting::AddFeelMapToProjectSettings(UFeelMap* FeelMap)
{
	if (!FeelMap)
	{
		return false;
	}
	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const TSoftObjectPtr<UFeelMap> Entry(FeelMap);
	if (!Settings->FeelMaps.Contains(Entry))
	{
		Settings->FeelMaps.Add(Entry);
		Settings->TryUpdateDefaultConfigFile();
	}
	return Settings->FeelMaps.Contains(Entry);
}

bool UFeelEditorScripting::SetAccumulatorInProjectSettings(FName Name, float MaxValue, float DecayPerSecond, float DecayDelay)
{
	if (Name.IsNone())
	{
		return false;
	}
	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	FFeelAccumulatorDefinition* Definition = Settings->Accumulators.FindByPredicate([Name](const FFeelAccumulatorDefinition& Candidate) { return Candidate.Name == Name; });
	if (!Definition)
	{
		Definition = &Settings->Accumulators.AddDefaulted_GetRef();
		Definition->Name = Name;
	}
	Definition->MaxValue = FMath::Max(MaxValue, 0.0f);
	Definition->DecayPerSecond = FMath::Max(DecayPerSecond, 0.0f);
	Definition->DecayDelay = FMath::Max(DecayDelay, 0.0f);
	Settings->TryUpdateDefaultConfigFile();
	return Settings->FindAccumulator(Name) != nullptr;
}
// FEELKIT_PRO_END
