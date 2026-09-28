// Copyright 2026 Billo. All Rights Reserved.

#include "FeelSaveValidationLog.h"

#include "FeelRecipe.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "Presentation/MessageLogListingViewModel.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"

namespace FeelSaveValidationLogPrivate
{
	/** Message log Unreal's data validation writes to. */
	const FName AssetCheckLogName(TEXT("AssetCheck"));

	FDelegateHandle PreSaveHandle;

	void HandlePreSavePackage(UPackage* Package, FObjectPreSaveContext SaveContext)
	{
		// Unreal skips validation for procedural saves (cooking, automated resaves), so their messages are left alone.
		if (!Package || SaveContext.IsProceduralSave())
		{
			return;
		}

		ForEachObjectWithPackage(Package, [](UObject* Object)
		{
			if (Object->IsA<UFeelRecipe>())
			{
				FeelSaveValidationLog::ClearSavePage(Object->GetFName());
			}
			return true;
		});
	}
}

void FeelSaveValidationLog::Startup()
{
	FeelSaveValidationLogPrivate::PreSaveHandle = UPackage::PreSavePackageWithContextEvent.AddStatic(&FeelSaveValidationLogPrivate::HandlePreSavePackage);
}

void FeelSaveValidationLog::Shutdown()
{
	UPackage::PreSavePackageWithContextEvent.Remove(FeelSaveValidationLogPrivate::PreSaveHandle);
	FeelSaveValidationLogPrivate::PreSaveHandle.Reset();
}

FText FeelSaveValidationLog::GetSavePageTitle(FName AssetName)
{
	// Same namespace, key and source text as UEditorValidatorSubsystem::ValidateOnSave, so localized titles match too.
	return FText::Format(NSLOCTEXT("EditorValidationSubsystem", "MessageLogPageTitle.ValidateSavedAssets", "Asset Save: {0}"), FText::FromName(AssetName));
}

bool FeelSaveValidationLog::ClearSavePage(FName AssetName)
{
	FMessageLogModule* MessageLogModule = FModuleManager::GetModulePtr<FMessageLogModule>(TEXT("MessageLog"));
	if (!MessageLogModule || !MessageLogModule->IsRegisteredLogListing(FeelSaveValidationLogPrivate::AssetCheckLogName))
	{
		return false;
	}

	// The message log module hands out its listing view models behind the IMessageLogListing interface.
	const TSharedRef<FMessageLogListingViewModel> Listing = StaticCastSharedRef<FMessageLogListingViewModel>(MessageLogModule->GetLogListing(FeelSaveValidationLogPrivate::AssetCheckLogName));
	const FText PageTitle = GetSavePageTitle(AssetName);

	for (uint32 PageIndex = 0; PageIndex < Listing->GetPageCount(); ++PageIndex)
	{
		if (Listing->GetPageTitle(PageIndex).EqualTo(PageTitle))
		{
			// Clearing always empties the newest page, so bring this one to the front first.
			Listing->SetCurrentPage(PageIndex);
			Listing->ClearMessages();
			return true;
		}
	}
	return false;
}
