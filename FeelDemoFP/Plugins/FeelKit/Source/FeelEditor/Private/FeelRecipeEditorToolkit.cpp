// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeEditorToolkit.h"

#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelRecipeDetails.h"
#include "FeelRecipeEditorState.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SFeelIntensityGraph.h"
#include "SFeelPreviewViewport.h"
#include "SFeelTimeline.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

namespace FeelRecipeEditorTabs
{
	static const FName AppIdentifier(TEXT("FeelRecipeEditorApp"));
	static const FName Viewport(TEXT("FeelRecipeEditor_Viewport"));
	static const FName Timeline(TEXT("FeelRecipeEditor_Timeline"));
	static const FName Intensity(TEXT("FeelRecipeEditor_Intensity"));
	static const FName Details(TEXT("FeelRecipeEditor_Details"));
}

void FFeelRecipeEditorToolkit::InitRecipeEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UFeelRecipe* Recipe)
{
	State = MakeShared<FFeelRecipeEditorState>(Recipe);
	FFeelRecipeEditorState::Register(State.ToSharedRef());
	State->OnSelectionChanged.AddSP(this, &FFeelRecipeEditorToolkit::RefreshDetails);
	State->OnTracksChanged.AddSP(this, &FFeelRecipeEditorToolkit::RefreshDetails);

	FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsView = PropertyEditor.CreateDetailView(DetailsViewArgs);

	FFeelRecipeDetails::RegisterLayouts(*DetailsView, State);
	// Library recipes ship with FeelKit and are shared by every project on this engine, so their details are read-only.
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	DetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateLambda([WeakState]()
	{
		const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
		return !PinnedState.IsValid() || !PinnedState->IsReadOnly();
	}));
	DetailsView->SetObject(Recipe);

	Viewport = SNew(SFeelPreviewViewport, State.ToSharedRef());
	Timeline = SNew(SFeelTimeline, State.ToSharedRef());
	IntensityGraph = SNew(SFeelIntensityGraph, State.ToSharedRef());

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TEXT("Standalone_FeelRecipeEditor_Layout_v2"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)->SetSizeCoefficient(0.72f)
				->Split
				(
					FTabManager::NewStack()->SetSizeCoefficient(0.5f)
					->AddTab(FeelRecipeEditorTabs::Viewport, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()->SetSizeCoefficient(0.35f)
					->AddTab(FeelRecipeEditorTabs::Timeline, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()->SetSizeCoefficient(0.15f)
					->AddTab(FeelRecipeEditorTabs::Intensity, ETabState::OpenedTab)
				)
			)
			->Split
			(
				FTabManager::NewStack()->SetSizeCoefficient(0.28f)
				->AddTab(FeelRecipeEditorTabs::Details, ETabState::OpenedTab)
			)
		);

	// Always the normal open method. The Content Browser asks for Unreal's view-only mode for library recipes, which
	// disables every panel, including the preview and the Copy to Project button. Library recipes are protected by
	// FeelKit's own read-only rules instead (FeelLibrary::IsReadOnly), and saving them is refused by the folder permission.
	// The array overload is used on purpose: in UE 5.6 the single-object overload drops the open method.
	const TArray<UObject*> ObjectsToEdit = { Recipe };
	InitAssetEditor(Mode, InitToolkitHost, FeelRecipeEditorTabs::AppIdentifier, Layout, true, true, ObjectsToEdit, false, false, EAssetOpenMethod::Edit);
}

void FFeelRecipeEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu", "Feel Recipe Editor"));
	const TSharedRef<FWorkspaceItem> Group = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FeelRecipeEditorTabs::Viewport, FOnSpawnTab::CreateSP(this, &FFeelRecipeEditorToolkit::SpawnViewportTab))
		.SetDisplayName(LOCTEXT("ViewportTab", "Preview"))
		.SetGroup(Group);

	InTabManager->RegisterTabSpawner(FeelRecipeEditorTabs::Timeline, FOnSpawnTab::CreateSP(this, &FFeelRecipeEditorToolkit::SpawnTimelineTab))
		.SetDisplayName(LOCTEXT("TimelineTab", "Timeline"))
		.SetGroup(Group);

	InTabManager->RegisterTabSpawner(FeelRecipeEditorTabs::Intensity, FOnSpawnTab::CreateSP(this, &FFeelRecipeEditorToolkit::SpawnIntensityTab))
		.SetDisplayName(LOCTEXT("IntensityTab", "Intensity"))
		.SetGroup(Group);

	InTabManager->RegisterTabSpawner(FeelRecipeEditorTabs::Details, FOnSpawnTab::CreateSP(this, &FFeelRecipeEditorToolkit::SpawnDetailsTab))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(Group);
}

void FFeelRecipeEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FeelRecipeEditorTabs::Viewport);
	InTabManager->UnregisterTabSpawner(FeelRecipeEditorTabs::Timeline);
	InTabManager->UnregisterTabSpawner(FeelRecipeEditorTabs::Intensity);
	InTabManager->UnregisterTabSpawner(FeelRecipeEditorTabs::Details);
}

FName FFeelRecipeEditorToolkit::GetToolkitFName() const
{
	return TEXT("FeelRecipeEditor");
}

FText FFeelRecipeEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "Feel Recipe Editor");
}

FString FFeelRecipeEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("FeelRecipe ");
}

FLinearColor FFeelRecipeEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(1.0f, 0.45f, 0.1f, 0.5f);
}

void FFeelRecipeEditorToolkit::PostUndo(bool bSuccess)
{
	if (State.IsValid())
	{
		State->HandleExternalChange();
	}
}

void FFeelRecipeEditorToolkit::PostRedo(bool bSuccess)
{
	PostUndo(bSuccess);
}

void FFeelRecipeEditorToolkit::Tick(float DeltaTime)
{
	if (State.IsValid())
	{
		State->Tick(DeltaTime);
	}
}

ETickableTickType FFeelRecipeEditorToolkit::GetTickableTickType() const
{
	return ETickableTickType::Always;
}

TStatId FFeelRecipeEditorToolkit::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FFeelRecipeEditorToolkit, STATGROUP_Tickables);
}

TSharedRef<SDockTab> FFeelRecipeEditorToolkit::SpawnViewportTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab", "Preview"))
		[
			Viewport.ToSharedRef()
		];
}

TSharedRef<SDockTab> FFeelRecipeEditorToolkit::SpawnTimelineTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("TimelineTab", "Timeline"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				MakeReadOnlyBanner()
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				Timeline.ToSharedRef()
			]
		];
}

TSharedRef<SWidget> FFeelRecipeEditorToolkit::MakeReadOnlyBanner()
{
	const TWeakPtr<FFeelRecipeEditorState> WeakState = State;
	// Same outer padding as the timeline toolbar below it, so the text lines up with + Track.
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Header")))
		.Padding(FMargin(8.0f, 4.0f, 4.0f, 4.0f))
		.Visibility_Lambda([WeakState]()
		{
			const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
			return PinnedState.IsValid() && PinnedState->IsReadOnly() ? EVisibility::Visible : EVisibility::Collapsed;
		})
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.Lock")))
				.ColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.35f))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ReadOnlyBanner", "Library recipe (read-only). Copy it to your project to edit."))
				.ToolTipText(LOCTEXT("ReadOnlyBannerTip", "Recipes under /FeelKit/Library ship with the plugin and are shared by every project on this engine. Copy one into your project and edit the copy, so a plugin update never changes your game. To author the library itself, turn on Editor Preferences > Plugins > FeelKit > Allow Library Editing."))
				.ColorAndOpacity(FLinearColor(1.0f, 0.8f, 0.35f))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), TEXT("PrimaryButton"))
				.Text(LOCTEXT("CopyToProjectButton", "Copy to Project"))
				.ToolTipText(LOCTEXT("CopyToProjectTip", "Create an editable copy of this recipe in your project and open it."))
				.OnClicked_Lambda([WeakState]()
				{
					const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
					if (const UFeelRecipe* Recipe = PinnedState.IsValid() ? PinnedState->GetRecipe() : nullptr)
					{
						FeelLibrary::CopyToProjectWithDialog(*Recipe, FeelLibrary::GetDefaultCopyFolder());
					}
					return FReply::Handled();
				})
			]
		];
}

TSharedRef<SDockTab> FFeelRecipeEditorToolkit::SpawnIntensityTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("IntensityTab", "Intensity"))
		[
			IntensityGraph.ToSharedRef()
		];
}

TSharedRef<SDockTab> FFeelRecipeEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

void FFeelRecipeEditorToolkit::RefreshDetails()
{
	if (DetailsView.IsValid())
	{
		DetailsView->ForceRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
