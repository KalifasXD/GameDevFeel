// Copyright 2026 Billo. All Rights Reserved.

#include "SFeelDebugger.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "FeelComfortSubsystem.h"
#include "FeelPlayCapture.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/SlateIconFinder.h"
#include "Styling/StyleColors.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FeelDebugger"

const FName SFeelDebugger::TabName(TEXT("FeelKitDebugger"));

void SFeelDebugger::RegisterTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				SNew(SFeelDebugger)
			];
	}))
	.SetDisplayName(LOCTEXT("TabTitle", "FeelKit Debugger"))
	.SetTooltipText(LOCTEXT("TabTooltip", "See what FeelKit is playing during Play In Editor, and open recent plays in the recipe editor to replay them exactly."))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory());
}

void SFeelDebugger::UnregisterTab()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
	}
}

/** One row of the debugger tree. Fields are updated in place on every refresh; the row widgets read them. */
struct FFeelDebugItem
{
	FString Id;
	FText Name;
	FText Target;
	FText Time;
	FText Value;
	FText Details;
	const FSlateBrush* Icon = nullptr;
	FSlateColor IconColor = FSlateColor::UseForeground();
	bool bWarning = false;
	bool bSubdued = false;
	TOptional<FFeelPlayCapture> Capture;
	TArray<TSharedPtr<FFeelDebugItem>> Children;
};

const FName SFeelDebugger::ColumnName(TEXT("Name"));
const FName SFeelDebugger::ColumnTarget(TEXT("Target"));
const FName SFeelDebugger::ColumnTime(TEXT("Time"));
const FName SFeelDebugger::ColumnValue(TEXT("Value"));
const FName SFeelDebugger::ColumnDetails(TEXT("Details"));
const FName SFeelDebugger::ColumnReplay(TEXT("Replay"));

namespace FeelDebuggerPrivate
{
	FText Number(float Value)
	{
		FNumberFormattingOptions Options;
		Options.MinimumFractionalDigits = 2;
		Options.MaximumFractionalDigits = 2;
		return FText::AsNumber(Value, &Options);
	}

	const FSlateBrush* FolderIcon()
	{
		return FAppStyle::GetBrush(TEXT("SceneOutliner.FolderClosed"));
	}

	void OpenCapture(const FFeelDebugItem& Item)
	{
		if (Item.Capture.IsSet() && Item.Capture->Recipe.IsValid())
		{
			FFeelRecipeEditorState::OpenCapture(Item.Capture.GetValue());
		}
	}
}

/** Outliner-style row: the name column carries the expander and icon, the other columns are plain text. */
class SFeelDebugRow : public SMultiColumnTableRow<TSharedPtr<FFeelDebugItem>>
{
public:
	SLATE_BEGIN_ARGS(SFeelDebugRow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable, TSharedPtr<FFeelDebugItem> InItem)
	{
		Item = InItem;
		SMultiColumnTableRow<TSharedPtr<FFeelDebugItem>>::Construct(
			FSuperRowType::FArguments().Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>(TEXT("SceneOutliner.TableViewRow"))),
			OwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnId) override
	{
		const TWeakPtr<FFeelDebugItem> WeakItem = Item;
		auto TextColor = [WeakItem]()
		{
			const TSharedPtr<FFeelDebugItem> Pinned = WeakItem.Pin();
			if (Pinned.IsValid() && Pinned->bWarning)
			{
				return FSlateColor(FStyleColors::Warning);
			}
			return Pinned.IsValid() && Pinned->bSubdued ? FSlateColor::UseSubduedForeground() : FSlateColor::UseForeground();
		};
		auto Field = [WeakItem](FText FFeelDebugItem::* Member)
		{
			return [WeakItem, Member]()
			{
				const TSharedPtr<FFeelDebugItem> Pinned = WeakItem.Pin();
				return Pinned.IsValid() ? (*Pinned).*Member : FText::GetEmpty();
			};
		};

		if (ColumnId == SFeelDebugger::ColumnName)
		{
			// Outliner rows are 20 pixels tall.
			return SNew(SBox)
				.HeightOverride(20.0f)
				[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(12.0f)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 6.0f, 0.0f)
				[
					SNew(SImage)
					.Image_Lambda([WeakItem]()
					{
						const TSharedPtr<FFeelDebugItem> Pinned = WeakItem.Pin();
						return Pinned.IsValid() ? Pinned->Icon : nullptr;
					})
					.ColorAndOpacity_Lambda([WeakItem]()
					{
						const TSharedPtr<FFeelDebugItem> Pinned = WeakItem.Pin();
						return Pinned.IsValid() ? Pinned->IconColor : FSlateColor::UseForeground();
					})
					.DesiredSizeOverride(FVector2D(16.0, 16.0))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda(Field(&FFeelDebugItem::Name))
					.ColorAndOpacity_Lambda(TextColor)
				]
				];
		}

