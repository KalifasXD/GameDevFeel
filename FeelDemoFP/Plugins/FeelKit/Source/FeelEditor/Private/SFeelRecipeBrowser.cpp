// Copyright 2026 Billo. All Rights Reserved.

#include "SFeelRecipeBrowser.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetThumbnail.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "FeelEditorColors.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelTags.h"
#include "GameplayTagsManager.h"
#include "FeelRecipeEditorState.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IContentBrowserSingleton.h"
#include "Modules/ModuleManager.h"
#include "SFeelPreviewViewport.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STileView.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FeelRecipeBrowser"

const FName SFeelRecipeBrowser::TabName(TEXT("FeelKitRecipeBrowser"));

namespace FeelRecipeBrowserPrivate
{
	/** Time the cursor must rest on a tile before its recipe starts playing. */
	constexpr float HoverDelaySeconds = 0.25f;

	/** Tile size of the Content Browser's default (medium) grid. */
	constexpr uint32 ThumbnailSize = 96;
	constexpr float TileWidth = 108.0f;
	constexpr float TileHeight = 160.0f;

	FText FeelingLabel(const FName& Feeling)
	{
		if (Feeling.IsNone())
		{
			return LOCTEXT("NoFeeling", "No feeling");
		}
		FString Name = Feeling.ToString();
		int32 LastDot = INDEX_NONE;
		return Name.FindLastChar(TEXT('.'), LastDot) ? FText::FromString(Name.RightChop(LastDot + 1)) : FText::FromString(Name);
	}

	/** Every tag under a root, so projects can add their own feelings and genres. */
	TArray<FName> TagsUnder(const TCHAR* Root)
	{
		TArray<FName> Names;
		FGameplayTagContainer Children = UGameplayTagsManager::Get().RequestGameplayTagChildren(FGameplayTag::RequestGameplayTag(FName(Root), false));
		for (const FGameplayTag& Tag : Children)
		{
			Names.Add(Tag.GetTagName());
		}
		Names.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });
		return Names;
	}
}

void SFeelRecipeBrowser::Construct(const FArguments& InArgs)
{
	bPickerMode = InArgs._PickerMode;
	OnRecipePicked = InArgs._OnRecipePicked;
	if (bPickerMode)
	{
		Filter.Source = EFeelRecipeSource::Library;
	}

	PreviewState = MakeShared<FFeelRecipeEditorState>(nullptr);
	ThumbnailPool = MakeShared<FAssetThumbnailPool>(128);

	TSharedRef<STileView<TSharedPtr<FFeelRecipeEntry>>> Tiles = SNew(STileView<TSharedPtr<FFeelRecipeEntry>>)
		.ListItemsSource(&VisibleEntries)
		.OnGenerateTile(this, &SFeelRecipeBrowser::MakeTile)
		.OnSelectionChanged(this, &SFeelRecipeBrowser::OnSelectionChanged)
		.OnMouseButtonDoubleClick(this, &SFeelRecipeBrowser::OnRowDoubleClicked)
		.SelectionMode(ESelectionMode::Single)
		.ItemWidth(FeelRecipeBrowserPrivate::TileWidth)
		.ItemHeight(FeelRecipeBrowserPrivate::TileHeight)
		.ItemAlignment(EListItemAlignment::LeftAligned);
	ListView = Tiles;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
		.Padding(0.0f)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(2.0f)
			+ SSplitter::Slot()
			.Value(0.2f)
			[
				SAssignNew(FilterBox, SBox)
				[
					MakeFilterColumn()
				]
			]
			+ SSplitter::Slot()
			.Value(0.45f)
			[
				SNew(SVerticalBox)
				// Search row, as in the Content Browser.
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(8.0f, 6.0f)
				[
					SAssignNew(SearchBox, SSearchBox)
					.HintText(LOCTEXT("SearchHint", "Search Recipes"))
					.ToolTipText(LOCTEXT("SearchTip", "Search names, descriptions and parameter names."))
					.OnTextChanged_Lambda([this](const FText& Text)
					{
						Filter.Search = Text.ToString();
						RefreshList();
					})
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(8.0f, 0.0f, 0.0f, 0.0f)
				[
					Tiles
				]
				// Item count, as at the bottom of the Content Browser.
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(10.0f, 6.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return GetSelectedEntry().IsValid()
							? FText::Format(LOCTEXT("ItemCountSelected", "{0} items (1 selected)"), FText::AsNumber(VisibleEntries.Num()))
							: FText::Format(LOCTEXT("ItemCount", "{0} items"), FText::AsNumber(VisibleEntries.Num()));
					})
				]
			]
			+ SSplitter::Slot()
			.Value(0.35f)
			[
				MakePreviewColumn()
			]
		]
	];

	FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetAddedHandle = AssetRegistry.Get().OnAssetAdded().AddSP(this, &SFeelRecipeBrowser::HandleAssetRegistryChange);
	AssetRemovedHandle = AssetRegistry.Get().OnAssetRemoved().AddSP(this, &SFeelRecipeBrowser::HandleAssetRegistryChange);
	AssetUpdatedHandle = AssetRegistry.Get().OnAssetUpdated().AddSP(this, &SFeelRecipeBrowser::HandleAssetRegistryChange);

	RefreshEntries();
	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SFeelRecipeBrowser::TickPreview));
}

