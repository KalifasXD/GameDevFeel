// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeJson.h"

#include "ContentBrowserMenuContexts.h"
#include "DesktopPlatformModule.h"
#include "Dom/JsonObject.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelStep.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IDesktopPlatform.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ScopedTransaction.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ToolMenus.h"
#include "UObject/UObjectHash.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "FeelRecipeJson"

const TCHAR* FFeelRecipeJson::FormatName = TEXT("FeelKitRecipe");

namespace FeelRecipeJsonPrivate
{
	/** Transient data never belongs in the file. Everything else, including editor-only curve keys, is written. */
	constexpr int64 SkipFlags = CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient;

	/** Which library recipe an asset was copied from belongs to the asset, not to the recipe data. */
	const FName BasedOnProperty(TEXT("BasedOn"));
	const FString BasedOnField(TEXT("basedOn"));

	void Notify(const FText& Message, bool bSuccess)
	{
		FNotificationInfo Info(Message);
		Info.ExpireDuration = bSuccess ? 4.0f : 8.0f;
		const TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info);
		if (Item.IsValid())
		{
			Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}
}

FString FFeelRecipeJson::Export(const UFeelRecipe& Recipe)
{
	const TSharedRef<FJsonObject> RecipeObject = MakeShared<FJsonObject>();
	FJsonObjectConverter::UStructToJsonObject(UFeelRecipe::StaticClass(), &Recipe, RecipeObject, 0, FeelRecipeJsonPrivate::SkipFlags);

	// Where a copy came from is about this asset, not about the recipe, so it is never written or read.
	RecipeObject->RemoveField(FeelRecipeJsonPrivate::BasedOnField);

	const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetStringField(TEXT("format"), FormatName);
	Root->SetNumberField(TEXT("schemaVersion"), UFeelRecipe::CurrentSchemaVersion);
	Root->SetStringField(TEXT("name"), Recipe.GetName());
	Root->SetObjectField(TEXT("recipe"), RecipeObject);

	FString Out;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Root, Writer);
	return Out;
}