		if (ColumnId == SFeelDebugger::ColumnReplay)
		{
			if (!Item->Capture.IsSet())
			{
				return SNullWidget::NullWidget;
			}
			return SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), TEXT("SimpleButton"))
					.ContentPadding(FMargin(2.0f, 0.0f))
					.ToolTipText(LOCTEXT("OpenCaptureTip", "Open the recipe and replay this play in its preview, with the same random rolls, intensity, parameters and comfort. Double-clicking the row does the same."))
					.IsEnabled(Item->Capture->Recipe.IsValid())
					.OnClicked_Lambda([WeakItem]()
					{
						if (const TSharedPtr<FFeelDebugItem> Pinned = WeakItem.Pin())
						{
							FeelDebuggerPrivate::OpenCapture(*Pinned);
						}
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.Image(FAppStyle::GetBrush(TEXT("Icons.Play")))
						.DesiredSizeOverride(FVector2D(12.0, 12.0))
						.ColorAndOpacity(FSlateColor::UseForeground())
					]
				];
		}

		FText FFeelDebugItem::* Member = &FFeelDebugItem::Details;
		if (ColumnId == SFeelDebugger::ColumnTarget)
		{
			Member = &FFeelDebugItem::Target;
		}
		else if (ColumnId == SFeelDebugger::ColumnTime)
		{
			Member = &FFeelDebugItem::Time;
		}
		else if (ColumnId == SFeelDebugger::ColumnValue)
		{
			Member = &FFeelDebugItem::Value;
		}
		return SNew(SBox)
			.VAlign(VAlign_Center)
			.Padding(FMargin(4.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text_Lambda(Field(Member))
				.ColorAndOpacity_Lambda(TextColor)
			];
	}

private:
	TSharedPtr<FFeelDebugItem> Item;
};

void SFeelDebugger::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("Brushes.Panel")))
		.Padding(0.0f)
		[
			SNew(SVerticalBox)
			// Toolbar row, as in the Outliner: search, then actions.
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(SearchBox, SSearchBox)
					.HintText(LOCTEXT("SearchHint", "Search recipes, targets and values"))
					.OnTextChanged_Lambda([this](const FText& NewText)
					{
						SearchText = NewText.ToString();
						Refresh();
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(6.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), TEXT("SimpleButton"))
					.ToolTipText(LOCTEXT("ClearCapturesTip", "Forget every recorded play."))
					.OnClicked_Lambda([]()
					{
						FFeelPlayCaptureStore::Get().Clear();
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
							.Image(FAppStyle::GetBrush(TEXT("Icons.Delete")))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(LOCTEXT("ClearCaptures", "Clear Recent Plays"))
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(TreeView, STreeView<TSharedPtr<FFeelDebugItem>>)
				.TreeItemsSource(&RootItems)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SFeelDebugger::GenerateRow)
				.OnGetChildren(this, &SFeelDebugger::GetItemChildren)
				.OnMouseButtonDoubleClick(this, &SFeelDebugger::OnItemDoubleClicked)
				.OnContextMenuOpening(this, &SFeelDebugger::OnContextMenuOpening)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ColumnName).DefaultLabel(LOCTEXT("ColumnName", "Name")).FillWidth(0.30f)
					+ SHeaderRow::Column(ColumnTarget).DefaultLabel(LOCTEXT("ColumnTarget", "Target")).FillWidth(0.16f)
					+ SHeaderRow::Column(ColumnTime).DefaultLabel(LOCTEXT("ColumnTime", "Time")).FillWidth(0.12f)
					+ SHeaderRow::Column(ColumnValue).DefaultLabel(LOCTEXT("ColumnValue", "Value")).FillWidth(0.08f)
					+ SHeaderRow::Column(ColumnDetails).DefaultLabel(LOCTEXT("ColumnDetails", "Details")).FillWidth(0.34f)
					+ SHeaderRow::Column(ColumnReplay).DefaultLabel(LOCTEXT("ColumnReplay", "Replay")).FixedWidth(56.0f).HAlignHeader(HAlign_Center)
				)
			]
		]
	];

	CaptureStoreHandle = FFeelPlayCaptureStore::Get().OnChanged.AddSP(this, &SFeelDebugger::Refresh);
	Refresh();
	RegisterActiveTimer(0.25f, FWidgetActiveTimerDelegate::CreateSP(this, &SFeelDebugger::RefreshTimer));
}

