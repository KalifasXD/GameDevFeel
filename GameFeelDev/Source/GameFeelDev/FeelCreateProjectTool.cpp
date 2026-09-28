// Development tool: creates a new project from one of the engine's templates, exactly as the editor's New Project
// dialog does (same code path, including the template's feature packs). Lives in the host project, not in the plugin.
// Usage (editor closed):
// UnrealEditor-Cmd.exe GameFeelDev.uproject -ExecCmds="Automation RunTests DiagFeel.CreateProjectFromTemplate"
//   -FeelNewProject=B:/Path/Name/Name.uproject -FeelTemplate=B:/UE_5.6/Templates/TP_FirstPerson/TP_FirstPerson.uproject
// Editor builds only.

#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS

#include "GameProjectUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelCreateProjectFromTemplate, "DiagFeel.CreateProjectFromTemplate", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelCreateProjectFromTemplate::RunTest(const FString& Parameters)
{
	FString ProjectFile;
	FString TemplateFile;
	if (!FParse::Value(FCommandLine::Get(), TEXT("FeelNewProject="), ProjectFile) || !FParse::Value(FCommandLine::Get(), TEXT("FeelTemplate="), TemplateFile))
	{
		AddInfo(TEXT("CREATEPROJECT skipped: pass -FeelNewProject=<path>.uproject and -FeelTemplate=<template>.uproject"));
		return true;
	}
	if (FPaths::FileExists(ProjectFile))
	{
		AddError(FString::Printf(TEXT("CREATEPROJECT %s already exists; not touching it"), *ProjectFile));
		return false;
	}

	FProjectInformation Info;
	Info.ProjectFilename = ProjectFile;
	Info.TemplateFile = TemplateFile;
	Info.TemplateCategory = FName(TEXT("Games"));
	Info.bShouldGenerateCode = true;
	Info.TargetedHardware = EHardwareClass::Desktop;
	Info.DefaultGraphicsPerformance = EGraphicsPreset::Maximum;

	FText FailReason;
	FText FailLog;
	TArray<FString> CreatedFiles;
	const bool bCreated = GameProjectUtils::CreateProject(Info, FailReason, FailLog, &CreatedFiles);
	AddInfo(FString::Printf(TEXT("CREATEPROJECT %s: %s, %d files"), *ProjectFile, bCreated ? TEXT("created") : TEXT("FAILED"), CreatedFiles.Num()));
	if (!bCreated)
	{
		AddError(FString::Printf(TEXT("CREATEPROJECT failed: %s %s"), *FailReason.ToString(), *FailLog.ToString()));
	}
	return bCreated;
}

#endif
