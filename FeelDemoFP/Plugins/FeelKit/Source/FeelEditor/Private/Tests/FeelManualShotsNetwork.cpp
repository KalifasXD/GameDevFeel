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
#include "FeelRecipe.h"
#include "FeelReplicationComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Character.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_Self.h"
#include "K2Node_VariableGet.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/App.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * Diagnostic (filter DiagFeel): the picture of the manual's multiplayer chapter, saved to Saved/FeelKit/Manual/Net_Graph.png
 * at twice the screen's pixels and 1.25x application scale: a character Blueprint in a transient package with a Feel
 * Replication component, whose event graph plays the project's HeavyHit through Play Feel Networked. Needs a rendering
 * session and the /Game/HeavyHit copy.
 */
namespace FeelNetworkShots
{
	constexpr float Density = 2.0f;

	struct FRun
	{
		int32 Stage = 0;
		double StageStart = 0.0;
		float PreviousScale = 1.0f;
		TWeakObjectPtr<UBlueprint> Blueprint;
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

	UBlueprint* BuildGraph(UFeelRecipe* Recipe)
	{
		UPackage* Package = CreatePackage(TEXT("/Temp/FeelKitManualNetwork/BP_ThirdPersonCharacter"));
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ACharacter::StaticClass(), Package, TEXT("BP_ThirdPersonCharacter"),
			BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
		USCS_Node* Component = Blueprint->SimpleConstructionScript->CreateNode(UFeelReplicationComponent::StaticClass(), TEXT("FeelReplication"));
		Blueprint->SimpleConstructionScript->AddNode(Component);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);

		UEdGraph* Graph = FBlueprintEditorUtils::FindEventGraph(Blueprint);
		TArray<UEdGraphNode*> Existing = Graph->Nodes;
		for (UEdGraphNode* Node : Existing)
		{
			Graph->RemoveNode(Node);
		}
		const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

		UK2Node_CustomEvent* Hit = NewObject<UK2Node_CustomEvent>(Graph);
		Hit->CustomFunctionName = TEXT("OnHitLanded");
		Place(Graph, Hit, -1120, -140);

		UK2Node_VariableGet* Replication = NewObject<UK2Node_VariableGet>(Graph);
		Replication->VariableReference.SetSelfMember(TEXT("FeelReplication"));
		Place(Graph, Replication, -1120, 20);

		UK2Node_CallFunction* Play = NewObject<UK2Node_CallFunction>(Graph);
		Play->SetFromFunction(UFeelReplicationComponent::StaticClass()->FindFunctionByName(TEXT("PlayFeelNetworked")));
		Place(Graph, Play, -620, -140);

		UK2Node_Self* Self = Place(Graph, NewObject<UK2Node_Self>(Graph), -1120, 140);
		UK2Node_CallFunction* FromActor = NewObject<UK2Node_CallFunction>(Graph);
		FromActor->SetFromFunction(UFeelBlueprintLibrary::StaticClass()->FindFunctionByName(TEXT("MakeFeelTargetFromActor")));
		Place(Graph, FromActor, -960, 120);

		Schema->TryCreateConnection(Hit->FindPin(UEdGraphSchema_K2::PN_Then), Play->GetExecPin());
		Schema->TryCreateConnection(Replication->GetValuePin(), Play->FindPin(UEdGraphSchema_K2::PN_Self));
		Schema->TryCreateConnection(Self->FindPin(UEdGraphSchema_K2::PN_Self), FromActor->FindPin(TEXT("Actor")));
		Schema->TryCreateConnection(FromActor->GetReturnValuePin(), Play->FindPin(TEXT("Target")));
		if (UEdGraphPin* RecipePin = Play->FindPin(TEXT("Recipe")))
		{
			Schema->TrySetDefaultObject(*RecipePin, Recipe);
		}
		if (UEdGraphPin* Intensity = Play->FindPin(TEXT("Intensity")))
		{
			Schema->TrySetDefaultValue(*Intensity, TEXT("1.0"));
		}

		UEdGraphNode_Comment* Note = NewObject<UEdGraphNode_Comment>(Graph);
		Note->NodeComment = TEXT("Every machine plays the hit, each with its own comfort settings");
		Graph->AddNode(Note, false, false);
		Note->CreateNewGuid();
		Note->NodePosX = -1160;
		Note->NodePosY = -210;
		Note->NodeWidth = 1000;
		Note->NodeHeight = 440;

		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return Blueprint;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FFeelNetworkShotsCommand, TSharedRef<FeelNetworkShots::FRun>, Run, FAutomationTestBase*, Test);

bool FFeelNetworkShotsCommand::Update()
{
	using namespace FeelNetworkShots;
	const double Now = FPlatformTime::Seconds();
	if (Run->StageStart == 0.0)
	{
		Run->StageStart = Now;
	}
	const double Elapsed = Now - Run->StageStart;

	switch (Run->Stage)
	{
	case 0:
	{
		UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, TEXT("/Game/HeavyHit.HeavyHit"));
		if (!Recipe)
		{
			Test->AddError(TEXT("The project copy /Game/HeavyHit is missing."));
			return true;
		}
		UBlueprint* Blueprint = BuildGraph(Recipe);
		Run->Blueprint = Blueprint;
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Blueprint);
		if (const TSharedPtr<SWindow> Window = FeelManualCapture::AssetEditorWindow(Blueprint))
		{
			// Wider than one screen (the height cannot be), so the graph fits at 1:1.
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
		++Run->Stage;
		Run->StageStart = Now;
		return false;
	}

	case 1:
		if (Elapsed < 4.0 || !FeelManualCapture::SaveWindow(FeelManualCapture::AssetEditorWindow(Run->Blueprint.Get()), TEXT("Net_Graph"), Density, Test))
		{
			return false;
		}
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(Run->Blueprint.Get());
		FSlateApplication::Get().SetApplicationScale(Run->PreviousScale);
		return true;

	default:
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelNetworkShotsDiagnostic, "DiagFeel.ManualShotsNetwork", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelNetworkShotsDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender() || !FSlateApplication::IsInitialized() || !GEditor)
	{
		AddError(TEXT("Needs a rendering session."));
		return false;
	}
	const TSharedRef<FeelNetworkShots::FRun> Run = MakeShared<FeelNetworkShots::FRun>();
	Run->PreviousScale = FSlateApplication::Get().GetApplicationScale();
	FSlateApplication::Get().SetApplicationScale(1.25f);
	ADD_LATENT_AUTOMATION_COMMAND(FFeelNetworkShotsCommand(Run, this));
	return true;
}

#endif
