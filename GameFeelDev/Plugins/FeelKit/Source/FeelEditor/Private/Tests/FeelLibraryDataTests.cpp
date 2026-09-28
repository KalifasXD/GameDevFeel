// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelComfortTypes.h"
#include "FeelEvaluator.h"
#include "FeelFrameOutput.h"
#include "FeelRecipe.h"
#include "FeelRecipeJson.h"
#include "FeelTags.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/DataValidation.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/UObjectIterator.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelLibraryDataTests
{
	/** Parameter names every library recipe is allowed to use, so games can drive any of them the same way. */
	const TSet<FName> AllowedParameters =
	{
		FName(TEXT("Damage")), FName(TEXT("FallSpeed")), FName(TEXT("Health")),
		FName(TEXT("Fear")), FName(TEXT("Charge")), FName(TEXT("Combo")), FName(TEXT("Distance")),
	};

	/** Property of Struct whose name matches a JSON key (the importer matches names without regard to case). */
	const FProperty* FindPropertyForKey(const UStruct* Struct, const FString& Key)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if (It->GetName().Equals(Key, ESearchCase::IgnoreCase))
			{
				return *It;
			}
		}
		return nullptr;
	}

	/**
	 * Every key of every step object names a real property of that step. The JSON importer skips keys it does not know,
	 * so a misspelled property would otherwise import without an error and silently keep its default value.
	 */
	void FindUnknownStepKeys(const FString& Json, TArray<FString>& OutUnknown)
	{
		TSharedPtr<FJsonObject> Root;
		if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
		{
			OutUnknown.Add(TEXT("(not valid JSON)"));
			return;
		}

		TFunction<void(const TSharedPtr<FJsonObject>&)> Visit = [&OutUnknown, &Visit](const TSharedPtr<FJsonObject>& Object)
		{
			if (!Object.IsValid())
			{
				return;
			}
			FString ClassPath;
			if (Object->TryGetStringField(TEXT("_ClassName"), ClassPath))
			{
				const UClass* StepClass = FindObject<UClass>(nullptr, *ClassPath);
				if (!StepClass)
				{
					OutUnknown.Add(FString::Printf(TEXT("unknown step class %s"), *ClassPath));
				}
				else
				{
					for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
					{
						if (Pair.Key != TEXT("_ClassName") && !FindPropertyForKey(StepClass, Pair.Key))
						{
							OutUnknown.Add(FString::Printf(TEXT("%s has no property '%s'"), *StepClass->GetName(), *Pair.Key));
						}
					}
				}
			}
			for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Object->Values)
			{
				if (Pair.Value->Type == EJson::Object)
				{
					Visit(Pair.Value->AsObject());
				}
				else if (Pair.Value->Type == EJson::Array)
				{
					for (const TSharedPtr<FJsonValue>& Element : Pair.Value->AsArray())
					{
						if (Element->Type == EJson::Object)
						{
							Visit(Element->AsObject());
						}
					}
				}
			}
		};
		Visit(Root);
	}

	FString LibraryDir()
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FeelKit"));
		return Plugin.IsValid() ? FPaths::Combine(Plugin->GetBaseDir(), TEXT("Library")) : FString();
	}

	/** Every output of the recipe over its whole length, to prove it evaluates without producing invalid numbers. */
	bool EvaluatesCleanly(UFeelRecipe& Recipe, float ParameterAlpha, FString& OutProblem)
	{
		TMap<FName, float> ParameterValues;
		for (const FFeelRecipeParameter& Parameter : Recipe.Parameters)
		{
			if (!Parameter.Name.IsNone())
			{
				ParameterValues.Add(Parameter.Name, FMath::Lerp(Parameter.MinValue, Parameter.MaxValue, ParameterAlpha));
			}
		}

		FFeelEvalParams Params;
		Params.InstanceSeed = 12345;
		Params.ParameterValues = &ParameterValues;

		const float Length = FMath::Max(Recipe.GetDuration(), 0.05f);
		for (float Time = 0.0f; Time <= Length + UE_KINDA_SMALL_NUMBER; Time += Length / 24.0f)
		{
			FFeelOutputAccumulator Accumulator;
			FFeelEvaluator::Evaluate(Recipe, Time, Params, Accumulator);
			const FFeelFrameOutput& Output = Accumulator.Output;
			if (!FMath::IsFinite(Output.FlashAlpha) || !FMath::IsFinite(Output.GlobalTimeDilation.Dilation)
				|| Output.CameraLocationOffset.ContainsNaN() || !FMath::IsFinite(Output.FieldOfViewOffset)
				|| Output.TargetScaleDelta.ContainsNaN())
			{
				OutProblem = FString::Printf(TEXT("output is not a finite number at %.2f s"), Time);
				return false;
			}
		}
		return true;
	}
}

