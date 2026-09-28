// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FeelRecipe.h"
#include "FeelRecipeJson.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/** Diagnostic (filter DiagFeel): writes a recipe of this project as JSON to Saved/FeelKit, to read the exact file format. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelRecipeExportDiagnostic, "DiagFeel.ExportRecipe", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelRecipeExportDiagnostic::RunTest(const FString& Parameters)
{
	for (const TCHAR* Path : { TEXT("/Game/FeelKitTests/R_Hit.R_Hit"), TEXT("/Game/FeelKitTests/R_Senses.R_Senses"), TEXT("/Game/FeelKitTests/R_Sustain.R_Sustain") })
	{
		UFeelRecipe* Recipe = LoadObject<UFeelRecipe>(nullptr, Path);
		if (!Recipe)
		{
			AddInfo(FString::Printf(TEXT("EXPORTDIAG missing %s"), Path));
			continue;
		}
		const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / (Recipe->GetName() + TEXT(".json"));
		FFileHelper::SaveStringToFile(FFeelRecipeJson::Export(*Recipe), *File, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		AddInfo(FString::Printf(TEXT("EXPORTDIAG wrote %s"), *File));
	}
	return true;
}

#endif
