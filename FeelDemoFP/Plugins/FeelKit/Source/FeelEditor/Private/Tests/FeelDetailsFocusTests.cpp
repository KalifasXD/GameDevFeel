// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/App.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UObject/Package.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace FeelDetailsFocusTests
{
	void CollectWidgets(const TSharedRef<SWidget>& Widget, TArray<TSharedRef<SWidget>>& Out, int32 Depth = 0)
	{
		if (Depth > 200)
		{
			return;
		}
		Out.Add(Widget);
		FChildren* Children = Widget->GetChildren();
		for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
		{
			CollectWidgets(Children->GetChildAt(Index), Out, Depth + 1);
		}
	}

	void CollectWindows(const TSharedRef<SWindow>& Window, TArray<TSharedRef<SWindow>>& Out)
	{
		Out.Add(Window);
		for (const TSharedRef<SWindow>& Child : Window->GetChildWindows())
		{
			CollectWindows(Child, Out);
		}
	}

	TSharedPtr<SWidget> FindAncestor(const TSharedPtr<SWidget>& Widget, const FName Type)
	{
		for (TSharedPtr<SWidget> Parent = Widget; Parent.IsValid(); Parent = Parent->GetParentWidget())
		{
			if (Parent->GetType() == Type)
			{
				return Parent;
			}
		}
		return nullptr;
	}

	/** The property name shown on the details row that contains Widget. */
	FString RowLabel(const TSharedPtr<SWidget>& Widget)
	{
		const TSharedPtr<SWidget> Row = FindAncestor(Widget, FName(TEXT("SDetailSingleItemRow")));
		if (!Row.IsValid())
		{
			return FString();
		}
		TArray<TSharedRef<SWidget>> Descendants;
		CollectWidgets(Row.ToSharedRef(), Descendants);
		for (const TSharedRef<SWidget>& Descendant : Descendants)
		{
			if (Descendant->GetType() == FName(TEXT("STextBlock")))
			{
				const FString Text = StaticCastSharedRef<STextBlock>(Descendant)->GetText().ToString();
				if (!Text.IsEmpty())
				{
					return Text;
				}
			}
		}
		return FString();
	}

	struct FRun
	{
		TWeakObjectPtr<UFeelRecipe> Recipe;
		FString WindowTitle;
		FString StartRow;
		FString EditedRow;
		TWeakPtr<SWidget> EditedWidget;
		int32 Frame = 0;
		int32 Stage = 0;
		double SearchStart = 0.0;
	};

	void PressKey(const FKey& Key)
	{
		FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(Key, FModifierKeysState(), 0, false, 0, 0));
		FSlateApplication::Get().ProcessKeyUpEvent(FKeyEvent(Key, FModifierKeysState(), 0, false, 0, 0));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelDetailsFocusCommand, TSharedRef<FeelDetailsFocusTests::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelDetailsFocusCommand::Update()
{
	using namespace FeelDetailsFocusTests;
	FSlateApplication& Slate = FSlateApplication::Get();
	++Run->Frame;

	switch (Run->Stage)
	{
	case 0:
	{
		// Let the editor window build its rows, then focus the start field.
		if (Run->Frame < 30)
		{
			return false;
		}
		TArray<TSharedRef<SWindow>> Windows;
		for (const TSharedRef<SWindow>& Window : Slate.GetTopLevelWindows())
		{
			CollectWindows(Window, Windows);
		}
		TArray<TSharedRef<SWidget>> Widgets;
		for (const TSharedRef<SWindow>& Window : Windows)
		{
			if (Window->GetTitle().ToString() == Run->WindowTitle)
			{
				CollectWidgets(Window, Widgets);
			}
		}
		for (const TSharedRef<SWidget>& Widget : Widgets)
		{
			if (Widget->GetTypeAsString().StartsWith(TEXT("SSpinBox")) && RowLabel(Widget) == Run->StartRow)
			{
				Slate.SetKeyboardFocus(Widget, EFocusCause::Mouse);
				Run->Stage = 1;
				Run->Frame = 0;
				return false;
			}
		}
		// A slow first start (right after a build) can take longer to build the Details rows; keep looking for up to 30
		// seconds of real time (frames can pass much faster than the rows are built).
		if (Run->SearchStart == 0.0)
		{
			Run->SearchStart = FPlatformTime::Seconds();
		}
		if (FPlatformTime::Seconds() - Run->SearchStart < 30.0)
		{
			return false;
		}
		Test->AddError(FString::Printf(TEXT("No '%s' field found in the recipe editor"), *Run->StartRow));
		return true;
	}

	case 1:
		// Tab puts the next field into text editing, as when a user tabs into it.
		if (Run->Frame < 5)
		{
			return false;
		}
		PressKey(EKeys::Tab);
		Run->Stage = 2;
		Run->Frame = 0;
		return false;

	case 2:
	{
		if (Run->Frame < 5)
		{
			return false;
		}
		const TSharedPtr<SWidget> Focused = Slate.GetKeyboardFocusedWidget();
		if (!Test->TestTrue(TEXT("Tab moves into the next field's text"), Focused.IsValid() && Focused->GetType() == FName(TEXT("SEditableText"))))
		{
			return true;
		}
		Run->EditedRow = RowLabel(Focused);
		Run->EditedWidget = Focused;
		StaticCastSharedPtr<SEditableText>(Focused)->SetText(FText::FromString(TEXT("3")));
		PressKey(EKeys::Tab);
		Run->Stage = 3;
		Run->Frame = 0;
		return false;
	}

	case 3:
	{
		if (Run->Frame < 30)
		{
			return false;
		}
		const TSharedPtr<SWidget> Focused = Slate.GetKeyboardFocusedWidget();
		Test->TestTrue(FString::Printf(TEXT("After changing '%s' and pressing Tab, the edited field still exists (the panel was not rebuilt)"), *Run->EditedRow), Run->EditedWidget.IsValid());
		Test->TestTrue(FString::Printf(TEXT("After changing '%s' and pressing Tab, keyboard focus is in the next field"), *Run->EditedRow), Focused.IsValid() && !RowLabel(Focused).IsEmpty() && RowLabel(Focused) != Run->EditedRow);
		return true;
	}

	default:
		return true;
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFeelDetailsFocusTest, "FeelKit.Editor.DetailsKeepFocusAfterEdit", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FFeelDetailsFocusTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("TrackField"));
	OutTestCommands.Add(TEXT("Start Time"));
	OutBeautifiedNames.Add(TEXT("RecipeField"));
	OutTestCommands.Add(TEXT("Cooldown"));
}

bool FFeelDetailsFocusTest::RunTest(const FString& Parameters)
{
	if (!FSlateApplication::IsInitialized() || !GEditor || !FApp::CanEverRender())
	{
		// Details rows are only built when the editor renders (not with -nullrhi).
		AddInfo(TEXT("Needs a rendering editor session; skipped. Run without -nullrhi."));
		return true;
	}

	const FString Name = FString::Printf(TEXT("R_FocusTest_%d"), FMath::Abs(static_cast<int32>(GetTypeHash(Parameters))));
	UPackage* Package = CreatePackage(*(TEXT("/Temp/FeelKitTests/") + Name));
	UFeelRecipe* Recipe = NewObject<UFeelRecipe>(Package, *Name, RF_Public | RF_Standalone | RF_Transactional);
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	Track.Step = NewObject<UFeelStep_ScalePunch>(Recipe, NAME_None, RF_Transactional);
	Track.Duration = 0.5f;

	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Recipe);
	const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Recipe);
	if (!TestTrue(TEXT("Recipe editor opened"), State.IsValid()))
	{
		return false;
	}
	// Track fields are shown for the selected track; recipe fields only while no track is selected, because the details
	// panel collapses the Recipe category to make room for the track.
	State->SetSelectedTrack(Parameters == TEXT("Start Time") ? 0 : INDEX_NONE);

	const TSharedRef<FeelDetailsFocusTests::FRun> Run = MakeShared<FeelDetailsFocusTests::FRun>();
	Run->Recipe = Recipe;
	Run->WindowTitle = Name;
	Run->StartRow = Parameters;
	ADD_LATENT_AUTOMATION_COMMAND(FFeelDetailsFocusCommand(Run, this));
	return true;
}

#endif