bool FFeelRecipeJson::Import(UFeelRecipe& Recipe, const FString& Json, FText& OutError, TArray<FString>* OutMissingAssets)
{
	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = LOCTEXT("NotJson", "The file is not valid JSON.");
		return false;
	}

	FString Format;
	const TSharedPtr<FJsonObject>* RecipeObject = nullptr;
	if (!Root->TryGetStringField(TEXT("format"), Format) || Format != FormatName || !Root->TryGetObjectField(TEXT("recipe"), RecipeObject))
	{
		OutError = LOCTEXT("NotRecipe", "The file is not a FeelKit recipe (its \"format\" field is missing or different).");
		return false;
	}

	int32 FileSchema = 0;
	Root->TryGetNumberField(TEXT("schemaVersion"), FileSchema);
	if (FileSchema > UFeelRecipe::CurrentSchemaVersion)
	{
		OutError = FText::Format(LOCTEXT("NewerSchema", "The file was written by a newer FeelKit (recipe format {0}; this version reads up to {1}). Update FeelKit to import it."),
			FText::AsNumber(FileSchema), FText::AsNumber(UFeelRecipe::CurrentSchemaVersion));
		return false;
	}

	// Import into a scratch recipe first, so a failure leaves the real recipe untouched.
	UFeelRecipe* Scratch = NewObject<UFeelRecipe>(GetTransientPackage());
	FText FailReason;

	// An asset the importing project does not have (a sound, material or curve) leaves that reference empty and is
	// reported, instead of failing the whole import.
	TArray<FString> MissingAssets;
	const FJsonObjectConverter::CustomImportCallback ImportCallback = FJsonObjectConverter::CustomImportCallback::CreateLambda(
		[&MissingAssets](const TSharedPtr<FJsonValue>& JsonValue, FProperty* Property, void* Value)
		{
			const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property);
			if (!ObjectProperty || !JsonValue.IsValid() || JsonValue->Type != EJson::String)
			{
				return false;
			}

			const FString Path = JsonValue->AsString();
			if (Path.IsEmpty() || Path == TEXT("None"))
			{
				ObjectProperty->SetObjectPropertyValue(Value, nullptr);
				return true;
			}

			UObject* Asset = StaticLoadObject(ObjectProperty->PropertyClass, nullptr, *Path, nullptr, LOAD_NoWarn | LOAD_Quiet);
			if (!Asset)
			{
				MissingAssets.AddUnique(Path);
			}
			ObjectProperty->SetObjectPropertyValue(Value, Asset);
			return true;
		});

	if (!FJsonObjectConverter::JsonObjectToUStruct((*RecipeObject).ToSharedRef(), UFeelRecipe::StaticClass(), Scratch, 0, FeelRecipeJsonPrivate::SkipFlags, false, &FailReason, &ImportCallback))
	{
		OutError = FText::Format(LOCTEXT("ImportFailed", "The recipe could not be read: {0}"), FailReason);
		return false;
	}

	// Copy the scratch recipe over the real one: every property, with steps duplicated into the real recipe.
	Recipe.Modify();
	for (TFieldIterator<FProperty> It(UFeelRecipe::StaticClass()); It; ++It)
	{
		if (!It->HasAnyPropertyFlags(FeelRecipeJsonPrivate::SkipFlags) && It->GetFName() != FeelRecipeJsonPrivate::BasedOnProperty)
		{
			It->CopyCompleteValue_InContainer(&Recipe, Scratch);
		}
	}
	for (FFeelTrack& Track : Recipe.Tracks)
	{
		Track.Step = Track.Step ? DuplicateObject<UFeelStep>(Track.Step, &Recipe) : nullptr;
		Track.SubstituteStep = Track.SubstituteStep ? DuplicateObject<UFeelStep>(Track.SubstituteStep, &Recipe) : nullptr;
	}

	TArray<UObject*> Subobjects;
	GetObjectsWithOuter(&Recipe, Subobjects);
	for (UObject* Subobject : Subobjects)
	{
		Subobject->SetFlags(RF_Transactional);
	}

	Recipe.SchemaVersion = UFeelRecipe::CurrentSchemaVersion;
	Recipe.PostEditChange();
	Recipe.MarkPackageDirty();
	if (OutMissingAssets)
	{
		*OutMissingAssets = MoveTemp(MissingAssets);
	}
	return true;
}

