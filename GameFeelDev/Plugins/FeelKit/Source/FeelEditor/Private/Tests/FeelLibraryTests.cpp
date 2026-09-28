// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetToolsModule.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "FeelEditorSettings.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "IAssetTools.h"
#include "Misc/NamePermissionList.h"
#include "Modules/ModuleManager.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelLibraryTests
{
	/** A recipe in a package with the given path, without writing anything to disk. */
	UFeelRecipe* MakeRecipeInPackage(const TCHAR* PackageName, const TCHAR* AssetName)
	{
		UPackage* Package = CreatePackage(PackageName);
		Package->AddToRoot();
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(Package, AssetName, RF_Public | RF_Standalone | RF_Transactional);
		return Recipe;
	}

	void DiscardPackage(UFeelRecipe* Recipe)
	{
		if (!Recipe)
		{
			return;
		}
		UPackage* Package = Recipe->GetPackage();
		Recipe->ClearFlags(RF_Public | RF_Standalone);
		Recipe->MarkAsGarbage();
		Package->RemoveFromRoot();
		Package->MarkAsGarbage();
	}

	bool IsWritable(const UFeelRecipe& Recipe)
	{
		FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		return AssetTools.Get().GetWritableFolderPermissionList()->PassesStartsWithFilter(Recipe.GetPackage()->GetName());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryReadOnlyTest, "FeelKit.Library.ReadOnly", FEEL_TEST_FLAGS)
bool FFeelLibraryReadOnlyTest::RunTest(const FString& Parameters)
{
	using namespace FeelLibraryTests;

	TestTrue(TEXT("A library path is recognized"), FeelLibrary::IsLibraryPath(TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit")));
	TestFalse(TEXT("Other plugin content is not the library"), FeelLibrary::IsLibraryPath(TEXT("/FeelKit/Samples/Sounds/S_FK_Hit_Light")));
	TestFalse(TEXT("A similarly named root is not the library"), FeelLibrary::IsLibraryPath(TEXT("/FeelKitLibrary/Impact/FR_Impact_HeavyHit")));
	TestFalse(TEXT("A project folder of the same name is not the library"), FeelLibrary::IsLibraryPath(TEXT("/Game/FeelKit/Library/FR_Impact_HeavyHit")));

	UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
	const bool bPreviousAllow = Settings->bAllowLibraryEditing;
	Settings->bAllowLibraryEditing = false;
	FeelLibrary::ApplyWritePermission();

	UFeelRecipe* LibraryRecipe = MakeRecipeInPackage(TEXT("/FeelKit/Library/Impact/FR_Impact_TestOnly"), TEXT("FR_Impact_TestOnly"));
	UFeelRecipe* ProjectRecipe = MakeRecipeInPackage(TEXT("/Game/FeelKitTests_Temp/R_TestOnly"), TEXT("R_TestOnly"));
	ON_SCOPE_EXIT
	{
		DiscardPackage(LibraryRecipe);
		DiscardPackage(ProjectRecipe);
		Settings->bAllowLibraryEditing = bPreviousAllow;
		FeelLibrary::ApplyWritePermission();
	};

	TestTrue(TEXT("A library recipe is read-only"), FeelLibrary::IsReadOnly(LibraryRecipe));
	TestFalse(TEXT("A project recipe is editable"), FeelLibrary::IsReadOnly(ProjectRecipe));
	TestFalse(TEXT("Saving the library folder is blocked"), IsWritable(*LibraryRecipe));
	TestTrue(TEXT("Saving a project folder is allowed"), IsWritable(*ProjectRecipe));

	// The Content Browser opens assets in folders that do not allow edits only for viewing. Recipes must support that,
	// otherwise double-clicking a library recipe opens nothing.
	{
		FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		const TSharedPtr<IAssetTypeActions> Actions = AssetTools.Get().GetAssetTypeActionsForClass(UFeelRecipe::StaticClass()).Pin();
		if (TestTrue(TEXT("Recipes have asset type actions"), Actions.IsValid()))
		{
			TestTrue(TEXT("Recipes can be opened for viewing"), Actions->SupportsOpenedMethod(EAssetTypeActivationOpenedMethod::View));
			TestTrue(TEXT("Recipes can be opened for editing"), Actions->SupportsOpenedMethod(EAssetTypeActivationOpenedMethod::Edit));
		}
		if (GEditor)
		{
			TestTrue(TEXT("The editor can open a library recipe for viewing"),
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CanOpenEditorForAsset(LibraryRecipe, EAssetTypeActivationOpenedMethod::View, nullptr));
		}
	}

	{
		const TSharedRef<FFeelRecipeEditorState> LibraryState = MakeShared<FFeelRecipeEditorState>(LibraryRecipe);
		TestTrue(TEXT("The editor state reports read-only"), LibraryState->IsReadOnly());
		LibraryState->AddTrack(UFeelStep_ScreenFlash::StaticClass());
		TestEqual(TEXT("Adding a track does nothing"), LibraryRecipe->Tracks.Num(), 0);

		// A track added before the protection (for example by an older build) still cannot be edited.
		FFeelTrack& Track = LibraryRecipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(LibraryRecipe);
		Track.Duration = 0.5f;
		LibraryState->HandleExternalChange();
		LibraryState->SetSelectedTrack(0);
		LibraryState->DuplicateTrack(0);
		TestEqual(TEXT("Duplicating a track does nothing"), LibraryRecipe->Tracks.Num(), 1);
		LibraryState->ToggleMute(0);
		TestTrue(TEXT("Muting does nothing"), LibraryRecipe->Tracks[0].bEnabled);
		LibraryState->DeleteTrack(0);
		TestEqual(TEXT("Deleting a track does nothing"), LibraryRecipe->Tracks.Num(), 1);
		TestFalse(TEXT("Adding a curve key does nothing"), LibraryState->AddCurveKey(0, 0.5f, 0.5f));
		TestFalse(TEXT("Pasting is not offered"), LibraryState->CanPaste());
	}

	{
		const TSharedRef<FFeelRecipeEditorState> ProjectState = MakeShared<FFeelRecipeEditorState>(ProjectRecipe);
		TestFalse(TEXT("A project recipe's state is editable"), ProjectState->IsReadOnly());
		ProjectState->AddTrack(UFeelStep_ScreenFlash::StaticClass());
		TestEqual(TEXT("Adding a track works"), ProjectRecipe->Tracks.Num(), 1);
	}

	// With the preference on, the library can be authored.
	Settings->bAllowLibraryEditing = true;
	FeelLibrary::ApplyWritePermission();
	TestFalse(TEXT("Library editing allowed clears read-only"), FeelLibrary::IsReadOnly(LibraryRecipe));
	TestTrue(TEXT("Library editing allowed clears the save block"), IsWritable(*LibraryRecipe));
	{
		const TSharedRef<FFeelRecipeEditorState> LibraryState = MakeShared<FFeelRecipeEditorState>(LibraryRecipe);
		LibraryState->AddTrack(UFeelStep_ScreenFlash::StaticClass());
		TestEqual(TEXT("Tracks can be added while authoring"), LibraryRecipe->Tracks.Num(), 2);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryTemplateCopyTest, "FeelKit.Library.TemplateCopy", FEEL_TEST_FLAGS)
bool FFeelLibraryTemplateCopyTest::RunTest(const FString& Parameters)
{
	using namespace FeelLibraryTests;

	TestEqual(TEXT("The library prefix is dropped"), FeelLibrary::SuggestCopyName(TEXT("FR_Impact_HeavyHit")), FString(TEXT("HeavyHit")));
	TestEqual(TEXT("Names with underscores keep the rest"), FeelLibrary::SuggestCopyName(TEXT("FR_Interface_ScoreTick_Big")), FString(TEXT("ScoreTick_Big")));
	TestEqual(TEXT("Other names are kept"), FeelLibrary::SuggestCopyName(TEXT("R_MyRecipe")), FString(TEXT("R_MyRecipe")));

	UFeelRecipe* Source = MakeRecipeInPackage(TEXT("/FeelKit/Library/Impact/FR_Impact_CopyTestOnly"), TEXT("FR_Impact_CopyTestOnly"));
	UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Source);
	Flash->MaxOpacity = 0.6f;
	FFeelTrack& Track = Source->Tracks.AddDefaulted_GetRef();
	Track.Step = Flash;
	Track.Duration = 0.4f;
#if WITH_EDITORONLY_DATA
	Source->Description = FText::FromString(TEXT("A sharp hit."));
#endif

	UFeelRecipe* Copy = FeelLibrary::DuplicateRecipe(*Source, TEXT("/Game/FeelKitTests_Temp"), TEXT("HeavyHitCopyTestOnly"));
	ON_SCOPE_EXIT
	{
		DiscardPackage(Source);
		DiscardPackage(Copy);
	};

	if (!TestNotNull(TEXT("The copy is created"), Copy))
	{
		return false;
	}
	TestEqual(TEXT("The copy lands in the chosen folder"), Copy->GetPackage()->GetName(), FString(TEXT("/Game/FeelKitTests_Temp/HeavyHitCopyTestOnly")));
	TestEqual(TEXT("The copy has the same tracks"), Copy->Tracks.Num(), 1);
	const UFeelStep_ScreenFlash* CopiedFlash = Copy->Tracks.Num() == 1 ? Cast<UFeelStep_ScreenFlash>(Copy->Tracks[0].Step) : nullptr;
	if (TestNotNull(TEXT("The copy has its own step"), CopiedFlash))
	{
		TestTrue(TEXT("The step is a new object inside the copy"), CopiedFlash != Flash && CopiedFlash->IsIn(Copy));
		TestEqual(TEXT("Step settings are kept"), CopiedFlash->MaxOpacity, 0.6f);
	}
#if WITH_EDITORONLY_DATA
	TestEqual(TEXT("The description is kept"), Copy->Description.ToString(), FString(TEXT("A sharp hit.")));
	TestEqual(TEXT("The copy records where it came from"), Copy->BasedOn.ToString(), FSoftObjectPath(Source).ToString());
#endif
	TestFalse(TEXT("The copy is editable"), FeelLibrary::IsReadOnly(Copy));

	// Editing the copy leaves the library recipe alone.
	Copy->Tracks[0].Duration = 1.0f;
	TestEqual(TEXT("The library recipe is unchanged"), Source->Tracks[0].Duration, 0.4f);

	return true;
}

#endif