SFeelDebugger::~SFeelDebugger()
{
	FFeelPlayCaptureStore::Get().OnChanged.Remove(CaptureStoreHandle);
}

EActiveTimerReturnType SFeelDebugger::RefreshTimer(double InCurrentTime, float InDeltaTime)
{
	Refresh();
	return EActiveTimerReturnType::Continue;
}

TSharedPtr<FFeelDebugItem> SFeelDebugger::FindOrAddItem(const FString& Id)
{
	TSharedPtr<FFeelDebugItem>& Item = ItemsById.FindOrAdd(Id);
	if (!Item.IsValid())
	{
		Item = MakeShared<FFeelDebugItem>();
		Item->Id = Id;
	}
	Item->Children.Reset();
	Item->bWarning = false;
	Item->bSubdued = false;
	Item->IconColor = FSlateColor::UseForeground();
	Item->Target = FText::GetEmpty();
	Item->Time = FText::GetEmpty();
	Item->Value = FText::GetEmpty();
	Item->Details = FText::GetEmpty();
	Item->Capture.Reset();
	return Item;
}

bool SFeelDebugger::PassesSearch(const FFeelDebugItem& Item) const
{
	if (SearchText.IsEmpty())
	{
		return true;
	}
	if (Item.Name.ToString().Contains(SearchText) || Item.Target.ToString().Contains(SearchText) || Item.Details.ToString().Contains(SearchText))
	{
		return true;
	}
	for (const TSharedPtr<FFeelDebugItem>& Child : Item.Children)
	{
		if (PassesSearch(*Child))
		{
			return true;
		}
	}
	return false;
}

