// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "CanvasTypes.h"
#include "RenderingThread.h"
#include "TextureResource.h"
#include "UnrealClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "FeelRecipe.h"
#include "FeelRecipeJson.h"
#include "FeelRecipeThumbnailRenderer.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

/**
 * Diagnostic (filter DiagFeel): draws recipe thumbnails into one image sheet, saved to Saved/FeelKit/Thumbnails.png, so
 * the drawing can be looked at without opening the editor. Needs a rendering session.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelThumbnailRenderDiagnostic, "DiagFeel.ThumbnailSheet", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
bool FFeelThumbnailRenderDiagnostic::RunTest(const FString& Parameters)
{
	if (!FApp::CanEverRender())
	{
		AddInfo(TEXT("Skipped: this session cannot render."));
		return true;
	}

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FeelKit"));
	const FString LibraryDir = Plugin.IsValid() ? FPaths::Combine(Plugin->GetBaseDir(), TEXT("Library")) : FString();
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *LibraryDir, TEXT("*.json"), true, false);
	Files.Sort();
	if (Files.Num() == 0)
	{
		AddError(TEXT("No library recipes found."));
		return false;
	}

	// One tile per recipe, laid out in a grid, at the size the Content Browser uses for medium tiles.
	constexpr int32 TileSize = 128;
	constexpr int32 Columns = 6;
	const int32 Count = FMath::Min(Files.Num(), 18);
	const int32 Rows = FMath::DivideAndRoundUp(Count, Columns);

	TStrongObjectPtr<UTextureRenderTarget2D> RenderTarget(NewObject<UTextureRenderTarget2D>(GetTransientPackage()));
	RenderTarget->ClearColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
	RenderTarget->InitAutoFormat(TileSize * Columns, TileSize * Rows);
	RenderTarget->UpdateResourceImmediate(true);

	TStrongObjectPtr<UFeelRecipeThumbnailRenderer> Renderer(NewObject<UFeelRecipeThumbnailRenderer>(GetTransientPackage()));
	FRenderTarget* Resource = RenderTarget->GameThread_GetRenderTargetResource();
	FCanvas Canvas(Resource, nullptr, FGameTime(), GMaxRHIFeatureLevel);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		FString Json;
		if (!FFileHelper::LoadFileToString(Json, *Files[Index]))
		{
			continue;
		}
		TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
		FText Error;
		TArray<FString> Missing;
		if (!FFeelRecipeJson::Import(*Recipe, Json, Error, &Missing))
		{
			continue;
		}

		const int32 X = (Index % Columns) * TileSize;
		const int32 Y = (Index / Columns) * TileSize;
		Renderer->Draw(Recipe.Get(), X, Y, TileSize, TileSize, Resource, &Canvas, false);
	}

	Canvas.Flush_GameThread();
	FlushRenderingCommands();

	TArray<FColor> Pixels;
	if (!Resource->ReadPixels(Pixels))
	{
		AddError(TEXT("Could not read the rendered pixels."));
		return false;
	}
	for (FColor& Pixel : Pixels)
	{
		Pixel.A = 255;
	}

	const FString File = FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("Thumbnails.png");
	FImageView Image(Pixels.GetData(), RenderTarget->SizeX, RenderTarget->SizeY, ERawImageFormat::BGRA8);
	if (!FImageUtils::SaveImageByExtension(*File, Image))
	{
		AddError(TEXT("Could not save the image."));
		return false;
	}
	AddInfo(FString::Printf(TEXT("THUMBSHEET wrote %s (%d tiles)"), *File, Count));
	return true;
}

#endif