SFeelRecipeBrowser::~SFeelRecipeBrowser()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
	{
		IAssetRegistry& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		AssetRegistry.OnAssetAdded().Remove(AssetAddedHandle);
		AssetRegistry.OnAssetRemoved().Remove(AssetRemovedHandle);
		AssetRegistry.OnAssetUpdated().Remove(AssetUpdatedHandle);
	}
}

void SFeelRecipeBrowser::HandleAssetRegistryChange(const FAssetData& Asset)
{
	if (Asset.AssetClassPath == UFeelRecipe::StaticClass()->GetClassPathName())
	{
		RefreshEntries();
	}
}

TSharedRef<SWidget> SFeelRecipeBrowser::MakeFilterColumn()
{
	using namespace FeelRecipeBrowserPrivate;

	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	// A collapsible section with a bold header, as the Content Browser's left panel has.
	auto Section = [&Column](const FText& Title, const FText& Note, const FText& NoteTip, const TSharedRef<SWidget>& Body)
	{
		Column->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 1.0f)
		[
			SNew(SExpandableArea)
			.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Header")))
			.BodyBorderImage(FAppStyle::GetBrush(TEXT("Brushes.Recessed")))
			.HeaderPadding(FMargin(6.0f, 5.0f))
			.Padding(FMargin(0.0f, 4.0f))
			.HeaderContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Title)
					.Font(FAppStyle::GetFontStyle(TEXT("NormalFontBold")))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Note)
					.ToolTipText(NoteTip)
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
			.BodyContent()
			[
				Body
			]
		];
	};

	// One filter entry: a check box, a color swatch when the value has a color, the label and a count on the right.
	auto Entry = [](const FText& Label, TOptional<FLinearColor> Swatch, TFunction<bool()> IsChecked, TFunction<void(bool)> SetChecked, TFunction<FText()> Count)
	{
		return SNew(SCheckBox)
			.Padding(FMargin(4.0f, 1.0f))
			.IsChecked_Lambda([IsChecked]() { return IsChecked() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([SetChecked](ECheckBoxState NewState) { SetChecked(NewState == ECheckBoxState::Checked); })
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(3.0f)
					.HeightOverride(14.0f)
					.Visibility(Swatch.IsSet() ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(TEXT("WhiteBrush")))
						.ColorAndOpacity(Swatch.Get(FLinearColor::White))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(Label)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(6.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text_Lambda([Count]() { return Count(); })
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			];
	};

	if (!bPickerMode)
	{
		Section(LOCTEXT("SourceHeading", "Source"), FText::GetEmpty(), FText::GetEmpty(),
			SNew(SBox)
			.Padding(FMargin(8.0f, 2.0f))
			[
				SNew(SSegmentedControl<EFeelRecipeSource>)
				.Value_Lambda([this]() { return Filter.Source; })
				.OnValueChanged_Lambda([this](EFeelRecipeSource NewSource)
				{
					Filter.Source = NewSource;
					RefreshList();
				})
				+ SSegmentedControl<EFeelRecipeSource>::Slot(EFeelRecipeSource::Both)
				.Text(LOCTEXT("SourceBoth", "All"))
				.ToolTip(LOCTEXT("SourceBothTip", "Recipes that ship with FeelKit and recipes in this project."))
				+ SSegmentedControl<EFeelRecipeSource>::Slot(EFeelRecipeSource::Library)
				.Text(LOCTEXT("SourceLibrary", "Library"))
				.ToolTip(LOCTEXT("SourceLibraryTip", "Only the recipes that ship with FeelKit."))
				+ SSegmentedControl<EFeelRecipeSource>::Slot(EFeelRecipeSource::Project)
				.Text(LOCTEXT("SourceProject", "Project"))
				.ToolTip(LOCTEXT("SourceProjectTip", "Only the recipes in this project."))
			]);
	}

	TSharedRef<SVerticalBox> Feelings = SNew(SVerticalBox);
	for (const FName& Feeling : TagsUnder(FeelTags::FeelingRoot))
	{
		Feelings->AddSlot().AutoHeight().Padding(4.0f, 0.0f)
		[
			Entry(FeelingLabel(Feeling), FeelEditorColors::GetFeelingColor(FGameplayTag::RequestGameplayTag(Feeling, false)),
				[this, Feeling]() { return Filter.Feelings.Contains(Feeling); },
				[this, Feeling](bool bChecked) { bChecked ? (void)Filter.Feelings.Add(Feeling) : (void)Filter.Feelings.Remove(Feeling); RefreshList(); },
				[this, Feeling]() { return FText::AsNumber(FeelingCounts.FindRef(Feeling)); })
		];
	}
	Section(LOCTEXT("FeelingHeading", "Feeling"), LOCTEXT("FeelingHeadingAny", "any of them"),
		LOCTEXT("FeelingHeadingAnyTip", "Picking two or more shows recipes with any of them."), Feelings);

	TSharedRef<SVerticalBox> Genres = SNew(SVerticalBox);
	for (const FName& Genre : TagsUnder(FeelTags::GenreRoot))
	{
		Genres->AddSlot().AutoHeight().Padding(4.0f, 0.0f)
		[
			Entry(FeelingLabel(Genre), TOptional<FLinearColor>(),
				[this, Genre]() { return Filter.Genres.Contains(Genre); },
				[this, Genre](bool bChecked) { bChecked ? (void)Filter.Genres.Add(Genre) : (void)Filter.Genres.Remove(Genre); RefreshList(); },
				[]() { return FText::GetEmpty(); })
		];
	}
	Section(LOCTEXT("GenreHeading", "Genre"), LOCTEXT("GenreHeadingAny", "any of them"),
		LOCTEXT("GenreHeadingAnyTip", "Picking two or more shows recipes with any of them."), Genres);

	TSharedRef<SVerticalBox> Channels = SNew(SVerticalBox);
	for (const FName& Group : FeelRecipeFilter::ChannelGroups)
	{
		Channels->AddSlot().AutoHeight().Padding(4.0f, 0.0f)
		[
			Entry(FeelingLabel(Group), FeelEditorColors::GetChannelAccent(FGameplayTag::RequestGameplayTag(Group, false)),
				[this, Group]() { return Filter.ChannelGroups.Contains(Group); },
				[this, Group](bool bChecked) { bChecked ? (void)Filter.ChannelGroups.Add(Group) : (void)Filter.ChannelGroups.Remove(Group); RefreshList(); },
				[]() { return FText::GetEmpty(); })
		];
	}
	Section(LOCTEXT("ChannelHeading", "Affects"), LOCTEXT("ChannelHeadingAll", "all of them"),
		LOCTEXT("ChannelHeadingAllTip", "Picking two or more shows only recipes that do all of them, for example shake and rumble."), Channels);

	Column->AddSlot()
	.AutoHeight()
	.Padding(8.0f, 8.0f)
	.HAlign(HAlign_Left)
	[
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), TEXT("SimpleButton"))
		.ToolTipText(LOCTEXT("ClearFiltersTip", "Show every recipe again."))
		.OnClicked_Lambda([this]()
		{
			const EFeelRecipeSource Source = bPickerMode ? EFeelRecipeSource::Library : EFeelRecipeSource::Both;
			Filter = FFeelRecipeFilterState();
			Filter.Source = Source;
			if (SearchBox.IsValid())
			{
				SearchBox->SetText(FText::GetEmpty());
			}
			RefreshList();
			return FReply::Handled();
		})
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.X")))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(LOCTEXT("ClearFilters", "Clear Filters"))
			]
		]
	];

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Recessed")))
		.Padding(0.0f)
		[
			SNew(SScrollBox) + SScrollBox::Slot()[Column]
		];
}