void SFeelDebugger::Refresh()
{
	using namespace FeelDebuggerPrivate;

	TArray<TSharedPtr<FFeelDebugItem>> NewRoots;
	TSet<FString> UsedIds;
	auto Add = [this, &UsedIds](const FString& Id)
	{
		UsedIds.Add(Id);
		return FindOrAddItem(Id);
	};

	int32 NumWorlds = 0;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World || (Context.WorldType != EWorldType::PIE && Context.WorldType != EWorldType::Game))
			{
				continue;
			}
			++NumWorlds;

			const FString WorldId = FString::Printf(TEXT("World:%s"), *Context.ContextHandle.ToString());
			const UFeelSubsystem* Subsystem = World->GetSubsystem<UFeelSubsystem>();
			const ENetMode NetMode = World->GetNetMode();
			const FText Role = NetMode == NM_ListenServer ? LOCTEXT("RoleListen", "Listen server")
				: NetMode == NM_Client ? LOCTEXT("RoleClient", "Client")
				: NetMode == NM_DedicatedServer ? LOCTEXT("RoleDedicated", "Dedicated server")
				: LOCTEXT("RoleSingle", "Single player");

			TSharedPtr<FFeelDebugItem> WorldItem = Add(WorldId);
			// The level's name as the Outliner shows it, without the Play In Editor prefix.
			WorldItem->Name = FText::FromString(UWorld::RemovePIEPrefix(World->GetMapName()));
			WorldItem->Icon = FAppStyle::GetBrush(TEXT("SceneOutliner.World"));
			WorldItem->Details = Role;
			NewRoots.Add(WorldItem);

			// Plays.
			TSharedPtr<FFeelDebugItem> PlaysItem = Add(WorldId + TEXT("/Plays"));
			PlaysItem->Icon = FolderIcon();
			PlaysItem->IconColor = FStyleColors::AccentFolder;
			int32 NumPlays = 0;
			if (Subsystem)
			{
				for (const FFeelInstance& Instance : Subsystem->GetInstances())
				{
					if (Instance.bFinished || !Instance.Recipe)
					{
						continue;
					}
					++NumPlays;
					const FFeelTarget Target = Instance.MakeTarget();
					const bool bSustaining = FFeelPlaybackClock::HasSustain(*Instance.Recipe) && !Instance.Clock.bReleased;

					TSharedPtr<FFeelDebugItem> PlayItem = Add(FString::Printf(TEXT("%s/Play:%d"), *WorldId, Instance.Id));
					PlayItem->Name = FText::FromString(Instance.Recipe->GetName());
					PlayItem->Icon = FSlateIconFinder::FindIconBrushForClass(UFeelRecipe::StaticClass());
					PlayItem->Target = Target.GetActor() ? FText::FromString(Target.GetActor()->GetActorNameOrLabel()) : LOCTEXT("NoTarget", "-");
					PlayItem->Time = FText::Format(LOCTEXT("PlayTime", "{0} / {1} s"), Number(Instance.Clock.RecipeTime), Number(Instance.Duration));
					PlayItem->Value = Number(Instance.Intensity);

					TArray<FString> Details;
					Details.Add(Instance.bStopping ? TEXT("Stopping") : (bSustaining ? TEXT("Sustaining") : TEXT("Playing")));
					for (const TPair<FName, float>& Pair : Instance.ParameterValues)
					{
						Details.Add(FString::Printf(TEXT("%s %.2f"), *Pair.Key.ToString(), Pair.Value));
					}
					PlayItem->Details = FText::FromString(FString::Join(Details, TEXT(", ")));
					PlaysItem->Children.Add(PlayItem);
				}
			}
			PlaysItem->Name = FText::Format(LOCTEXT("PlaysFolder", "Playing ({0})"), FText::AsNumber(NumPlays));
			WorldItem->Children.Add(PlaysItem);

			// Accumulators.
			if (Subsystem)
			{
				TSharedPtr<FFeelDebugItem> AccumulatorsItem = Add(WorldId + TEXT("/Accumulators"));
				AccumulatorsItem->Icon = FolderIcon();
				AccumulatorsItem->IconColor = FStyleColors::AccentFolder;
				Subsystem->ForEachAccumulator([&](FName Name, const AActor* Actor, float Value)
				{
					TSharedPtr<FFeelDebugItem> AccumulatorItem = Add(FString::Printf(TEXT("%s/Accumulator:%s:%s"), *WorldId, *Name.ToString(), Actor ? *Actor->GetPathName() : TEXT("global")));
					AccumulatorItem->Name = FText::FromName(Name);
					AccumulatorItem->Icon = FAppStyle::GetBrush(TEXT("Icons.Adjust"));
					AccumulatorItem->Target = Actor ? FText::FromString(Actor->GetActorNameOrLabel()) : LOCTEXT("GlobalAccumulator", "Global");
					AccumulatorItem->Value = Number(Value);
					AccumulatorsItem->Children.Add(AccumulatorItem);
				});
				AccumulatorsItem->Name = FText::Format(LOCTEXT("AccumulatorsFolder", "Accumulators ({0})"), FText::AsNumber(AccumulatorsItem->Children.Num()));
				WorldItem->Children.Add(AccumulatorsItem);
			}

			// Each local player's comfort and controller vibration.
			for (const TObjectPtr<ULocalPlayer>& LocalPlayer : World->GetGameInstance() ? World->GetGameInstance()->GetLocalPlayers() : TArray<TObjectPtr<ULocalPlayer>>())
			{
				const UFeelComfortSubsystem* Comfort = LocalPlayer ? LocalPlayer->GetSubsystem<UFeelComfortSubsystem>() : nullptr;
				if (!Comfort)
				{
					continue;
				}
				const int32 PlayerIndex = LocalPlayer->GetLocalPlayerIndex();
				const FString PlayerId = FString::Printf(TEXT("%s/Player:%d"), *WorldId, PlayerIndex);
				const FFeelComfortScales& Scales = Comfort->GetComfortScalesRef();

				TSharedPtr<FFeelDebugItem> PlayerItem = Add(PlayerId);
				PlayerItem->Name = FText::Format(LOCTEXT("PlayerRow", "Player {0} comfort"), FText::AsNumber(PlayerIndex));
				PlayerItem->Icon = FSlateIconFinder::FindIconBrushForClass(APlayerController::StaticClass());
				PlayerItem->Details = FText::Format(LOCTEXT("PlayerDetails", "Flash limiter {0}, camera roll {1}"),
					Scales.bLimitFlashes ? FText::Format(LOCTEXT("FlashLimit", "{0} per second"), FText::AsNumber(Scales.MaxFlashesPerSecond)) : LOCTEXT("Off", "off"),
					Scales.bAllowCameraRoll ? LOCTEXT("On", "on") : LOCTEXT("Off", "off"));

				const TPair<FText, float> Groups[] =
				{
					{ LOCTEXT("ComfortMaster", "Master"), Scales.Master },
					{ LOCTEXT("ComfortShake", "Camera Shake"), Scales.CameraShake },
					{ LOCTEXT("ComfortMotion", "Camera Motion"), Scales.CameraMotion },
					{ LOCTEXT("ComfortFlashes", "Flashes"), Scales.Flashes },
					{ LOCTEXT("ComfortHitstop", "Hitstop and Slow-mo"), Scales.HitstopAndSlowMo },
					{ LOCTEXT("ComfortDistortion", "Screen Distortion"), Scales.ScreenDistortion },
					{ LOCTEXT("ComfortHaptics", "Haptics"), Scales.Haptics },
				};
				for (int32 GroupIndex = 0; GroupIndex < UE_ARRAY_COUNT(Groups); ++GroupIndex)
				{
					TSharedPtr<FFeelDebugItem> GroupItem = Add(FString::Printf(TEXT("%s/Comfort:%d"), *PlayerId, GroupIndex));
					GroupItem->Name = Groups[GroupIndex].Key;
					GroupItem->Icon = FAppStyle::GetBrush(TEXT("Icons.Adjust"));
					GroupItem->Value = Number(Groups[GroupIndex].Value);
					GroupItem->bSubdued = FMath::IsNearlyEqual(Groups[GroupIndex].Value, 1.0f);
					PlayerItem->Children.Add(GroupItem);
				}

				// Vibration that never reaches the controller is impossible to see anywhere else.
				const APlayerController* PlayerController = LocalPlayer->PlayerController.Get();
				const float ComfortScale = Comfort->GetEffectiveForceFeedbackScale();
				const float ControllerScale = PlayerController ? PlayerController->ForceFeedbackScale : 1.0f;
				const bool bSilent = ControllerScale <= UE_KINDA_SMALL_NUMBER;
				TSharedPtr<FFeelDebugItem> VibrationItem = Add(PlayerId + TEXT("/Vibration"));
				VibrationItem->Name = LOCTEXT("VibrationRow", "Controller vibration");
				VibrationItem->Icon = FAppStyle::GetBrush(bSilent ? TEXT("Icons.Warning") : TEXT("Icons.Adjust"));
				VibrationItem->IconColor = bSilent ? FSlateColor(FStyleColors::Warning) : FSlateColor::UseForeground();
				VibrationItem->Value = Number(ControllerScale);
				VibrationItem->Details = bSilent
					? FText::Format(LOCTEXT("VibrationSilent", "Nothing reaches the controller (comfort scale {0}, controller scale {1})"), Number(ComfortScale), Number(ControllerScale))
					: FText::Format(LOCTEXT("VibrationDetails", "Comfort scale {0}, controller scale {1}"), Number(ComfortScale), Number(ControllerScale));
				VibrationItem->bWarning = bSilent;
				PlayerItem->Children.Add(VibrationItem);

				WorldItem->Children.Add(PlayerItem);
			}
		}
	}

	if (NumWorlds == 0)
	{
		TSharedPtr<FFeelDebugItem> Message = Add(TEXT("NothingRunning"));
		Message->Name = LOCTEXT("NoWorlds", "Nothing running. Start Play In Editor to see plays, accumulators and comfort here.");
		Message->Icon = FAppStyle::GetBrush(TEXT("Icons.Info"));
		Message->bSubdued = true;
		NewRoots.Add(Message);
	}

	// Recent plays, newest first.
	const TArray<FFeelPlayCapture>& Captures = FFeelPlayCaptureStore::Get().GetCaptures();
	TSharedPtr<FFeelDebugItem> RecentItem = Add(TEXT("Recent"));
	RecentItem->Name = FText::Format(LOCTEXT("RecentFolder", "Recent plays ({0})"), FText::AsNumber(Captures.Num()));
	RecentItem->Icon = FolderIcon();
	RecentItem->IconColor = FStyleColors::AccentFolder;
	RecentItem->Details = Captures.Num() == 0 ? LOCTEXT("NoCaptures", "Every play that ends during Play In Editor is recorded here.") : LOCTEXT("RecentHint", "Double-click a play to replay it in the recipe editor.");
	for (int32 CaptureIndex = Captures.Num() - 1; CaptureIndex >= 0; --CaptureIndex)
	{
		const FFeelPlayCapture& Capture = Captures[CaptureIndex];
		TSharedPtr<FFeelDebugItem> CaptureItem = Add(FString::Printf(TEXT("Recent:%s:%s:%d"), *Capture.RecipeName, *Capture.EndedAt.ToString(), Capture.Seed));
		CaptureItem->Name = FText::FromString(Capture.RecipeName);
		CaptureItem->Icon = FSlateIconFinder::FindIconBrushForClass(UFeelRecipe::StaticClass());
		CaptureItem->Target = FText::FromString(Capture.TargetName);
		CaptureItem->Time = FText::FromString(Capture.EndedAt.ToString(TEXT("%H:%M:%S")));
		CaptureItem->Value = Number(Capture.Intensity);

		TArray<FString> Details;
		Details.Add(FString::Printf(TEXT("%.2f s"), Capture.PlayedSeconds));
		if (Capture.bReleased)
		{
			Details.Add(TEXT("released"));
		}
		if (Capture.bInterrupted)
		{
			Details.Add(TEXT("stopped early"));
		}
		for (const TPair<FName, float>& Pair : Capture.ParameterValues)
		{
			Details.Add(FString::Printf(TEXT("%s %.2f"), *Pair.Key.ToString(), Pair.Value));
		}
		CaptureItem->Details = FText::FromString(FString::Join(Details, TEXT(", ")));
		CaptureItem->Capture = Capture;
		CaptureItem->bSubdued = !Capture.Recipe.IsValid();
		RecentItem->Children.Add(CaptureItem);
	}
	NewRoots.Add(RecentItem);

	// Search keeps an item when it or anything under it matches.
	TFunction<void(TArray<TSharedPtr<FFeelDebugItem>>&)> Filter = [this, &Filter](TArray<TSharedPtr<FFeelDebugItem>>& Items)
	{
		Items.RemoveAll([this](const TSharedPtr<FFeelDebugItem>& Item) { return !PassesSearch(*Item); });
		for (const TSharedPtr<FFeelDebugItem>& Item : Items)
		{
			if (!SearchText.IsEmpty() && (Item->Name.ToString().Contains(SearchText) || Item->Target.ToString().Contains(SearchText)))
			{
				continue;
			}
			Filter(Item->Children);
		}
	};
	Filter(NewRoots);

	// Drop items that no longer exist, and open new ones once.
	for (auto It = ItemsById.CreateIterator(); It; ++It)
	{
		if (!UsedIds.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}
	RootItems = MoveTemp(NewRoots);
	for (const TPair<FString, TSharedPtr<FFeelDebugItem>>& Pair : ItemsById)
	{
		if (!KnownIds.Contains(Pair.Key))
		{
			KnownIds.Add(Pair.Key);
			// Worlds, folders and players start open; comfort rows start open too, since they are the reason to look.
			TreeView->SetItemExpansion(Pair.Value, true);
		}
	}
	TreeView->RequestTreeRefresh();
}

