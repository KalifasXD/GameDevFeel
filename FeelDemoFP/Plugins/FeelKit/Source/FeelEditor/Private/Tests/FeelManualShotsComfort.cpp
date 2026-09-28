// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EdGraph/EdGraph.h"
#include "EdGraphNode_Comment.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/GameViewportClient.h"
#include "FeelBlueprintLibrary.h"
#include "FeelComfortAudit.h"
#include "FeelComfortMenu.h"
#include "FeelComfortSubsystem.h"
#include "FeelManualCapture.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "FeelSettings.h"
#include "Logging/MessageLog.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/App.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's comfort chapter, saved to Saved/FeelKit/Manual/Comfort_*.png
 * at twice the screen's pixels and 1.25x application scale: a Blueprint graph with the comfort nodes (built in a transient
 * package), a transient copy of FR_Dread_JumpScare with its essential flash track selected and only its Comfort category
 * open, the Comfort Audit page of the Message Log, and the comfort menu during Play In Editor at the project defaults.
 * Needs a rendering session.
 */
namespace FeelComfortShots
{
	constexpr float Density = 2.0f;

	const TCHAR* RecipeCategories[] = { TEXT("FeelRecipe.Recipe"), TEXT("FeelRecipe.Parameters"), TEXT("FeelRecipe.Sustain"), TEXT("FeelRecipe.Library"),
		TEXT("FeelRecipe.Preview"), TEXT("FeelRecipe.Parameter Mappings"), TEXT("FeelRecipe.Randomness"), TEXT("FeelRecipe.Conditions"), TEXT("FeelRecipe.Comfort") };

	struct FRun
	{
		TMap<FString, FString> SavedCategories;
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TWeakObjectPtr<UBlueprint> Graph;
		TWeakObjectPtr<UFeelRecipe> JumpScare;
	};

	template <typename NodeType>
	NodeType* Place(UEdGraph* Graph, NodeType* Node, int32 X, int32 Y)
	{
		Graph->AddNode(Node, false, false);
		Node->CreateNewGuid();
		Node->PostPlacedNewNode();
		Node->AllocateDefaultPins();
		Node->NodePosX = X;
		Node->NodePosY = Y;
		return Node;
	}

	UK2Node_CallFunction* Call(UEdGraph* Graph, UClass* Class, const TCHAR* Function, int32 X, int32 Y)
	{
		UK2Node_CallFunction* Node = NewObject<UK2Node_CallFunction>(Graph);
		Node->SetFromFunction(Class->FindFunctionByName(Function));
		return Place(Graph, Node, X, Y);
	}

	void Link(UEdGraphPin* A, UEdGraphPin* B)
	{
		if (A && B)
		{
			GetDefault<UEdGraphSchema_K2>()->TryCreateConnection(A, B);
		}
	}

	void SetDefault(UEdGraphNode* Node, const TCHAR* Pin, const FString& Value)
	{
		if (UEdGraphPin* Found = Node ? Node->FindPin(Pin) : nullptr)
		{
			GetDefault<UEdGraphSchema_K2>()->TrySetDefaultValue(*Found, Value);
		}
	}