/**
 * Proves every recipe of the shipped library (the JSON files under the plugin's Library folder) imports, is valid, uses
 * only the shared parameter names and evaluates cleanly at the lowest, default and highest parameter values.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelLibraryDataTest, "FeelKit.Library.Data", FEEL_TEST_FLAGS)
bool FFeelLibraryDataTest::RunTest(const FString& Parameters)
{
	using namespace FeelLibraryDataTests;

	const FString Directory = LibraryDir();
	if (!TestFalse(TEXT("The FeelKit plugin is found"), Directory.IsEmpty()))
	{
		return false;
	}

	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Directory, TEXT("*.json"), true, false);
	Files.Sort();
	if (!TestEqual(TEXT("The library has 38 recipes"), Files.Num(), 38))
	{
		AddError(FString::Printf(TEXT("Looked in %s"), *Directory));
		return false;
	}

	int32 MissingSampleAssets = 0;
	for (const FString& File : Files)
	{
		const FString FileName = FPaths::GetBaseFilename(File);
		const FString Folder = FPaths::GetCleanFilename(FPaths::GetPath(File));

		FString Json;
		if (!TestTrue(*FString::Printf(TEXT("%s can be read"), *FileName), FFileHelper::LoadFileToString(Json, *File)))
		{
			continue;
		}

		TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
		FText Error;
		TArray<FString> Missing;
		if (!TestTrue(*FString::Printf(TEXT("%s imports"), *FileName), FFeelRecipeJson::Import(*Recipe, Json, Error, &Missing)))
		{
			AddError(FString::Printf(TEXT("%s: %s"), *FileName, *Error.ToString()));
			continue;
		}
		MissingSampleAssets += Missing.Num();

		TArray<FString> Unknown;
		FindUnknownStepKeys(Json, Unknown);
		for (const FString& Problem : Unknown)
		{
			AddError(FString::Printf(TEXT("%s: %s"), *FileName, *Problem));
		}

		// Name, folder and feeling must agree: FR_<Feeling>_<Name>.json in the <Feeling> folder.
		const FString ExpectedPrefix = FString::Printf(TEXT("FR_%s_"), *Folder);
		TestTrue(*FString::Printf(TEXT("%s is named after its folder"), *FileName), FileName.StartsWith(ExpectedPrefix));
#if WITH_EDITORONLY_DATA
		TestEqual(*FString::Printf(TEXT("%s has the matching feeling"), *FileName), Recipe->Feeling.GetTagName().ToString(), FString::Printf(TEXT("Feel.Feeling.%s"), *Folder));
		TestTrue(*FString::Printf(TEXT("%s has at least one genre"), *FileName), Recipe->Genres.Num() > 0);
		TestFalse(*FString::Printf(TEXT("%s has a description"), *FileName), Recipe->Description.IsEmpty());
#endif
		TestTrue(*FString::Printf(TEXT("%s has tracks"), *FileName), Recipe->Tracks.Num() > 0);

		for (const FFeelTrack& Track : Recipe->Tracks)
		{
			TestNotNull(*FString::Printf(TEXT("%s: every track has a step"), *FileName), ToRawPtr(Track.Step));
			TestTrue(*FString::Printf(TEXT("%s: every track has a channel"), *FileName), Track.Channel.IsValid());
			for (const FFeelParameterMapping& Mapping : Track.ParameterMappings)
			{
				TestNotNull(*FString::Printf(TEXT("%s: mapping %s is declared"), *FileName, *Mapping.Parameter.ToString()), Recipe->FindParameter(Mapping.Parameter));
			}
		}

		for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
		{
			TestTrue(*FString::Printf(TEXT("%s uses a shared parameter name (%s)"), *FileName, *Parameter.Name.ToString()), AllowedParameters.Contains(Parameter.Name));
			TestTrue(*FString::Printf(TEXT("%s: parameter %s has a range"), *FileName, *Parameter.Name.ToString()), Parameter.MaxValue > Parameter.MinValue);
			TestFalse(*FString::Printf(TEXT("%s: parameter %s is described"), *FileName, *Parameter.Name.ToString()), Parameter.Description.IsEmpty());
			TestTrue(*FString::Printf(TEXT("%s: accumulator of %s is Combo or none"), *FileName, *Parameter.Name.ToString()),
				Parameter.Accumulator.IsNone() || Parameter.Accumulator == FName(TEXT("Combo")));
		}

		// Validation, ignoring the sample assets that the user imports separately.
		FDataValidationContext Context;
		Recipe->IsDataValid(Context);
		TArray<FText> Warnings;
		TArray<FText> Errors;
		Context.SplitIssues(Warnings, Errors);
		for (const FText& ValidationError : Errors)
		{
			const FString Message = ValidationError.ToString();
			if (!Message.Contains(TEXT("asset")) && !Message.Contains(TEXT("Sound")) && !Message.Contains(TEXT("Material")))
			{
				AddError(FString::Printf(TEXT("%s is not valid: %s"), *FileName, *Message));
			}
		}

		FString Problem;
		for (const float Alpha : { 0.0f, 0.5f, 1.0f })
		{
			if (!EvaluatesCleanly(*Recipe, Alpha, Problem))
			{
				AddError(FString::Printf(TEXT("%s with parameters at %.0f%%: %s"), *FileName, Alpha * 100.0f, *Problem));
			}
		}
	}

	AddInfo(FString::Printf(TEXT("Sample asset references not yet in this project: %d (they are created from the content checklist)."), MissingSampleAssets));
	return true;
}

/**
 * Proves every recipe of the demo kits (the JSON files under the plugin's Demos folder) imports, names only real step
 * properties, declares every parameter it maps, is valid apart from sample assets, and evaluates cleanly.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelDemoDataTest, "FeelKit.Demos.Data", FEEL_TEST_FLAGS)
bool FFeelDemoDataTest::RunTest(const FString& Parameters)
{
	using namespace FeelLibraryDataTests;

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FeelKit"));
	const FString Directory = Plugin.IsValid() ? FPaths::Combine(Plugin->GetBaseDir(), TEXT("Demos")) : FString();
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Directory, TEXT("*.json"), true, false);
	Files.Sort();
	if (!TestTrue(TEXT("The demo kits have recipes"), Files.Num() > 0))
	{
		AddError(FString::Printf(TEXT("Looked in %s"), *Directory));
		return false;
	}

	for (const FString& File : Files)
	{
		const FString FileName = FPaths::GetBaseFilename(File);
		FString Json;
		if (!TestTrue(*FString::Printf(TEXT("%s can be read"), *FileName), FFileHelper::LoadFileToString(Json, *File)))
		{
			continue;
		}

		TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
		FText Error;
		TArray<FString> Missing;
		if (!TestTrue(*FString::Printf(TEXT("%s imports"), *FileName), FFeelRecipeJson::Import(*Recipe, Json, Error, &Missing)))
		{
			AddError(FString::Printf(TEXT("%s: %s"), *FileName, *Error.ToString()));
			continue;
		}

		TArray<FString> Unknown;
		FindUnknownStepKeys(Json, Unknown);
		for (const FString& Problem : Unknown)
		{
			AddError(FString::Printf(TEXT("%s: %s"), *FileName, *Problem));
		}

		TestTrue(*FString::Printf(TEXT("%s has tracks"), *FileName), Recipe->Tracks.Num() > 0);
		for (const FFeelTrack& Track : Recipe->Tracks)
		{
			TestNotNull(*FString::Printf(TEXT("%s: every track has a step"), *FileName), ToRawPtr(Track.Step));
			for (const FFeelParameterMapping& Mapping : Track.ParameterMappings)
			{
				TestNotNull(*FString::Printf(TEXT("%s: mapping %s is declared"), *FileName, *Mapping.Parameter.ToString()), Recipe->FindParameter(Mapping.Parameter));
			}
		}

		FString Problem;
		for (const float Alpha : { 0.0f, 0.5f, 1.0f })
		{
			if (!EvaluatesCleanly(*Recipe, Alpha, Problem))
			{
				AddError(FString::Printf(TEXT("%s with parameters at %.0f%%: %s"), *FileName, Alpha * 100.0f, *Problem));
			}
		}
	}
	return true;
}

#endif