TSharedRef<SWidget> SFeelRecipeBrowser::MakePreviewColumn()
{
	PreviewViewport = SNew(SFeelPreviewViewport, PreviewState.ToSharedRef());

	// A category header and label / value rows, as in the Details panel.
	auto CategoryHeader = [](const FText& Title)
	{
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Header")))
			.Padding(FMargin(8.0f, 5.0f))
			[
				SNew(STextBlock)
				.Text(Title)
				.Font(FAppStyle::GetFontStyle(TEXT("NormalFontBold")))
			];
	};
	auto Property = [this](const FText& Label, TFunction<FText(const FFeelRecipeEntry&)> Value)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.35f)
			.VAlign(VAlign_Top)
			.Padding(FMargin(12.0f, 3.0f, 6.0f, 3.0f))
			[
				SNew(STextBlock)
				.Text(Label)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.65f)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.0f, 3.0f, 8.0f, 3.0f))
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text_Lambda([this, Value]()
				{
					const TSharedPtr<FFeelRecipeEntry> Entry = GetSelectedEntry();
					return Entry.IsValid() ? Value(*Entry) : FText::GetEmpty();
				})
			];
	};
	auto JoinNames = [](const TArray<FName>& Names)
	{
		TArray<FString> Parts;
		for (const FName& Name : Names)
		{
			Parts.Add(FeelRecipeBrowserPrivate::FeelingLabel(Name).ToString());
		}
		return Parts.Num() > 0 ? FText::FromString(FString::Join(Parts, TEXT(", "))) : LOCTEXT("None", "-");
	};

	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
	.Padding(0.0f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(PreviewBox, SBox)
			[
				PreviewViewport.ToSharedRef()
			]
		]
		// Preview options, as a small toolbar under the viewport.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(8.0f, 6.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]() { return bMuted ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
				{
					bMuted = NewState == ECheckBoxState::Checked;
					if (PreviewViewport.IsValid())
					{
						PreviewViewport->SetAudioMuted(bMuted);
					}
				})
				.ToolTipText(LOCTEXT("MuteTip", "Stop the preview from playing the recipe's sounds."))
				[
					SNew(STextBlock).Text(LOCTEXT("Mute", "Mute"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(12.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SComboButton)
				.ToolTipText(LOCTEXT("ComfortTip", "Preview the recipe as a player with these comfort settings would feel it."))
				.OnGetMenuContent(this, &SFeelRecipeBrowser::MakeComfortMenu)
				.ButtonContent()
				[
					SNew(STextBlock).Text(this, &SFeelRecipeBrowser::GetComfortText)
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(8.0f, 0.0f, 8.0f, 4.0f))
		[
			SAssignNew(ParameterSliderBox, SBox)
		]
		// What the selected recipe is.
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CategoryHeader(LOCTEXT("RecipeCategory", "Recipe"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Visibility_Lambda([this]() { return GetSelectedEntry().IsValid() ? EVisibility::Collapsed : EVisibility::Visible; })
			.Padding(FMargin(12.0f, 6.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NoSelection", "Select a recipe to see what it does."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([this]() { return GetSelectedEntry().IsValid() ? EVisibility::Visible : EVisibility::Collapsed; })
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("NameLabel", "Name"), [](const FFeelRecipeEntry& Entry) { return FText::FromString(Entry.GetName()); })]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("DescriptionLabel", "Description"), [](const FFeelRecipeEntry& Entry) { return FText::FromString(Entry.Description); })]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("FeelingLabel", "Feeling"), [](const FFeelRecipeEntry& Entry) { return FeelRecipeBrowserPrivate::FeelingLabel(Entry.Feeling); })]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("GenresLabel", "Genres"), [JoinNames](const FFeelRecipeEntry& Entry) { return JoinNames(Entry.Genres); })]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("AffectsLabel", "Affects"), [](const FFeelRecipeEntry& Entry)
			{
				TArray<FString> Parts;
				for (const FName& Channel : Entry.Channels)
				{
					Parts.Add(Channel.ToString().Replace(TEXT("Feel."), TEXT("")));
				}
				return Parts.Num() > 0 ? FText::FromString(FString::Join(Parts, TEXT(", "))) : LOCTEXT("None", "-");
			})]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("TracksLabel", "Tracks"), [](const FFeelRecipeEntry& Entry) { return FText::AsNumber(Entry.TrackCount); })]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("LengthLabel", "Length"), [](const FFeelRecipeEntry& Entry)
			{
				return Entry.bSustained
					? FText::Format(LOCTEXT("LengthSustained", "{0} s, sustained until released"), FText::AsNumber(Entry.Length))
					: FText::Format(LOCTEXT("LengthValue", "{0} s"), FText::AsNumber(Entry.Length));
			})]
			+ SVerticalBox::Slot().AutoHeight()[Property(LOCTEXT("SourceLabel", "Source"), [](const FFeelRecipeEntry& Entry)
			{
				return Entry.bLibrary ? LOCTEXT("SourceLibraryValue", "FeelKit library (read-only)") : LOCTEXT("SourceProjectValue", "This project");
			})]
		]
		// Actions, the main one in Unreal's primary (blue) style.
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(8.0f, 8.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), TEXT("PrimaryButton"))
				.Text_Lambda([this]()
				{
					const TSharedPtr<FFeelRecipeEntry> Entry = GetSelectedEntry();
					return bPickerMode ? LOCTEXT("CreateButton", "Create") : (Entry.IsValid() && !Entry->bLibrary ? LOCTEXT("OpenOwnButton", "Edit") : LOCTEXT("UseButton", "Use"));
				})
				.ToolTipText(LOCTEXT("UseTip", "Library recipe: copy it into your project, where you can edit it, and open the copy. Project recipe: open it."))
				.IsEnabled_Lambda([this]() { return GetSelectedEntry().IsValid(); })
				.OnClicked_Lambda([this]() { UseSelected(); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenButton", "Open"))
				.ToolTipText(LOCTEXT("OpenTip", "Open this recipe in the recipe editor. Library recipes open read-only."))
				.Visibility(bPickerMode ? EVisibility::Collapsed : EVisibility::Visible)
				.IsEnabled_Lambda([this]() { return GetSelectedEntry().IsValid(); })
				.OnClicked_Lambda([this]() { OpenSelected(); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ToolTipText(LOCTEXT("ShowTip", "Show this recipe in the Content Browser."))
				.Visibility(bPickerMode ? EVisibility::Collapsed : EVisibility::Visible)
				.IsEnabled_Lambda([this]() { return GetSelectedEntry().IsValid(); })
				.OnClicked_Lambda([this]() { ShowSelectedInContentBrowser(); return FReply::Handled(); })
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 4.0f, 0.0f)
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(TEXT("Icons.BrowseContent")))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock).Text(LOCTEXT("ShowButton", "Show in Content Browser"))
					]
				]
			]
		]
	];
}