	UBlueprint* BuildComfortGraph()
	{
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualComfort/BP_ThirdPersonCharacter"));
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), Package, TEXT("BP_ThirdPersonCharacter"),
			BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		TArray<UEdGraphNode*> Existing = Graph->Nodes;
		for (UEdGraphNode* Node : Existing)
		{
			Graph->RemoveNode(Node);
		}
		UK2Node_CustomEvent* Event = NewObject<UK2Node_CustomEvent>(Graph);
		Event->CustomFunctionName = TEXT("OnReducedMotionChosen");
		Place(Graph, Event, -1100, -120);
		UK2Node_CallFunction* Controller = Call(Graph, UGameplayStatics::StaticClass(), TEXT("GetPlayerController"), -1100, 60);
		UK2Node_CallFunction* Comfort = Call(Graph, UFeelBlueprintLibrary::StaticClass(), TEXT("GetFeelComfort"), -800, 60);
		UK2Node_CallFunction* Preset = Call(Graph, UFeelComfortSubsystem::StaticClass(), TEXT("ApplyComfortPreset"), -460, -120);
		SetDefault(Preset, TEXT("Preset"), TEXT("ReducedMotion"));
		UK2Node_CallFunction* Group = Call(Graph, UFeelComfortSubsystem::StaticClass(), TEXT("SetComfortGroupScale"), -80, -120);
		SetDefault(Group, TEXT("Group"), TEXT("CameraShake"));
		SetDefault(Group, TEXT("Scale"), TEXT("0.5"));
		Link(Event->FindPin(UEdGraphSchema_K2::PN_Then), Preset->GetExecPin());
		Link(Preset->GetThenPin(), Group->GetExecPin());
		Link(Controller->GetReturnValuePin(), Comfort->FindPin(TEXT("PlayerController")));
		Link(Comfort->GetReturnValuePin(), Preset->FindPin(UEdGraphSchema_K2::PN_Self));
		Link(Comfort->GetReturnValuePin(), Group->FindPin(UEdGraphSchema_K2::PN_Self));
		UEdGraphNode_Comment* Note = NewObject<UEdGraphNode_Comment>(Graph);
		Note->NodeComment = TEXT("Reduced motion, then a little less shake");
		Graph->AddNode(Note, false, false);
		Note->CreateNewGuid();
		Note->NodePosX = -1140;
		Note->NodePosY = -190;
		Note->NodeWidth = 1400;
		Note->NodeHeight = 380;
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return Blueprint;
	}

	TSharedPtr<SWindow> OpenEditor(UObject* Asset, const FVector2D& Size)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset);
		const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Asset);
		if (Window.IsValid())
		{
			Window->Resize(Size);
		}
		return Window;
	}

	void CloseEditor(UObject* Asset)
	{
		if (Asset)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Asset);
		}
	}

	void CloseEditorTabs(UObject* Asset, std::initializer_list<const TCHAR*> Ids)
	{
		IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		for (const TCHAR* Id : Ids)
		{
			if (const TSharedPtr<SDockTab> Tab = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(Id)) : nullptr)
			{
				Tab->RequestCloseTab();
			}
		}
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelComfortShotsCommand, TSharedRef<FeelComfortShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelComfortShotsCommand::Update()
{
	using namespace FeelComfortShots;
	using FeelManualCapture::SaveWindow;
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

	switch (Run->Stage)
	{
	case 0:
	{
		UBlueprint* Blueprint = BuildComfortGraph();
		Run->Graph = Blueprint;
		OpenEditor(Blueprint, FVector2D(3400.0, 1060.0));
		CloseEditorTabs(Blueprint, { TEXT("CompilerResults"), TEXT("FindResults") });
		if (UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint))
		{
			for (UEdGraphNode* Node : Graph->Nodes)
			{
				if (Node->IsA<UEdGraphNode_Comment>())
				{
					FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Node);
				}
			}
		}
		Next();
		return false;
	}

	case 1:
		if (Elapsed < 4.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Graph.Get()), TEXT("Comfort_Nodes"), Density, Test))
		{
			return false;
		}
		CloseEditor(Run->Graph.Get());
		{
			UFeelRecipe* Library = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Dread/FR_Dread_JumpScare.FR_Dread_JumpScare"));
			if (!Library)
			{
				Test->AddError(TEXT("FR_Dread_JumpScare is missing."));
				return true;
			}
			// Only the track and its Comfort category open, so the picture shows the essential settings.
			for (const TCHAR* Key : RecipeCategories)
			{
				FString Value;
				GConfig->GetString(TEXT("DetailCategories"), Key, Value, GEditorPerProjectIni);
				Run->SavedCategories.Add(Key, Value);
				GConfig->SetBool(TEXT("DetailCategories"), Key, FCString::Strcmp(Key, TEXT("FeelRecipe.Comfort")) == 0, GEditorPerProjectIni);
			}
			UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualComfort/JumpScare"));
			UFeelRecipe* Copy = DuplicateObject<UFeelRecipe>(Library, Package, TEXT("JumpScare"));
			Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
			Run->JumpScare = Copy;
			OpenEditor(Copy, FVector2D(1900.0, 1060.0));
		}
		Next();
		return false;

	case 2:
	{
		if (Elapsed < 1.0)
		{
			return false;
		}
		UFeelRecipe* Copy = Run->JumpScare.Get();
		CloseEditorTabs(Copy, { TEXT("FeelRecipeEditor_Intensity") });
		if (const TSharedPtr<FFeelRecipeEditorState> State = FFeelRecipeEditorState::FindOpenState(Copy))
		{
			for (int32 Index = 0; Copy && Index < Copy->Tracks.Num(); ++Index)
			{
				if (Cast<UFeelStep_ScreenFlash>(Copy->Tracks[Index].Step))
				{
					State->SetSelectedTrack(Index);
					break;
				}
			}
		}
		Next();
		return false;
	}

	case 3:
		if (Elapsed < 5.0)
		{
			return false;
		}
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Run->JumpScare.Get()), TEXT("Comfort_Essential"), Density, Test))
		{
			return false;
		}
		CloseEditor(Run->JumpScare.Get());
		for (const TPair<FString, FString>& Pair : Run->SavedCategories)
		{
			if (Pair.Value.IsEmpty())
			{
				GConfig->RemoveKey(TEXT("DetailCategories"), *Pair.Key, GEditorPerProjectIni);
			}
			else
			{
				GConfig->SetString(TEXT("DetailCategories"), *Pair.Key, *Pair.Value, GEditorPerProjectIni);
			}
		}
		Next();
		return false;

	case 4:
		FFeelComfortAudit::RunProjectAudit();
		FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TEXT("MessageLog")));
		FMessageLog(TEXT("FeelKitComfortAudit")).Open(EMessageSeverity::Info, true);
		Next();
		return false;

	case 5:
	{
		if (Elapsed < 3.0)
		{
			return false;
		}
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TEXT("MessageLog")));
		if (Elapsed < 3.5 && Tab.IsValid() && Tab->GetParentWindow().IsValid())
		{
			// Select the audit's page in the list of logs.
			FeelManualCapture::Click(FeelManualCapture::FindText(Tab->GetParentWindow().ToSharedRef(), TEXT("FeelKit Comfort Audit"), true));
			return false;
		}
		if (Elapsed < 5.0)
		{
			return false;
		}
		if (!SaveWindow(Tab.IsValid() ? Tab->GetParentWindow() : nullptr, TEXT("Comfort_Audit"), Density, Test))
		{
			return false;
		}
		if (Tab.IsValid())
		{
			Tab->RequestCloseTab();
		}
		ULevelEditorPlaySettings* PlaySettings = NewObject<ULevelEditorPlaySettings>(GetTransientPackage());
		PlaySettings->SetPlayNumberOfClients(1);
		PlaySettings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		PlaySettings->LastExecutedPlayModeType = EPlayModeType::PlayMode_InEditorFloating;
		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = PlaySettings;
		GEditor->RequestPlaySession(Params);
		Next();
		return false;
	}

	case 6:
	{
		UWorld* World = FeelManualCapture::PIEWorld();
		APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
		if (!Controller || !Controller->GetPawn() || Elapsed < 3.0)
		{
			return Elapsed > 90.0;
		}
		if (UFeelComfortSubsystem* Comfort = UFeelBlueprintLibrary::GetFeelComfort(Controller))
		{
			// The menu shows the project defaults, not what an earlier play session saved for this player.
			Comfort->SetComfortScalesWithoutSaving(GetDefault<UFeelSettings>()->DefaultComfortScales);
		}
		UFeelComfortMenu::ShowFeelComfortMenu(Controller, nullptr, true);
		Next();
		return false;
	}

	case 7:
	{
		if (Elapsed < 2.5)
		{
			return false;
		}
		UWorld* World = FeelManualCapture::PIEWorld();
		if (!FeelManualCapture::SaveGameView(World ? World->GetGameViewport() : nullptr, TEXT("Comfort_Menu"), Density, Test))
		{
			return false;
		}
		Next();
		return false;
	}

	case 8:
		if (Elapsed < 2.0)
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
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelComfortShotsDiagnostic, "DiagFeel.ManualShotsComfort", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelComfortShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelComfortShots::FRun> Run = MakeShared<FeelComfortShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelComfortShotsCommand(Run, this));
	return true;
}

#endif
