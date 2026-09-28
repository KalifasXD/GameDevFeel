// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EdGraph/EdGraph.h"
#include "EdGraphNode_Comment.h"
#include "EdGraphSchema_K2.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "FeelBlueprintLibrary.h"
#include "FeelManualCapture.h"
#include "FeelMap.h"
#include "FeelPlayAndWaitAction.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSwitch.h"
#include "FeelTriggerComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Character.h"
#include "ISettingsModule.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_MakeMap.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_Self.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the pictures of the manual's chapter on playing recipes, saved to
 * Saved/FeelKit/Manual/Play_*.png at twice the screen's pixels and 1.25x application scale: four small Blueprint graphs
 * built in transient packages (target nodes, Play Feel with Context, handles, Play Feel and Wait), the Action/RPG Feel
 * Map, the Feel Maps project setting, the notifies of the Action/RPG combo montage, and floating Details windows for the
 * Platformer character's Feel Trigger, a Feel Input component with one binding, and the Feel Switch defaults. The
 * details views' remembered expansion and the project's Feel Maps list are changed for the pictures and put back.
 * Needs a rendering session, the /Game/HeavyHit copy and the demo content of GameFeelDev.
 */
namespace FeelPlayingShots
{
	constexpr float Density = 2.0f;

	const TCHAR* ExpansionTypes[] = { TEXT("FeelSettings"), TEXT("FeelMap"), TEXT("FeelTriggerComponent"), TEXT("FeelInputComponent"), TEXT("FeelSwitch") };

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TArray<TWeakObjectPtr<UBlueprint>> Graphs;
		TSharedPtr<SWindow> Floating;
		TMap<FString, TArray<FString>> SavedExpansion;
		FString SavedEventsCategory;
		TArray<TSoftObjectPtr<UFeelMap>> SavedFeelMaps;
		TWeakObjectPtr<UObject> InputComponent;
		bool bSaved = false;
	};

	/** Writes an expansion list the way the details view does: each item quoted, since paths can contain spaces. */
	void WriteExpansion(const TCHAR* Type, const TArray<FString>& Items)
	{
		TArray<FString> Quoted;
		for (const FString& Item : Items)
		{
			Quoted.Add(FString::Printf(TEXT("\"%s\""), *Item));
		}
		GConfig->SetSingleLineArray(TEXT("DetailPropertyExpansion"), Type, Quoted, GEditorPerProjectIni);
	}

	void SaveState(FRun& Run)
	{
		for (const TCHAR* Type : ExpansionTypes)
		{
			TArray<FString> Items;
			GConfig->GetSingleLineArray(TEXT("DetailPropertyExpansion"), Type, Items, GEditorPerProjectIni);
			Run.SavedExpansion.Add(Type, Items);
		}
		GConfig->GetString(TEXT("DetailCategories"), TEXT("FeelSettings.Events"), Run.SavedEventsCategory, GEditorPerProjectIni);
		Run.SavedFeelMaps = GetDefault<UFeelSettings>()->FeelMaps;
		Run.bSaved = true;
	}

	void RestoreState(const FRun& Run)
	{
		if (!Run.bSaved)
		{
			return;
		}
		for (const TPair<FString, TArray<FString>>& Pair : Run.SavedExpansion)
		{
			WriteExpansion(*Pair.Key, Pair.Value);
		}
		if (Run.SavedEventsCategory.IsEmpty())
		{
			GConfig->RemoveKey(TEXT("DetailCategories"), TEXT("FeelSettings.Events"), GEditorPerProjectIni);
		}
		else
		{
			GConfig->SetString(TEXT("DetailCategories"), TEXT("FeelSettings.Events"), *Run.SavedEventsCategory, GEditorPerProjectIni);
		}
		GetMutableDefault<UFeelSettings>()->FeelMaps = Run.SavedFeelMaps;
	}

	void Expand(const TCHAR* Type, const TArray<FString>& Items)
	{
		WriteExpansion(Type, Items);
	}

	/** A graph of an empty character Blueprint in a transient package; nodes are added by the caller. */
	UEdGraph* NewGraph(FRun& Run, const TCHAR* Name)
	{
		UPackage* Package = CreatePackage(*FString::Printf(TEXT("/Temp/FeelKitManualPlaying/%s"), Name));
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), Package, TEXT("BP_ThirdPersonCharacter"),
			BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
		Run.Graphs.Add(Blueprint);
		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		TArray<UEdGraphNode*> Existing = Graph->Nodes;
		for (UEdGraphNode* Node : Existing)
		{
			Graph->RemoveNode(Node);
		}
		return Graph;
	}

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

	UK2Node_CustomEvent* Event(UEdGraph* Graph, const TCHAR* Name, int32 X, int32 Y)
	{
		UK2Node_CustomEvent* Node = NewObject<UK2Node_CustomEvent>(Graph);
		Node->CustomFunctionName = Name;
		return Place(Graph, Node, X, Y);
	}

	void Comment(UEdGraph* Graph, const TCHAR* Text, int32 X, int32 Y, int32 W, int32 H)
	{
		UEdGraphNode_Comment* Node = NewObject<UEdGraphNode_Comment>(Graph);
		Node->NodeComment = Text;
		Graph->AddNode(Node, false, false);
		Node->CreateNewGuid();
		Node->NodePosX = X;
		Node->NodePosY = Y;
		Node->NodeWidth = W;
		Node->NodeHeight = H;
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

	void SetObject(UEdGraphNode* Node, const TCHAR* Pin, UObject* Value)
	{
		if (UEdGraphPin* Found = Node ? Node->FindPin(Pin) : nullptr)
		{
			GetDefault<UEdGraphSchema_K2>()->TrySetDefaultObject(*Found, Value);
		}
	}

	UEdGraphPin* Then(UEdGraphNode* Node)
	{
		return Node ? Node->FindPin(UEdGraphSchema_K2::PN_Then) : nullptr;
	}

	UEdGraphPin* Exec(UEdGraphNode* Node)
	{
		return Node ? Node->FindPin(UEdGraphSchema_K2::PN_Execute) : nullptr;
	}

	/** The five target makers; the actor one feeds a Play Feel node. Laid out wide and short, so the graph fits at 1:1. */
	UBlueprint* BuildTargets(FRun& Run, UFeelRecipe* Recipe)
	{
		UEdGraph* Graph = NewGraph(Run, TEXT("Targets"));
		UClass* Library = UFeelBlueprintLibrary::StaticClass();
		UK2Node_CustomEvent* Hit = Event(Graph, TEXT("OnHit"), -1000, -120);
		UK2Node_CallFunction* Play = Call(Graph, Library, TEXT("PlayFeel"), -720, -120);
		SetObject(Play, TEXT("Recipe"), Recipe);
		UK2Node_Self* Self = Place(Graph, NewObject<UK2Node_Self>(Graph), -1180, 60);
		UK2Node_CallFunction* FromActor = Call(Graph, Library, TEXT("MakeFeelTargetFromActor"), -1020, 40);
		Call(Graph, Library, TEXT("MakeFeelTargetFromComponent"), -420, -120);
		Call(Graph, Library, TEXT("MakeFeelTargetAtLocation"), -420, 20);
		Call(Graph, Library, TEXT("MakeFeelTargetFromLocalPlayerCamera"), -100, -120);
		Call(Graph, Library, TEXT("MakeFeelTargetFromWidget"), -100, 20);
		Link(Then(Hit), Exec(Play));
		Link(Self->FindPin(UEdGraphSchema_K2::PN_Self), FromActor->FindPin(TEXT("Actor")));
		Link(FromActor->GetReturnValuePin(), Play->FindPin(TEXT("Target")));
		Comment(Graph, TEXT("Targets"), -1220, -190, 1460, 370);
		FKismetEditorUtilities::CompileBlueprint(Run.Graphs.Last().Get());
		return Run.Graphs.Last().Get();
	}

	UBlueprint* BuildContext(FRun& Run, UFeelRecipe* Recipe)
	{
		UEdGraph* Graph = NewGraph(Run, TEXT("Context"));
		UClass* Library = UFeelBlueprintLibrary::StaticClass();
		UK2Node_CustomEvent* Hit = Event(Graph, TEXT("OnHit"), -1000, -150);
		UK2Node_CallFunction* Play = Call(Graph, Library, TEXT("PlayFeelWithContext"), -220, -150);
		SetObject(Play, TEXT("Recipe"), Recipe);
		UK2Node_Self* Self = Place(Graph, NewObject<UK2Node_Self>(Graph), -1150, 20);
		UK2Node_CallFunction* FromActor = Call(Graph, Library, TEXT("MakeFeelTargetFromActor"), -980, 0);
		UK2Node_MakeStruct* Make = NewObject<UK2Node_MakeStruct>(Graph);
		Make->StructType = FFeelPlayContext::StaticStruct();
		Place(Graph, Make, -560, 30);
		UK2Node_MakeMap* Map = Place(Graph, NewObject<UK2Node_MakeMap>(Graph), -980, 110);
		Link(Then(Hit), Exec(Play));
		Link(Self->FindPin(UEdGraphSchema_K2::PN_Self), FromActor->FindPin(TEXT("Actor")));
		Link(Self->FindPin(UEdGraphSchema_K2::PN_Self), Make->FindPin(TEXT("Instigator")));
		Link(FromActor->GetReturnValuePin(), Play->FindPin(TEXT("Target")));
		Link(Map->GetOutputPin(), Make->FindPin(TEXT("Parameters")));
		SetDefault(Map, TEXT("Key 0"), TEXT("Damage"));
		SetDefault(Map, TEXT("Value 0"), TEXT("50.0"));
		for (UEdGraphPin* Pin : Make->Pins)
		{
			if (Pin->Direction == EGPD_Output)
			{
				Link(Pin, Play->FindPin(TEXT("Context")));
			}
		}
		Comment(Graph, TEXT("Play a hit with a Damage value"), -1190, -220, 1300, 420);
		FKismetEditorUtilities::CompileBlueprint(Run.Graphs.Last().Get());
		return Run.Graphs.Last().Get();
	}

	UBlueprint* BuildHandles(FRun& Run, UFeelRecipe* Recipe)
	{
		UEdGraph* Graph = NewGraph(Run, TEXT("Handles"));
		UClass* Library = UFeelBlueprintLibrary::StaticClass();
		UK2Node_CustomEvent* Start = Event(Graph, TEXT("OnChargeStarted"), -1080, -150);
		UK2Node_CallFunction* Play = Call(Graph, Library, TEXT("PlayFeel"), -640, -150);
		SetObject(Play, TEXT("Recipe"), Recipe);
		UK2Node_Self* Self = Place(Graph, NewObject<UK2Node_Self>(Graph), -1240, 20);
		UK2Node_CallFunction* FromActor = Call(Graph, Library, TEXT("MakeFeelTargetFromActor"), -1060, 0);
		UK2Node_CustomEvent* Grow = Event(Graph, TEXT("OnChargeGrew"), -260, -150);
		UK2Node_CallFunction* Set = Call(Graph, Library, TEXT("SetFeelParameter"), 60, -150);
		SetDefault(Set, TEXT("ParameterName"), TEXT("Charge"));
		SetDefault(Set, TEXT("Value"), TEXT("0.5"));
		UK2Node_CustomEvent* Released = Event(Graph, TEXT("OnChargeReleased"), -260, 60);
		UK2Node_CallFunction* Release = Call(Graph, Library, TEXT("ReleaseFeel"), 60, 60);
		Link(Then(Start), Exec(Play));
		Link(Self->FindPin(UEdGraphSchema_K2::PN_Self), FromActor->FindPin(TEXT("Actor")));
		Link(FromActor->GetReturnValuePin(), Play->FindPin(TEXT("Target")));
		Link(Then(Grow), Exec(Set));
		Link(Then(Released), Exec(Release));
		Link(Play->GetReturnValuePin(), Set->FindPin(TEXT("Handle")));
		Link(Play->GetReturnValuePin(), Release->FindPin(TEXT("Handle")));
		Comment(Graph, TEXT("One handle: play, change a parameter, release"), -1280, -220, 1640, 420);
		FKismetEditorUtilities::CompileBlueprint(Run.Graphs.Last().Get());
		return Run.Graphs.Last().Get();
	}

	UBlueprint* BuildWait(FRun& Run, UFeelRecipe* Recipe)
	{
		UEdGraph* Graph = NewGraph(Run, TEXT("Wait"));
		UClass* Library = UFeelBlueprintLibrary::StaticClass();
		UK2Node_CustomEvent* Unlock = Event(Graph, TEXT("OnUnlock"), -900, -120);
		UK2Node_AsyncAction* Wait = NewObject<UK2Node_AsyncAction>(Graph);
		Wait->InitializeProxyFromFunction(UFeelPlayAndWaitAction::StaticClass()->FindFunctionByName(TEXT("PlayFeelAndWait")));
		Place(Graph, Wait, -460, -120);
		SetObject(Wait, TEXT("Recipe"), Recipe);
		UK2Node_Self* Self = Place(Graph, NewObject<UK2Node_Self>(Graph), -1060, 50);
		UK2Node_CallFunction* FromActor = Call(Graph, Library, TEXT("MakeFeelTargetFromActor"), -880, 30);
		Link(Then(Unlock), Exec(Wait));
		Link(Self->FindPin(UEdGraphSchema_K2::PN_Self), FromActor->FindPin(TEXT("Actor")));
		Link(FromActor->GetReturnValuePin(), Wait->FindPin(TEXT("Target")));
		Comment(Graph, TEXT("Play, then continue when the recipe ends"), -1100, -190, 980, 340);
		FKismetEditorUtilities::CompileBlueprint(Run.Graphs.Last().Get());
		return Run.Graphs.Last().Get();
	}

	void OpenGraph(UBlueprint* Blueprint)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Blueprint))
		{
			// Wider than one screen (the height cannot be), so the wide graphs fit at 1:1.
			Window->Resize(FVector2D(3400.0, 1060.0));
		}
		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		for (UEdGraphNode* Node : Graph ? Graph->Nodes : TArray<TObjectPtr<UEdGraphNode>>())
		{
			if (Node->IsA<UEdGraphNode_Comment>())
			{
				FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Node);
			}
		}
		IAssetEditorInstance* Editor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Blueprint, false);
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		for (const TCHAR* Id : { TEXT("CompilerResults"), TEXT("FindResults") })
		{
			if (const TSharedPtr<SDockTab> Panel = Tabs.IsValid() ? Tabs->FindExistingLiveTab(FTabId(Id)) : nullptr)
			{
				Panel->RequestCloseTab();
			}
		}
	}

	void CloseEditor(UObject* Asset)
	{
		if (Asset)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Asset);
		}
	}

	TSharedPtr<SWindow> OpenDetails(UObject* Object, const FVector2D& Size)
	{
		FPropertyEditorModule& Editor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
		const TSharedRef<SWindow> Window = Editor.CreateFloatingDetailsView({ Object }, false);
		Window->Resize(Size);
		return Window;
	}

	void CloseDetails(FRun& Run)
	{
		if (Run.Floating.IsValid())
		{
			Run.Floating->RequestDestroyWindow();
			Run.Floating.Reset();
		}
	}

	TSharedPtr<SWindow> SettingsWindow()
	{
		const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TEXT("ProjectSettings")));
		return Tab.IsValid() ? Tab->GetParentWindow() : nullptr;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelPlayingShotsCommand, TSharedRef<FeelPlayingShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelPlayingShotsCommand::Update()
{
	using namespace FeelPlayingShots;
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
	UFeelRecipe* HeavyHit = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/HeavyHit.HeavyHit"));
	UFeelRecipe* ScalableHit = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Impact/FR_Impact_ScalableHit.FR_Impact_ScalableHit"));
	UFeelRecipe* ChargeUp = LoadObject<UFeelRecipe>(nullptr, TEXT("/FeelKit/Library/Power/FR_Power_ChargeUp.FR_Power_ChargeUp"));
	static const TCHAR* GraphNames[] = { TEXT("Play_Targets"), TEXT("Play_Context"), TEXT("Play_Handles"), TEXT("Play_Wait") };

	// Stages 0 to 7: the four graphs, each opened (even stage) and saved (odd stage).
	if (Run->Stage < 8)
	{
		const int32 Index = Run->Stage / 2;
		if (Run->Stage % 2 == 0)
		{
			UBlueprint* Blueprint = Index == 0 ? BuildTargets(*Run, HeavyHit)
				: Index == 1 ? BuildContext(*Run, ScalableHit)
				: Index == 2 ? BuildHandles(*Run, ChargeUp)
				: BuildWait(*Run, HeavyHit);
			OpenGraph(Blueprint);
			Next();
			return false;
		}
		if (Elapsed < 4.0)
		{
			return false;
		}
		UBlueprint* Blueprint = Run->Graphs.IsValidIndex(Index) ? Run->Graphs[Index].Get() : nullptr;
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Blueprint), GraphNames[Index], Density, Test))
		{
			return false;
		}
		CloseEditor(Blueprint);
		Next();
		return false;
	}

	switch (Run->Stage)
	{
	case 8:
	{
		SaveState(*Run);
		Expand(TEXT("FeelMap"), { TEXT("Object.Feel Map.Entries"), TEXT("Object.Feel Map.Entries.Entries[0]"), TEXT("Object.Feel Map.Entries.Entries[1]"),
			TEXT("Object.Feel Map.Entries.Entries[2]"), TEXT("Object.Feel Map.Entries.Entries[3]") });
		UFeelMap* Map = LoadObject<UFeelMap>(nullptr, TEXT("/Game/FeelKitDemos/ActionRPG/FM_ARPG.FM_ARPG"));
		if (!Map)
		{
			Test->AddError(TEXT("FM_ARPG is missing."));
			return true;
		}
		Run->Floating = OpenDetails(Map, FVector2D(1000.0, 1000.0));
		Next();
		return false;
	}

	case 9:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(Run->Floating, TEXT("Play_FeelMap"), Density, Test))
		{
			return false;
		}
		CloseDetails(*Run);
		{
			// Only the demo's map, as a project would have it (this project also lists a test map).
			UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
			Settings->FeelMaps.Reset();
			Settings->FeelMaps.Add(TSoftObjectPtr<UFeelMap>(FSoftObjectPath(TEXT("/Game/FeelKitDemos/ActionRPG/FM_ARPG.FM_ARPG"))));
			for (const TCHAR* Key : { TEXT("FeelSettings.Camera"), TEXT("FeelSettings.Playback"), TEXT("FeelSettings.Comfort") })
			{
				GConfig->SetBool(TEXT("DetailCategories"), Key, false, GEditorPerProjectIni);
			}
			GConfig->SetBool(TEXT("DetailCategories"), TEXT("FeelSettings.Events"), true, GEditorPerProjectIni);
			Expand(TEXT("FeelSettings"), { TEXT("Object.Events.FeelMaps") });
			FModuleManager::LoadModuleChecked<ISettingsModule>(TEXT("Settings")).ShowViewer(TEXT("Project"), TEXT("Plugins"), TEXT("FeelSettings"));
			if (const TSharedPtr<SWindow> Window = SettingsWindow())
			{
				Window->Resize(FVector2D(1500.0, 1060.0));
			}
		}
		Next();
		return false;

	case 10:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(SettingsWindow(), TEXT("Play_ProjectFeelMaps"), Density, Test))
		{
			return false;
		}
		if (const TSharedPtr<SDockTab> Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(FTabId(TEXT("ProjectSettings"))))
		{
			Tab->RequestCloseTab();
		}
		{
			UObject* Montage = LoadObject<UObject>(nullptr, TEXT("/Game/Variant_Combat/Anims/AM_ComboAttack.AM_ComboAttack"));
			if (Montage)
			{
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Montage);
				if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Montage))
				{
					Window->Resize(FVector2D(1900.0, 1060.0));
				}
			}
		}
		Next();
		return false;

	case 11:
	{
		if (Elapsed < 8.0)
		{
			return false;
		}
		UObject* Montage = LoadObject<UObject>(nullptr, TEXT("/Game/Variant_Combat/Anims/AM_ComboAttack.AM_ComboAttack"));
		if (!SaveWindow(FeelManualCapture::AssetEditorWindow(Montage), TEXT("Play_Montage"), Density, Test))
		{
			return false;
		}
		CloseEditor(Montage);
		UBlueprint* Character = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Variant_Platforming/Blueprints/BP_PlatformingCharacter.BP_PlatformingCharacter"));
		UObject* Trigger = nullptr;
		if (Character && Character->SimpleConstructionScript)
		{
			for (USCS_Node* Node : Character->SimpleConstructionScript->GetAllNodes())
			{
				if (Node && Node->ComponentTemplate && Node->ComponentTemplate->IsA<UFeelTriggerComponent>())
				{
					Trigger = Node->ComponentTemplate;
				}
			}
		}
		if (!Trigger)
		{
			Test->AddError(TEXT("The Platformer character has no Feel Trigger."));
			return true;
		}
		Expand(TEXT("FeelTriggerComponent"), { TEXT("Object.Feel.Triggers"), TEXT("Object.Feel.Triggers.Triggers[0]"), TEXT("Object.Feel.Triggers.Triggers[3]") });
		Run->Floating = OpenDetails(Trigger, FVector2D(1100.0, 1060.0));
		Next();
		return false;
	}

	case 12:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(Run->Floating, TEXT("Play_Trigger"), Density, Test))
		{
			return false;
		}
		CloseDetails(*Run);
		{
			// A Feel Input component with one binding, made from its class by name (FeelEditor does not link FeelEnhancedInput).
			UClass* InputClass = FindObject<UClass>(nullptr, TEXT("/Script/FeelEnhancedInput.FeelInputComponent"));
			if (!InputClass)
			{
				Test->AddError(TEXT("FeelInputComponent class not found."));
				return true;
			}
			UObject* Component = NewObject<UObject>(GetTransientPackage(), InputClass, TEXT("FeelInput"));
			Component->AddToRoot();
			Run->InputComponent = Component;
			if (FProperty* Bindings = InputClass->FindPropertyByName(TEXT("Bindings")))
			{
				Bindings->ImportText_InContainer(TEXT("((Action=\"/Game/Variant_Combat/Input/Actions/IA_ChargedAttack.IA_ChargedAttack\",PlayOn=Started,Recipe=\"/FeelKit/Library/Power/FR_Power_ChargeUp.FR_Power_ChargeUp\",Intensity=1.000000,bEndWhenInputEnds=True,bOncePerPress=True))"),
					Component, Component, PPF_None);
			}
			Expand(TEXT("FeelInputComponent"), { TEXT("Object.Feel.Bindings"), TEXT("Object.Feel.Bindings.Bindings[0]") });
			Run->Floating = OpenDetails(Component, FVector2D(1100.0, 700.0));
		}
		Next();
		return false;

	case 13:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(Run->Floating, TEXT("Play_Input"), Density, Test))
		{
			return false;
		}
		CloseDetails(*Run);
		if (UObject* Component = Run->InputComponent.Get())
		{
			Component->RemoveFromRoot();
		}
		Expand(TEXT("FeelSwitch"), { TEXT("Object.Feel Switch.SwitchKeys"), TEXT("Object.Feel Switch.Feel Switch|Display"), TEXT("Object.Feel Switch.Feel Switch|Comfort Menu"),
			TEXT("Object.Feel Switch.ComfortMenuKeys") });
		Run->Floating = OpenDetails(GetMutableDefault<AFeelSwitch>(), FVector2D(1100.0, 1060.0));
		Next();
		return false;

	case 14:
		if (Elapsed < 3.0)
		{
			return false;
		}
		if (!SaveWindow(Run->Floating, TEXT("Play_Switch"), Density, Test))
		{
			return false;
		}
		CloseDetails(*Run);
		Next();
		return false;

	default:
		if (Elapsed < 2.0)
		{
			return false;
		}
		RestoreState(*Run);
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelPlayingShotsDiagnostic, "DiagFeel.ManualShotsPlaying", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelPlayingShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelPlayingShots::FRun> Run = MakeShared<FeelPlayingShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelPlayingShotsCommand(Run, this));
	return true;
}

#endif