TSharedRef<ITableRow> SFeelRecipeBrowser::MakeTile(TSharedPtr<FFeelRecipeEntry> Entry, const TSharedRef<STableViewBase>& OwnerTable)
{
	using namespace FeelRecipeBrowserPrivate;

	// The same tile the Content Browser draws: the recipe's thumbnail over its asset color line, then the name and type.
	TSharedPtr<FAssetThumbnail>& Thumbnail = Thumbnails.FindOrAdd(Entry->Asset.PackageName);
	if (!Thumbnail.IsValid())
	{
		Thumbnail = MakeShared<FAssetThumbnail>(Entry->Asset, ThumbnailSize, ThumbnailSize, ThumbnailPool);
	}
	FAssetThumbnailConfig ThumbnailConfig;
	ThumbnailConfig.bAllowFadeIn = true;
	ThumbnailConfig.ColorStripOrientation = EThumbnailColorStripOrientation::HorizontalBottomEdge;

	TSharedRef<STableRow<TSharedPtr<FFeelRecipeEntry>>> Row = SNew(STableRow<TSharedPtr<FFeelRecipeEntry>>, OwnerTable)
		.Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("ContentBrowser.AssetListView.TileTableRow")))
		.Padding(FMargin(3.0f))
		.ToolTipText(FText::FromString(Entry->Description));

	const TWeakPtr<STableRow<TSharedPtr<FFeelRecipeEntry>>> WeakRow = Row;
	auto NameAreaBrush = [WeakRow]()
	{
		const TSharedPtr<STableRow<TSharedPtr<FFeelRecipeEntry>>> PinnedRow = WeakRow.Pin();
		const bool bSelected = PinnedRow.IsValid() && PinnedRow->IsItemSelected();
		const bool bHovered = PinnedRow.IsValid() && PinnedRow->IsHovered();
		return FAppStyle::GetBrush(bSelected
			? (bHovered ? TEXT("ContentBrowser.AssetTileItem.NameAreaSelectedHoverBackground") : TEXT("ContentBrowser.AssetTileItem.NameAreaSelectedBackground"))
			: (bHovered ? TEXT("ContentBrowser.AssetTileItem.NameAreaHoverBackground") : TEXT("ContentBrowser.AssetTileItem.NameAreaBackground")));
	};

	Row->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush(TEXT("ContentBrowser.AssetTileItem.ThumbnailAreaBackground")))
			.Padding(0.0f)
			[
				SNew(SBox)
				.WidthOverride(static_cast<float>(ThumbnailSize))
				.HeightOverride(static_cast<float>(ThumbnailSize))
				[
					Thumbnail->MakeThumbnailWidget(ThumbnailConfig)
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SBorder)
			.BorderImage_Lambda(NameAreaBrush)
			.Padding(FMargin(5.0f, 4.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Entry->GetName()))
					.Font(FAppStyle::GetFontStyle(TEXT("ContentBrowser.AssetTileViewNameFont")))
					.WrapTextAt(static_cast<float>(ThumbnailSize) - 10.0f)
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(Entry->bLibrary ? LOCTEXT("TileTypeLibrary", "Feel Recipe (Library)") : LOCTEXT("TileType", "Feel Recipe"))
					.Font(FAppStyle::GetFontStyle(TEXT("ContentBrowser.AssetTileViewClassNameFont")))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]
		]);
	return Row;
}