TSharedRef<ITableRow> SFeelDebugger::GenerateRow(TSharedPtr<FFeelDebugItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SFeelDebugRow, OwnerTable, Item);
}

void SFeelDebugger::GetItemChildren(TSharedPtr<FFeelDebugItem> Item, TArray<TSharedPtr<FFeelDebugItem>>& OutChildren)
{
	if (Item.IsValid())
	{
		OutChildren = Item->Children;
	}
}

void SFeelDebugger::OnItemDoubleClicked(TSharedPtr<FFeelDebugItem> Item)
{
	if (Item.IsValid())
	{
		if (Item->Capture.IsSet())
		{
			FeelDebuggerPrivate::OpenCapture(*Item);
		}
		else if (!Item->Children.IsEmpty())
		{
			TreeView->SetItemExpansion(Item, !TreeView->IsItemExpanded(Item));
		}
	}
}

TSharedPtr<SWidget> SFeelDebugger::OnContextMenuOpening()
{
	const TArray<TSharedPtr<FFeelDebugItem>> Selected = TreeView->GetSelectedItems();
	if (Selected.Num() != 1 || !Selected[0]->Capture.IsSet())
	{
		return nullptr;
	}
	const TSharedPtr<FFeelDebugItem> Item = Selected[0];
	FMenuBuilder Menu(true, nullptr);
	Menu.AddMenuEntry(
		LOCTEXT("ReplayMenu", "Open and Replay"),
		LOCTEXT("OpenCaptureTip", "Open the recipe and replay this play in its preview, with the same random rolls, intensity, parameters and comfort. Double-clicking the row does the same."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Play")),
		FUIAction(
			FExecuteAction::CreateLambda([Item]() { FeelDebuggerPrivate::OpenCapture(*Item); }),
			FCanExecuteAction::CreateLambda([Item]() { return Item->Capture->Recipe.IsValid(); })));
	return Menu.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
