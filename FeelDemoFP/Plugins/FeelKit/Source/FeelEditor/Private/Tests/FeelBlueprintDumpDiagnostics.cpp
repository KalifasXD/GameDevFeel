// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Engine/Blueprint.h"
#include "Misc/CommandLine.h"

/**
 * Diagnostic (filter DiagFeel): prints every node of a Blueprint's graphs with pin defaults and links, so a test setup can be
 * read without opening the editor. Parameter: the Blueprint object path.
 */
class FFeelBlueprintDumpDiagnostic : public FAutomationTestBase
{
public:
	FFeelBlueprintDumpDiagnostic(const FString& InName) : FAutomationTestBase(InName, false) {}
	virtual EAutomationTestFlags GetTestFlags() const override { return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter; }
	virtual bool IsStressTest() const { return false; }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }
	virtual FString GetTestSourceFileName() const override { return __FILE__; }
	virtual int32 GetTestSourceFileLine() const override { return __LINE__; }

protected:
	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override
	{
		OutBeautifiedNames.Add(TEXT("DiagFeel.BlueprintDump"));
		OutTestCommands.Add(FString());
	}
	virtual bool RunTest(const FString& Parameters) override;
	virtual FString GetBeautifiedTestName() const override { return TEXT("DiagFeel.BlueprintDump"); }

private:
	void DumpBlueprint(const UBlueprint* Blueprint);
};

namespace
{
	FFeelBlueprintDumpDiagnostic FeelBlueprintDumpDiagnosticInstance(TEXT("FFeelBlueprintDumpDiagnostic"));
}
bool FFeelBlueprintDumpDiagnostic::RunTest(const FString& Parameters)
{
	// -FeelDump=<object path> picks what to dump; a montage prints its sections and notifies instead of graphs.
	FString Path = TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter");
	FParse::Value(FCommandLine::Get(), TEXT("FeelDump="), Path);
	TArray<FString> Paths;
	Path.ParseIntoArray(Paths, TEXT(","));
	for (const FString& OnePath : Paths)
	{
		if (const UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *OnePath))
		{
			AddInfo(FString::Printf(TEXT("BPDUMP montage %s length %.3f"), *OnePath, Montage->GetPlayLength()));
			for (const FCompositeSection& Section : Montage->CompositeSections)
			{
				AddInfo(FString::Printf(TEXT("BPDUMP     section %s at %.3f next %s"), *Section.SectionName.ToString(), Section.GetTime(), *Section.NextSectionName.ToString()));
			}
			for (const FAnimNotifyEvent& Notify : Montage->Notifies)
			{
				AddInfo(FString::Printf(TEXT("BPDUMP     notify %s at %.3f duration %.3f class %s"), *Notify.NotifyName.ToString(), Notify.GetTime(), Notify.GetDuration(),
					Notify.Notify ? *Notify.Notify->GetClass()->GetName() : (Notify.NotifyStateClass ? *Notify.NotifyStateClass->GetClass()->GetName() : TEXT("-"))));
			}
		}
		else if (const UBlueprint* OneBlueprint = LoadObject<UBlueprint>(nullptr, *OnePath))
		{
			AddInfo(FString::Printf(TEXT("BPDUMP blueprint %s"), *OnePath));
			// A Widget Blueprint also lists its widgets: name, class and parent.
			if (const UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(OneBlueprint->GeneratedClass))
			{
				if (const UWidgetTree* Tree = WidgetClass->GetWidgetTreeArchetype())
				{
					Tree->ForEachWidget([this](UWidget* Widget)
					{
						const UPanelWidget* Parent = Widget->GetParent();
						AddInfo(FString::Printf(TEXT("BPDUMP     widget %s (%s) in %s"), *Widget->GetName(), *Widget->GetClass()->GetName(), Parent ? *Parent->GetName() : TEXT("root")));
					});
				}
			}
			DumpBlueprint(OneBlueprint);
		}
		else
		{
			AddError(FString::Printf(TEXT("BPDUMP not found: %s"), *OnePath));
		}
	}
	return true;
}

void FFeelBlueprintDumpDiagnostic::DumpBlueprint(const UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return;
	}

	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (const UEdGraph* Graph : Graphs)
	{
		for (const UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node)
			{
				continue;
			}
			AddInfo(FString::Printf(TEXT("BPDUMP [%s] %s (%s)"), *Graph->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(), *Node->GetName()));
			for (const UEdGraphPin* Pin : Node->Pins)
			{
				FString Links;
				for (const UEdGraphPin* Linked : Pin->LinkedTo)
				{
					Links += FString::Printf(TEXT(" -> %s.%s"), *Linked->GetOwningNode()->GetName(), *Linked->PinName.ToString());
				}
				const FString Default = Pin->DefaultObject ? Pin->DefaultObject->GetName() : Pin->DefaultValue;
				if (!Default.IsEmpty() || !Links.IsEmpty())
				{
					AddInfo(FString::Printf(TEXT("BPDUMP     %s %s = '%s'%s"), Pin->Direction == EGPD_Input ? TEXT("in ") : TEXT("out"), *Pin->PinName.ToString(), *Default, *Links));
				}
			}
		}
	}
}

#endif