void SFeelRecipeBrowser::SelectRecipe(const FSoftObjectPath& RecipePath)
{
	auto Find = [this, &RecipePath]() -> TSharedPtr<FFeelRecipeEntry>
	{
		for (const TSharedPtr<FFeelRecipeEntry>& Entry : VisibleEntries)
		{
			if (Entry->Asset.GetSoftObjectPath() == RecipePath)
			{
				return Entry;
			}
		}
		return nullptr;
	};

	TSharedPtr<FFeelRecipeEntry> Entry = Find();
	if (!Entry.IsValid())
	{
		const EFeelRecipeSource Source = bPickerMode ? EFeelRecipeSource::Library : EFeelRecipeSource::Both;
		Filter = FFeelRecipeFilterState();
		Filter.Source = Source;
		RefreshList();
		Entry = Find();
	}
	if (Entry.IsValid() && ListView.IsValid())
	{
		ListView->SetSelection(Entry, ESelectInfo::Direct);
		ListView->RequestScrollIntoView(Entry);
	}
}

void SFeelRecipeBrowser::RefreshEntries()
{
	AllEntries = FeelRecipeFilter::GatherAll();

	// Recipes are small. Loaded recipes get their thumbnail drawn live, so the tiles always show the current recipe
	// instead of the picture saved inside the asset when it was last saved.
	for (const FFeelRecipeEntry& Entry : AllEntries)
	{
		Entry.Asset.GetAsset();
	}
	RefreshList();
}

