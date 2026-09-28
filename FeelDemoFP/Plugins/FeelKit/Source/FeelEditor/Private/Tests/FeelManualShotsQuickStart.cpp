// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "ContentBrowserModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FeelBlueprintLibrary.h"
#include "FeelEditorSettings.h"
#include "FeelLibrary.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelSwitch.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "IContentBrowserSingleton.h"
#include "K2Node_CallFunction.h"
#include "K2Node_InputKey.h"
#include "K2Node_Self.h"
#include "EdGraphNode_Comment.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/ContentBrowserSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/SWindow.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's quick start chapter, saved to Saved/FeelKit/Manual/QS_*.png
 * at 1.25x application scale. Content Browser at the Impact library with FR_Impact_HeavyHit selected; that library
 * recipe open read-only; a character Blueprint whose Event Graph plays the project copy /Game/HeavyHit on the 1 key
 * (built in a transient package, never saved); then Play In Editor with a default Feel Switch: start card, the moment
 * of the hit, and the badge after Tab. Needs a rendering session and the /Game/HeavyHit copy.
 */
namespace FeelQuickStartShots
{
	/** Pixel density of the pictures (FeelManualCapture). */
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		bool bPreviousPluginFolders = false;
		bool bPreviousAllowLibrary = false;
		TWeakObjectPtr<UBlueprint> Blueprint;
		TWeakObjectPtr<UEdGraphNode> PlayNode;
		int32 PreviousMotionBlur = 4;
	};

	UWorld* PIEWorld()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::PIE && Context.World())
			{
				return Context.World();
			}
		}
		return nullptr;
	}

	bool SaveWindow(const TSharedPtr<SWindow>& Window, const TCHAR* Name, FAutomationTestBase* Test)
	{
		return FeelManualCapture::SaveWindow(Window, Name, Density, Test);
	}

	bool SaveGame(const TCHAR* Name, FAutomationTestBase* Test)
	{
		UWorld* World = PIEWorld();
		return FeelManualCapture::SaveGameView(World ? World->GetGameViewport() : nullptr, Name, Density, Test);
	}

	TSharedPtr<SWindow> MainWindow()
	{
		return FGlobalTabmanager::Get()->GetRootWindow();
	}

	TSharedPtr<SWindow> EditorWindow(UObject* Asset)
	{
		UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Asset, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		const TSharedPtr<SDockTab> Owner = Tabs.IsValid() ? Tabs->GetOwnerTab() : nullptr;
		return Owner.IsValid() ? Owner->GetParentWindow() : nullptr;
	}

	/** A character Blueprint whose Event Graph reads: 1 (Pressed) -> Play Feel (Recipe, Target <- Make Feel Target from Actor <- Self). */
	UBlueprint* BuildQuickStartBlueprint(UFeelRecipe* Recipe, TWeakObjectPtr<UEdGraphNode>& OutPlayNode)
	{
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManual/BP_ThirdPersonCharacter"));
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), Package, TEXT("BP_ThirdPersonCharacter"),
			BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		if (!Graph)
		{
			return Blueprint;
		}
		// Remove the default event nodes so the graph shows only the quick start.
		TArray<UEdGraphNode*> Existing = Graph->Nodes;
		for (UEdGraphNode* Node : Existing)
		{
			Graph->RemoveNode(Node);
		}

		auto Place = [Graph](UEdGraphNode* Node, int32 X, int32 Y)
		{
			Graph->AddNode(Node, false, false);
			Node->CreateNewGuid();
			Node->PostPlacedNewNode();
			Node->AllocateDefaultPins();
			Node->NodePosX = X;
			Node->NodePosY = Y;
		};

		UK2Node_InputKey* Key = NewObject<UK2Node_InputKey>(Graph);
		Key->InputKey = EKeys::One;
		Place(Key, -420, 0);

		UK2Node_CallFunction* Play = NewObject<UK2Node_CallFunction>(Graph);
		Play->SetFromFunction(UFeelBlueprintLibrary::StaticClass()->FindFunctionByName(TEXT("PlayFeel")));
		Place(Play, 0, 0);

		UK2Node_CallFunction* Make = NewObject<UK2Node_CallFunction>(Graph);
		Make->SetFromFunction(UFeelBlueprintLibrary::StaticClass()->FindFunctionByName(TEXT("MakeFeelTargetFromActor")));
		Place(Make, -370, 200);

		UK2Node_Self* Self = NewObject<UK2Node_Self>(Graph);
		Place(Self, -560, 224);

		// A comment around the chain; the editor centers the view on it.
		UEdGraphNode_Comment* Comment = NewObject<UEdGraphNode_Comment>(Graph);
		Comment->NodeComment = TEXT("Play a recipe when 1 is pressed");
		Graph->AddNode(Comment, false, false);
		Comment->CreateNewGuid();
		Comment->NodePosX = -600;
		Comment->NodePosY = -70;
		Comment->NodeWidth = 860;
		Comment->NodeHeight = 380;
		OutPlayNode = Comment;

		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
		Schema->TryCreateConnection(Key->FindPin(TEXT("Pressed")), Play->GetExecPin());
		Schema->TryCreateConnection(Self->FindPin(UEdGraphSchema_K2::PN_Self), Make->FindPin(TEXT("Actor")));
		Schema->TryCreateConnection(Make->GetReturnValuePin(), Play->FindPin(TEXT("Target")));
		if (UEdGraphPin* RecipePin = Play->FindPin(TEXT("Recipe")))
		{
			Schema->TrySetDefaultObject(*RecipePin, Recipe);
		}
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return Blueprint;
	}

	void PressKey(const FKey& Key)
	{
		FSlateApplication& Slate = FSlateApplication::Get();
		Slate.SetAllUserFocusToGameViewport();
		const FKeyEvent Event(Key, FModifierKeysState(), 0, false, 0, 0);
		Slate.ProcessKeyDownEvent(Event);
		Slate.ProcessKeyUpEvent(Event);
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelQuickStartShotsCommand, TSharedRef<FeelQuickStartShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelQuickStartShotsCommand::Update()
{
	using namespace FeelQuickStartShots;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;
	auto Next = [this, Now]()
	{
		++Run->Stage;
		Run->StageStart = Now;
	};
	UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	UFeelRecipe* Library = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_HeavyHit.FR_Impact_HeavyHit"));
	UFeelRecipe* Copy = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/HeavyHit.HeavyHit"));

	switch (Run->Stage)
	{
	case 0:
	{
		// Content Browser at the Impact folder of the library, with the recipe selected.
		UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
		Run->bPreviousPluginFolders = Browser->GetDisplayPluginFolders();
		Browser->SetDisplayPluginFolders(true);
		Browser->PostEditChange();
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("ContentBrowserTab2")));
		if (const TSharedPtr<SWindow> Window = Tab.IsValid() ? Tab->GetParentWindow() : nullptr)
		{
			Window->Resize(FVector2D(1500.0, 820.0));
		}
		IContentBrowserSingleton& ContentBrowser = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser")).Get();
		ContentBrowser.SyncBrowserToAssets(TArray<FAssetData>{ FAssetData(Library) }, false, true, TEXT("ContentBrowserTab2"));
		Next();
		return false;
	}

	case 1:
		if (Elapsed < 4.0)
		{
			return false;
		}
		{
			const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TEXT("ContentBrowserTab2")));
			if (!SaveWindow(Tab.IsValid() ? Tab->GetParentWindow() : MainWindow(), TEXT("QS_ContentBrowser"), Test))
			{
				return false;
			}
			if (Tab.IsValid())
			{
				Tab->RequestCloseTab();
			}
		}
		{
			// The library recipe as a double-click opens it: read-only, with the Copy to Project banner.
			UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
			Run->bPreviousAllowLibrary = Settings->bAllowLibraryEditing;
			Settings->bAllowLibraryEditing = false;
			FeelLibrary::ApplyWritePermission();
			AssetEditors->OpenEditorForAssets({ Library }, EAssetTypeActivationOpenedMethod::View);
			if (const TSharedPtr<SWindow> Window = EditorWindow(Library))
			{
				Window->Resize(FVector2D(1900.0, 1060.0));
			}
		}
		Next();
		return false;

	case 2:
		if (Elapsed < 8.0)
		{
			return false;
		}
		if (!SaveWindow(EditorWindow(Library), TEXT("QS_LibraryRecipe"), Test))
		{
			return false;
		}
		AssetEditors->CloseAllEditorsForAsset(Library);
		{
			UFeelEditorSettings* Settings = GetMutableDefault<UFeelEditorSettings>();
			Settings->bAllowLibraryEditing = Run->bPreviousAllowLibrary;
			FeelLibrary::ApplyWritePermission();
		}
		if (!Copy)
		{
			Test->AddError(TEXT("The project copy /Game/HeavyHit is missing."));
			return true;
		}
		Run->Blueprint = BuildQuickStartBlueprint(Copy, Run->PlayNode);
		AssetEditors->OpenEditorForAsset(Run->Blueprint.Get());
		if (const TSharedPtr<SWindow> Window = EditorWindow(Run->Blueprint.Get()))
		{
			Window->Resize(FVector2D(1900.0, 1060.0));
		}
		if (UEdGraphNode* PlayNode = Run->PlayNode.Get())
		{
			// Shows the Event Graph and centers it on the nodes.
			FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(PlayNode);
		}
		{
			// The compiler results panel only takes room from the graph in the picture.
			IAssetEditorInstance* Editor = AssetEditors->FindEditorForAsset(Run->Blueprint.Get(), false);
			const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
			for (const TCHAR* Id : { TEXT("CompilerResults"), TEXT("FindResults") })
			{
				if (const TSharedPtr<SDockTab> Panel = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(Id)) : nullptr)
				{
					Panel->RequestCloseTab();
				}
			}
		}
		Next();
		return false;

	case 3:
		if (Elapsed < 4.0)
		{
			return false;
		}
		if (!SaveWindow(EditorWindow(Run->Blueprint.Get()), TEXT("QS_Blueprint"), Test))
		{
			return false;
		}
		AssetEditors->CloseAllEditorsForAsset(Run->Blueprint.Get());
		{
			ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
			PlaySettings->SetPlayNumberOfClients(1);
			PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
			PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;
			FRequestPlaySessionParams Params;
			Params.EditorPlaySettings = PlaySettings;
			GEditor->RequestPlaySession(Params);
		}
		Next();
		return false;

	case 4:
	{
		UWorld* World = PIEWorld();
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		if (!Controller || !Controller->GetPawn() || Elapsed < 3.0)
		{
			return Elapsed > 90.0;
		}
		for (TActorIterator<AFeelSwitch> It(World); It; ++It)
		{
			It->Destroy();
		}
		World->SpawnActor<AFeelSwitch>(AFeelSwitch::StaticClass(), FTransform::Identity);
		Next();
		return false;
	}

	case 5:
		if (Elapsed < 2.5)
		{
			return false;
		}
		if (!SaveGame(TEXT("QS_SwitchStart"), Test))
		{
			return false;
		}
		Next();
		return false;

	case 6:
	{
		// Wait until the start card has faded, then play the recipe as the 1 key would.
		if (Elapsed < 9.0)
		{
			return false;
		}
		UWorld* World = PIEWorld();
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		if (IConsoleVariable* MotionBlur = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
		{
			Run->PreviousMotionBlur = MotionBlur->GetInt();
			MotionBlur->Set(0, ECVF_SetByConsole);
		}
		if (Controller && Copy)
		{
			UFeelBlueprintLibrary::PlayFeel(World, Copy, UFeelBlueprintLibrary::MakeFeelTargetFromActor(Controller->GetPawn()));
		}
		Next();
		return false;
	}

	case 7:
		if (Elapsed < 0.07)
		{
			return false;
		}
		{
			// The scene only: the moment matters more than the interface, which is outside the picture.
			UWorld* World = PIEWorld();
			FeelManualCapture::SaveGameScene(World ? World->GetGameViewport() : nullptr, TEXT("QS_Hit"), Density, Test);
		}
		if (IConsoleVariable* MotionBlur = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
		{
			MotionBlur->Set(Run->PreviousMotionBlur, ECVF_SetByConsole);
		}
		Next();
		return false;

	case 8:
		if (Elapsed < 1.5)
		{
			return false;
		}
		PressKey(EKeys::Tab);
		Next();
		return false;

	case 9:
		if (Elapsed < 1.2)
		{
			return false;
		}
		if (!SaveGame(TEXT("QS_SwitchOff"), Test))
		{
			return false;
		}
		GEditor->RequestEndPlayMap();
		Next();
		return false;

	default:
		if (Elapsed < 2.0)
		{
			return false;
		}
		{
			UContentBrowserSettings* Browser = GetMutableDefault<UContentBrowserSettings>();
			Browser->SetDisplayPluginFolders(Run->bPreviousPluginFolders);
			Browser->PostEditChange();
			FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelQuickStartShotsDiagnostic, "DiagFeel.ManualShotsQuickStart", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelQuickStartShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelQuickStartShots::FRun> Run = MakeShared<FeelQuickStartShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelQuickStartShotsCommand(Run, this));
	return true;
}

#endif