void FFeelRecipeJson::RegisterMenus()
{
	UToolMenu* Menu = UE::ContentBrowser::ExtendToolMenu_AssetContextMenu(UFeelRecipe::StaticClass());
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("GetAssetActions"));

	Section.AddDynamicEntry(TEXT("FeelRecipeJson"), FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
	{
		const UContentBrowserAssetContextMenuContext* Context = InSection.FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context)
		{
			return;
		}

		InSection.AddMenuEntry(
			TEXT("FeelRecipeExportJson"),
			LOCTEXT("ExportJson", "Export to JSON..."),
			LOCTEXT("ExportJsonTip", "Save the selected recipes as JSON text files, for review, sharing or scripted edits."),
			FSlateIcon(),
			FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& MenuContext)
			{
				const UContentBrowserAssetContextMenuContext* ExecuteContext = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
				IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
				if (!ExecuteContext || !DesktopPlatform)
				{
					return;
				}
				for (UFeelRecipe* Recipe : ExecuteContext->LoadSelectedObjects<UFeelRecipe>())
				{
					TArray<FString> Files;
					if (!DesktopPlatform->SaveFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
						FText::Format(LOCTEXT("ExportDialogTitle", "Export {0} to JSON"), FText::FromString(Recipe->GetName())).ToString(),
						FPaths::ProjectSavedDir(), Recipe->GetName() + TEXT(".json"), TEXT("JSON (*.json)|*.json"), EFileDialogFlags::None, Files) || Files.Num() == 0)
					{
						continue;
					}
					const bool bSaved = FFileHelper::SaveStringToFile(FFeelRecipeJson::Export(*Recipe), *Files[0], FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
					FeelRecipeJsonPrivate::Notify(bSaved
						? FText::Format(LOCTEXT("Exported", "Exported {0} to {1}"), FText::FromString(Recipe->GetName()), FText::FromString(Files[0]))
						: FText::Format(LOCTEXT("ExportFailed", "Could not write {0}"), FText::FromString(Files[0])), bSaved);
				}
			}));

		FToolUIAction ImportAction;
		ImportAction.ExecuteAction = FToolMenuExecuteAction::CreateLambda([](const FToolMenuContext& MenuContext)
		{
			const UContentBrowserAssetContextMenuContext* ExecuteContext = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
			const TArray<UFeelRecipe*> Recipes = ExecuteContext ? ExecuteContext->LoadSelectedObjects<UFeelRecipe>() : TArray<UFeelRecipe*>();
			if (Recipes.Num() != 1 || !DesktopPlatform)
			{
				return;
			}

			TArray<FString> Files;
			if (!DesktopPlatform->OpenFileDialog(FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
				FText::Format(LOCTEXT("ImportDialogTitle", "Import JSON into {0}"), FText::FromString(Recipes[0]->GetName())).ToString(),
				FPaths::ProjectSavedDir(), TEXT(""), TEXT("JSON (*.json)|*.json"), EFileDialogFlags::None, Files) || Files.Num() == 0)
			{
				return;
			}

			FString Json;
			FText Error;
			bool bImported = false;
			TArray<FString> MissingAssets;
			if (!FFileHelper::LoadFileToString(Json, *Files[0]))
			{
				Error = LOCTEXT("ReadFailed", "The file could not be read.");
			}
			else
			{
				FScopedTransaction Transaction(LOCTEXT("ImportTransaction", "Import Feel Recipe from JSON"));
				bImported = FFeelRecipeJson::Import(*Recipes[0], Json, Error, &MissingAssets);
				if (!bImported)
				{
					Transaction.Cancel();
				}
			}

			FText Message = Error;
			if (bImported)
			{
				Message = MissingAssets.Num() == 0
					? FText::Format(LOCTEXT("Imported", "Imported {0} into {1}. Undo reverts it."), FText::FromString(FPaths::GetCleanFilename(Files[0])), FText::FromString(Recipes[0]->GetName()))
					: FText::Format(LOCTEXT("ImportedWithMissing", "Imported {0} into {1}. {2} asset reference(s) are not in this project and were left empty: {3}"),
						FText::FromString(FPaths::GetCleanFilename(Files[0])), FText::FromString(Recipes[0]->GetName()),
						FText::AsNumber(MissingAssets.Num()), FText::FromString(FString::Join(MissingAssets, TEXT(", "))));
			}
			FeelRecipeJsonPrivate::Notify(Message, bImported);
		});
		ImportAction.CanExecuteAction = FToolMenuCanExecuteAction::CreateLambda([](const FToolMenuContext& MenuContext)
		{
			const UContentBrowserAssetContextMenuContext* CanContext = MenuContext.FindContext<UContentBrowserAssetContextMenuContext>();
			// Library recipes are read-only unless Allow Library Editing is on, so importing into them is not offered.
			return CanContext && CanContext->SelectedAssets.Num() == 1 && !(FeelLibrary::IsLibraryAsset(CanContext->SelectedAssets[0]) && !FeelLibrary::IsLibraryEditingAllowed());
		});

		InSection.AddMenuEntry(
			TEXT("FeelRecipeImportJson"),
			LOCTEXT("ImportJson", "Import from JSON..."),
			LOCTEXT("ImportJsonTip", "Replace this recipe's contents with a recipe JSON file. Asset references in the file must exist in this project. Undo reverts the import. Recipes of the FeelKit library are read-only: copy one into your project first, or turn on Editor Preferences > Plugins > FeelKit > Allow Library Editing."),
			FSlateIcon(),
			ImportAction);
	}));
}

#undef LOCTEXT_NAMESPACE