void SFeelRecipeBrowser::RefreshList()
{
	const TSharedPtr<FFeelRecipeEntry> Selected = GetSelectedEntry();
	const FName SelectedName = Selected.IsValid() ? Selected->Asset.PackageName : NAME_None;

	VisibleEntries.Reset();
	for (const FFeelRecipeEntry& Entry : FeelRecipeFilter::Apply(AllEntries, Filter))
	{
		VisibleEntries.Add(MakeShared<FFeelRecipeEntry>(Entry));
	}
	FeelingCounts = FeelRecipeFilter::CountByFeeling(AllEntries, Filter);

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
		if (!SelectedName.IsNone())
		{
			for (const TSharedPtr<FFeelRecipeEntry>& Entry : VisibleEntries)
			{
				if (Entry->Asset.PackageName == SelectedName)
				{
					ListView->SetSelection(Entry, ESelectInfo::Direct);
					break;
				}
			}
		}
	}
}

TSharedPtr<FFeelRecipeEntry> SFeelRecipeBrowser::GetSelectedEntry() const
{
	if (!ListView.IsValid())
	{
		return nullptr;
	}
	const TArray<TSharedPtr<FFeelRecipeEntry>> Selection = ListView->GetSelectedItems();
	return Selection.Num() > 0 ? Selection[0] : nullptr;
}

UFeelRecipe* SFeelRecipeBrowser::GetSelectedRecipe() const
{
	const TSharedPtr<FFeelRecipeEntry> Entry = GetSelectedEntry();
	return Entry.IsValid() ? Cast<UFeelRecipe>(Entry->Asset.GetAsset()) : nullptr;
}

void SFeelRecipeBrowser::OnSelectionChanged(TSharedPtr<FFeelRecipeEntry> Entry, ESelectInfo::Type SelectInfo)
{
	UFeelRecipe* Recipe = Entry.IsValid() ? Cast<UFeelRecipe>(Entry->Asset.GetAsset()) : nullptr;
	SetPreviewRecipe(Recipe, true);
	RebuildParameterSliders();
}

/** The row the cursor is on, or none. Hovering is read from the widgets instead of per-row callbacks. */
TSharedPtr<FFeelRecipeEntry> SFeelRecipeBrowser::FindHoveredEntry() const
{
	if (!ListView.IsValid())
	{
		return nullptr;
	}
	for (const TSharedPtr<FFeelRecipeEntry>& Entry : VisibleEntries)
	{
		const TSharedPtr<ITableRow> Row = ListView->WidgetFromItem(Entry);
		if (Row.IsValid() && Row->AsWidget()->IsHovered())
		{
			return Entry;
		}
	}
	return nullptr;
}

void SFeelRecipeBrowser::OnRowDoubleClicked(TSharedPtr<FFeelRecipeEntry> Entry)
{
	ListView->SetSelection(Entry, ESelectInfo::Direct);
	if (Entry.IsValid() && (Entry->bLibrary || bPickerMode))
	{
		UseSelected();
	}
	else
	{
		OpenSelected();
	}
}

void SFeelRecipeBrowser::SetPreviewRecipe(UFeelRecipe* Recipe, bool bLoop)
{
	if (!PreviewState.IsValid() || (PreviewRecipe.Get() == Recipe && bLoopPreview == bLoop))
	{
		return;
	}

	PreviewRecipe = Recipe;
	bLoopPreview = bLoop;

	// The preview state belongs to one recipe, so a new recipe needs a new state and viewport.
	PreviewState = MakeShared<FFeelRecipeEditorState>(Recipe);
	PreviewState->SetLooping(bLoop);
	PreviewViewport = SNew(SFeelPreviewViewport, PreviewState.ToSharedRef());
	PreviewViewport->SetAudioMuted(bMuted);
	if (PreviewBox.IsValid())
	{
		PreviewBox->SetContent(PreviewViewport.ToSharedRef());
	}

	if (Recipe)
	{
		PreviewState->SetTime(0.0f);
		PreviewState->TogglePlay();
	}
}

void SFeelRecipeBrowser::RebuildParameterSliders()
{
	if (!ParameterSliderBox.IsValid())
	{
		return;
	}

	UFeelRecipe* Recipe = PreviewRecipe.Get();
	if (!Recipe || Recipe->Parameters.Num() == 0)
	{
		ParameterSliderBox->SetContent(SNullWidget::NullWidget);
		return;
	}

	const TWeakPtr<FFeelRecipeEditorState> WeakState = PreviewState;
	TSharedRef<SWrapBox> Sliders = SNew(SWrapBox).UseAllottedSize(true);
	for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
	{
		if (Parameter.Name.IsNone() || Parameter.MaxValue <= Parameter.MinValue)
		{
			continue;
		}

		const FName ParameterName = Parameter.Name;
		Sliders->AddSlot()
		.Padding(0.0f, 2.0f, 12.0f, 2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox)
			.ToolTipText(Parameter.Description.IsEmpty()
				? FText::Format(LOCTEXT("BrowserParameterTip", "{0}: {1} to {2}."), FText::FromName(ParameterName), FText::AsNumber(Parameter.MinValue), FText::AsNumber(Parameter.MaxValue))
				: Parameter.Description)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(STextBlock).Text(FText::FromName(ParameterName))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(110.0f)
				[
					SNew(SSpinBox<float>)
					.MinValue(Parameter.MinValue)
					.MaxValue(Parameter.MaxValue)
					.MinSliderValue(Parameter.MinValue)
					.MaxSliderValue(Parameter.MaxValue)
					.Value_Lambda([WeakState, ParameterName]()
					{
						const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin();
						return PinnedState.IsValid() ? PinnedState->GetPreviewParameterValue(ParameterName) : 0.0f;
					})
					.OnValueChanged_Lambda([WeakState, ParameterName](float NewValue)
					{
						if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = WeakState.Pin())
						{
							PinnedState->SetPreviewParameterValue(ParameterName, NewValue);
						}
					})
				]
			]
		];
	}
	ParameterSliderBox->SetContent(Sliders);
}

TSharedRef<SWidget> SFeelRecipeBrowser::MakeComfortMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	const TWeakPtr<FFeelRecipeEditorState> WeakState = PreviewState;

	auto AddPreset = [&MenuBuilder, this](const FText& Label, TOptional<EFeelBuiltInComfortPreset> Preset)
	{
		MenuBuilder.AddMenuEntry(Label, FText::GetEmpty(), FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this, Preset]()
				{
					ComfortPreset = Preset;
					if (PreviewState.IsValid())
					{
						PreviewState->SetPreviewComfortPreset(Preset);
					}
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([this, Preset]() { return ComfortPreset == Preset; })),
			NAME_None, EUserInterfaceActionType::RadioButton);
	};

	AddPreset(LOCTEXT("ComfortNeutral", "No comfort limits"), TOptional<EFeelBuiltInComfortPreset>());
	AddPreset(LOCTEXT("ComfortDefault", "Project defaults"), EFeelBuiltInComfortPreset::Default);
	AddPreset(LOCTEXT("ComfortReducedMotion", "Reduced Motion"), EFeelBuiltInComfortPreset::ReducedMotion);
	AddPreset(LOCTEXT("ComfortReducedFlashing", "Reduced Flashing"), EFeelBuiltInComfortPreset::ReducedFlashing);
	AddPreset(LOCTEXT("ComfortNoHaptics", "No Haptics"), EFeelBuiltInComfortPreset::NoHaptics);
	return MenuBuilder.MakeWidget();
}

FText SFeelRecipeBrowser::GetComfortText() const
{
	if (!ComfortPreset.IsSet())
	{
		return LOCTEXT("ComfortNeutralShort", "Comfort: none");
	}
	switch (ComfortPreset.GetValue())
	{
	case EFeelBuiltInComfortPreset::ReducedMotion:
		return LOCTEXT("ComfortReducedMotionShort", "Comfort: Reduced Motion");
	case EFeelBuiltInComfortPreset::ReducedFlashing:
		return LOCTEXT("ComfortReducedFlashingShort", "Comfort: Reduced Flashing");
	case EFeelBuiltInComfortPreset::NoHaptics:
		return LOCTEXT("ComfortNoHapticsShort", "Comfort: No Haptics");
	default:
		return LOCTEXT("ComfortDefaultShort", "Comfort: project defaults");
	}
}

EActiveTimerReturnType SFeelRecipeBrowser::TickPreview(double InCurrentTime, float InDeltaTime)
{
	// A recipe starts playing once the cursor has rested on its row, so scrolling the list loads and plays nothing.
	const TSharedPtr<FFeelRecipeEntry> Hovered = FindHoveredEntry();
	if (Hovered != HoveredEntry)
	{
		HoveredEntry = Hovered;
		HoverSeconds = 0.0f;
	}
	else if (Hovered.IsValid())
	{
		HoverSeconds += InDeltaTime;
	}

	if (HoveredEntry.IsValid() && HoverSeconds >= FeelRecipeBrowserPrivate::HoverDelaySeconds)
	{
		if (UFeelRecipe* Recipe = Cast<UFeelRecipe>(HoveredEntry->Asset.GetAsset()))
		{
			if (Recipe != PreviewRecipe.Get())
			{
				SetPreviewRecipe(Recipe, false);
				RebuildParameterSliders();
			}
		}
	}

	if (PreviewState.IsValid())
	{
		PreviewState->Tick(InDeltaTime);
	}
	return EActiveTimerReturnType::Continue;
}

void SFeelRecipeBrowser::UseSelected()
{
	UFeelRecipe* Recipe = GetSelectedRecipe();
	if (!Recipe)
	{
		return;
	}

	if (bPickerMode)
	{
		OnRecipePicked.ExecuteIfBound(Recipe);
		return;
	}

	if (FeelLibrary::IsLibraryRecipe(Recipe))
	{
		FeelLibrary::CopyToProjectWithDialog(*Recipe, FeelLibrary::GetDefaultCopyFolder());
	}
	else
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Recipe);
	}
}

void SFeelRecipeBrowser::OpenSelected()
{
	if (UFeelRecipe* Recipe = GetSelectedRecipe())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Recipe);
	}
}

void SFeelRecipeBrowser::ShowSelectedInContentBrowser()
{
	const TSharedPtr<FFeelRecipeEntry> Entry = GetSelectedEntry();
	if (!Entry.IsValid())
	{
		return;
	}
	const TArray<FAssetData> Assets = { Entry->Asset };
	IContentBrowserSingleton::Get().SyncBrowserToAssets(Assets);
}

void SFeelRecipeBrowser::RegisterTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SFeelRecipeBrowser)
			];
	}))
	.SetDisplayName(LOCTEXT("TabTitle", "FeelKit Recipe Browser"))
	.SetTooltipText(LOCTEXT("TabTooltip", "Browse the recipes that ship with FeelKit and the recipes of this project, see and hear them in a preview, and copy one into your project."))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void SFeelRecipeBrowser::UnregisterTab()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
	}
}

void SFeelRecipeBrowser::OpenTemplatePicker(const FString& TargetFolder)
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("PickerTitle", "Create a recipe from a template"))
		.ClientSize(FVector2D(1100.0f, 640.0f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TWeakPtr<SWindow> WeakWindow = Window;
	Window->SetContent(
		SNew(SFeelRecipeBrowser)
		.PickerMode(true)
		.OnRecipePicked_Lambda([WeakWindow, TargetFolder](UFeelRecipe* Recipe)
		{
			if (const TSharedPtr<SWindow> PinnedWindow = WeakWindow.Pin())
			{
				PinnedWindow->RequestDestroyWindow();
			}
			if (Recipe)
			{
				FeelLibrary::CopyToProjectWithDialog(*Recipe, TargetFolder);
			}
		}));

	FSlateApplication::Get().AddModalWindow(Window, FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr));
}

#undef LOCTEXT_NAMESPACE
