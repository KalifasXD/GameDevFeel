// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Framework/Application/SlateApplication.h"
#include "ImageUtils.h"
#include "Input/HittestGrid.h"
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "RenderingThread.h"
#include "Slate/WidgetRenderer.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Docking/SDockTab.h"
#include "TextureResource.h"
#include "Widgets/SViewport.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWindow.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

/**
 * Pictures for the manual at a higher pixel density than the screen. A window cannot be taller than the desktop, so
 * instead of enlarging windows the interface is painted off screen with the same layout and Density times the pixels.
 * Viewports are rendered by the engine, not by Slate, so they stay empty in those paintings: each window picture is
 * also saved as the screen shows it (<name>_screen.png), and the game view is taken as a HighResShot of the scene
 * (<name>_scene.png) plus the interface layer with transparency (<name>_ui.png). Tools/Manual/prepare_shots.py puts
 * the parts together.
 *
 * Vector icons are rasterized for the new size only after an engine frame, so each capture first paints once and
 * returns false; call it again on later frames until it returns true (the picture is then saved, or could not be).
 */
namespace FeelManualCapture
{
	/** True once the capture called Name has painted once and a few engine frames have passed since. */
	inline bool Primed(const FString& Name)
	{
		static TMap<FString, uint64> PrimedAt;
		const uint64* Frame = PrimedAt.Find(Name);
		if (!Frame)
		{
			PrimedAt.Add(Name, GFrameCounter);
			return false;
		}
		if (GFrameCounter < *Frame + 4)
		{
			return false;
		}
		PrimedAt.Remove(Name);
		return true;
	}

	inline FString ShotFile(const FString& Name)
	{
		return FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("Manual") / (Name + TEXT(".png"));
	}

	inline bool SavePixels(TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Name, bool bKeepAlpha, FAutomationTestBase* Test)
	{
		if (Pixels.Num() != Width * Height || Pixels.Num() == 0)
		{
			Test->AddWarning(FString::Printf(TEXT("MANUALSHOT %s: nothing was read"), *Name));
			return false;
		}
		if (!bKeepAlpha)
		{
			for (FColor& Pixel : Pixels)
			{
				Pixel.A = 255;
			}
		}
		const FString File = ShotFile(Name);
		FImageUtils::SaveImageByExtension(*File, FImageView(Pixels.GetData(), Width, Height, ERawImageFormat::BGRA8));
		Test->AddInfo(FString::Printf(TEXT("MANUALSHOT %s %s (%dx%d)"), *Name, *File, Width, Height));
		return true;
	}

	inline UWorld* PIEWorld()
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

	/** The window that holds the editor of Asset. */
	inline TSharedPtr<SWindow> AssetEditorWindow(UObject* Asset)
	{
		UAssetEditorSubsystem* AssetEditors = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		IAssetEditorInstance* Editor = Asset ? AssetEditors->FindEditorForAsset(Asset, false) : nullptr;
		const TSharedPtr<FTabManager> Tabs = Editor ? Editor->GetAssociatedTabManager() : nullptr;
		const TSharedPtr<SDockTab> Owner = Tabs.IsValid() ? Tabs->GetOwnerTab() : nullptr;
		return Owner.IsValid() ? Owner->GetParentWindow() : nullptr;
	}

	/** The first widget of the given Slate type under Root, depth first. */
	inline TSharedPtr<SWidget> FindWidget(const TSharedRef<SWidget>& Root, const FName Type)
	{
		if (Root->GetType() == Type)
		{
			return Root;
		}
		FChildren* Children = Root->GetChildren();
		for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
		{
			if (TSharedPtr<SWidget> Found = FindWidget(Children->GetChildAt(Index), Type))
			{
				return Found;
			}
		}
		return nullptr;
	}

	/** The first widget under Root whose Slate type starts with Prefix (for template types such as STreeView<...>). */
	inline TSharedPtr<SWidget> FindWidgetStartingWith(const TSharedRef<SWidget>& Root, const FString& Prefix)
	{
		if (Root->GetType().ToString().StartsWith(Prefix))
		{
			return Root;
		}
		FChildren* Children = Root->GetChildren();
		for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
		{
			if (TSharedPtr<SWidget> Found = FindWidgetStartingWith(Children->GetChildAt(Index), Prefix))
			{
				return Found;
			}
		}
		return nullptr;
	}

	/** The first text block under Root whose text equals Text, or starts with it when bPrefix is set. */
	inline TSharedPtr<SWidget> FindText(const TSharedRef<SWidget>& Root, const FString& Text, bool bPrefix = false)
	{
		if (Root->GetType() == FName(TEXT("STextBlock")))
		{
			const FString Shown = StaticCastSharedRef<STextBlock>(Root)->GetText().ToString();
			if (bPrefix ? Shown.StartsWith(Text) : Shown == Text)
			{
				return Root;
			}
		}
		FChildren* Children = Root->GetChildren();
		for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
		{
			if (TSharedPtr<SWidget> Found = FindText(Children->GetChildAt(Index), Text, bPrefix))
			{
				return Found;
			}
		}
		return nullptr;
	}

	/** The nearest ancestor of Widget of the given Slate type. */
	inline TSharedPtr<SWidget> FindAncestor(const TSharedPtr<SWidget>& Widget, const FName Type)
	{
		for (TSharedPtr<SWidget> Parent = Widget.IsValid() ? Widget->GetParentWidget() : nullptr; Parent.IsValid(); Parent = Parent->GetParentWidget())
		{
			if (Parent->GetType() == Type)
			{
				return Parent;
			}
		}
		return nullptr;
	}

	/** Clicks a widget with the left mouse button, as the player would. The widget must be visible on screen. */
	inline void Click(const TSharedPtr<SWidget>& Widget)
	{
		if (!Widget.IsValid())
		{
			return;
		}
		const FGeometry& Geometry = Widget->GetTickSpaceGeometry();
		const FVector2D Center = FVector2D(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f));
		FSlateApplication& Slate = FSlateApplication::Get();
		const FPointerEvent Down(0, Center, Center, TSet<FKey>({ EKeys::LeftMouseButton }), EKeys::LeftMouseButton, 0.0f, FModifierKeysState());
		const FPointerEvent Up(0, Center, Center, TSet<FKey>(), EKeys::LeftMouseButton, 0.0f, FModifierKeysState());
		Slate.ProcessMouseButtonDownEvent(nullptr, Down);
		Slate.ProcessMouseButtonUpEvent(Up);
	}

	/** Draws Paint into a new render target of DrawSize pixels and saves it. */
	template <typename PaintFunc>
	bool ReadOffscreen(const FVector2D& DrawSize, bool bRead, TArray<FColor>& OutPixels, FIntPoint& OutSize, PaintFunc&& Paint)
	{
		UTextureRenderTarget2D* Target = FWidgetRenderer::CreateTargetFor(DrawSize, TF_Bilinear, true);
		if (!Target)
		{
			return false;
		}
		Target->AddToRoot();
		FWidgetRenderer* Renderer = new FWidgetRenderer(true, true);
		Paint(*Renderer, Target);
		FlushRenderingCommands();
		FTextureRenderTargetResource* Resource = Target->GameThread_GetRenderTargetResource();
		const bool bDone = bRead && Resource && Resource->ReadPixels(OutPixels);
		if (bDone)
		{
			// The gamma-correct render target comes back with the sRGB curve applied a second time (a screen gray of 38
			// reads 108); undo one application so the picture matches the screen. Alpha is left as it is.
			static uint8 Decode[256];
			static bool bDecodeReady = false;
			if (!bDecodeReady)
			{
				for (int32 Value = 0; Value < 256; ++Value)
				{
					Decode[Value] = static_cast<uint8>(FMath::RoundToInt(FMath::Clamp(FLinearColor::FromSRGBColor(FColor(Value, 0, 0)).R, 0.0f, 1.0f) * 255.0f));
				}
				bDecodeReady = true;
			}
			for (FColor& Pixel : OutPixels)
			{
				Pixel.R = Decode[Pixel.R];
				Pixel.G = Decode[Pixel.G];
				Pixel.B = Decode[Pixel.B];
			}
		}
		OutSize = FIntPoint(Target->SizeX, Target->SizeY);
		BeginCleanup(Renderer);
		Target->RemoveFromRoot();
		return bDone;
	}

	template <typename PaintFunc>
	bool SaveOffscreen(const FVector2D& DrawSize, const FString& Name, bool bKeepAlpha, bool bSave, FAutomationTestBase* Test, PaintFunc&& Paint)
	{
		TArray<FColor> Pixels;
		FIntPoint Size;
		return ReadOffscreen(DrawSize, bSave, Pixels, Size, Paint) && SavePixels(Pixels, Size.X, Size.Y, Name, bKeepAlpha, Test);
	}

	/** Paints a window at Density times its pixels. */
	inline bool ReadWindow(const TSharedRef<SWindow>& Window, float Density, bool bRead, TArray<FColor>& OutPixels, FIntPoint& OutSize)
	{
		const float Scale = Density * FSlateApplication::Get().GetApplicationScale() * Window->GetDPIScaleFactor();
		const FVector2D DrawSize = FVector2D(Window->GetSizeInScreen()) * Density;
		return ReadOffscreen(DrawSize, bRead, OutPixels, OutSize, [&Window, Scale, DrawSize](FWidgetRenderer& Renderer, UTextureRenderTarget2D* Target)
		{
			FHittestGrid Grid;
			Renderer.DrawWindow(Target, Grid, Window, Scale, DrawSize, 0.0f);
		});
	}

	/** The window as the screen shows it. */
	inline bool SaveScreen(const TSharedPtr<SWindow>& Window, const FString& Name, FAutomationTestBase* Test)
	{
		TArray<FColor> Pixels;
		FIntVector Size(0, 0, 0);
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size))
		{
			Test->AddWarning(FString::Printf(TEXT("MANUALSHOT %s: window could not be read"), *Name));
			return false;
		}
		return SavePixels(Pixels, Size.X, Size.Y, Name, false, Test);
	}

	/**
	 * <Name>.png: the whole window painted at Density times its pixels, with its open menus (child windows) painted on
	 * top where the screen shows them; <Name>_screen.png: the window as shown.
	 */
	inline bool SaveWindow(const TSharedPtr<SWindow>& Window, const FString& Name, float Density, FAutomationTestBase* Test)
	{
		if (!Window.IsValid())
		{
			Test->AddWarning(FString::Printf(TEXT("MANUALSHOT %s: no window"), *Name));
			return true;
		}
		const bool bSave = Primed(Name);
		if (bSave)
		{
			SaveScreen(Window, Name + TEXT("_screen"), Test);
		}
		TArray<FColor> Pixels;
		FIntPoint Size;
		const bool bRead = ReadWindow(Window.ToSharedRef(), Density, bSave, Pixels, Size);
		for (const TSharedRef<SWindow>& Child : Window->GetChildWindows())
		{
			// Menus: painting a menu window off screen leaves it empty, so its content is painted on its own and laid over
			// the menu as the screen shows it (which supplies the background).
			if (!Child->IsVisible() || Child->GetType() != EWindowType::Menu)
			{
				continue;
			}
			const TSharedRef<SWidget> Content = Child->GetContent();
			const float ChildScale = Density * FSlateApplication::Get().GetApplicationScale() * Child->GetDPIScaleFactor();
			const FVector2D ChildDraw = FVector2D(Child->GetSizeInScreen()) * Density;
			TArray<FColor> ChildPixels;
			FIntPoint ChildSize;
			const bool bChild = ReadOffscreen(ChildDraw, bSave, ChildPixels, ChildSize, [&Content, ChildScale, ChildDraw](FWidgetRenderer& Renderer, UTextureRenderTarget2D* Target)
			{
				Renderer.DrawWidget(Target, Content, ChildScale, ChildDraw, 0.0f);
			});
			TArray<FColor> Back;
			FIntVector BackSize(0, 0, 0);
			if (!bChild || !bRead || !FSlateApplication::Get().TakeScreenshot(Child, Back, BackSize) || BackSize.X <= 0 || BackSize.Y <= 0)
			{
				continue;
			}
			const FVector2D Offset = (FVector2D(Child->GetPositionInScreen()) - FVector2D(Window->GetPositionInScreen())) * Density;
			const int32 OX = FMath::RoundToInt(Offset.X);
			const int32 OY = FMath::RoundToInt(Offset.Y);
			for (int32 Y = 0; Y < ChildSize.Y; ++Y)
			{
				for (int32 X = 0; X < ChildSize.X; ++X)
				{
					const int32 TX = OX + X;
					const int32 TY = OY + Y;
					if (TX < 0 || TY < 0 || TX >= Size.X || TY >= Size.Y)
					{
						continue;
					}
					// Bilinear sample of the on-screen menu.
					const float SX = FMath::Clamp((X + 0.5f) / Density - 0.5f, 0.0f, float(BackSize.X - 1));
					const float SY = FMath::Clamp((Y + 0.5f) / Density - 0.5f, 0.0f, float(BackSize.Y - 1));
					const int32 X0 = int32(SX);
					const int32 Y0 = int32(SY);
					const int32 X1 = FMath::Min(X0 + 1, BackSize.X - 1);
					const int32 Y1 = FMath::Min(Y0 + 1, BackSize.Y - 1);
					const float FX = SX - X0;
					const float FY = SY - Y0;
					auto At = [&Back, &BackSize](int32 PX, int32 PY) { return FLinearColor(Back[PY * BackSize.X + PX].ReinterpretAsLinear()); };
					const FLinearColor Under = FMath::Lerp(FMath::Lerp(At(X0, Y0), At(X1, Y0), FX), FMath::Lerp(At(X0, Y1), At(X1, Y1), FX), FY);
					const FColor& Over = ChildPixels[Y * ChildSize.X + X];
					const float A = Over.A / 255.0f;
					FColor& Dest = Pixels[TY * Size.X + TX];
					Dest.R = uint8(FMath::Clamp(Over.R * A + Under.R * 255.0f * (1.0f - A), 0.0f, 255.0f));
					Dest.G = uint8(FMath::Clamp(Over.G * A + Under.G * 255.0f * (1.0f - A), 0.0f, 255.0f));
					Dest.B = uint8(FMath::Clamp(Over.B * A + Under.B * 255.0f * (1.0f - A), 0.0f, 255.0f));
				}
			}
		}
		if (bSave && bRead)
		{
			SavePixels(Pixels, Size.X, Size.Y, Name, false, Test);
		}
		return bSave;
	}

	/** <Name>_scene.png: the game's scene without its interface at Density times its pixels, on the next rendered frame. */
	inline void SaveGameScene(UGameViewportClient* Client, const FString& Name, float Density, FAutomationTestBase* Test)
	{
		if (Client && Client->GetWorld())
		{
			const FString SceneFile = ShotFile(Name + TEXT("_scene"));
			Client->Exec(Client->GetWorld(), *FString::Printf(TEXT("HighResShot %d filename=\"%s\""), FMath::RoundToInt(Density), *SceneFile), *GLog);
			Test->AddInfo(FString::Printf(TEXT("MANUALSHOT %s_scene %s (requested)"), *Name, *SceneFile));
		}
	}

	/**
	 * <Name>.png: the game view as it is on the screen, read from the desktop. The only way to include what the engine
	 * draws last, such as the showdebug text, which no engine screenshot contains. The view must be visible on screen.
	 */
	inline bool SaveGameViewFromDesktop(UGameViewportClient* Client, const FString& Name, FAutomationTestBase* Test)
	{
#if PLATFORM_WINDOWS
		const TSharedPtr<SViewport> ViewportWidget = Client ? Client->GetGameViewportWidget() : nullptr;
		if (!ViewportWidget.IsValid())
		{
			return false;
		}
		const FGeometry& Geometry = ViewportWidget->GetTickSpaceGeometry();
		const FVector2D Position = FVector2D(Geometry.GetAbsolutePosition());
		const FVector2D PixelSize = FVector2D(Geometry.GetAbsoluteSize());
		const int32 X = FMath::RoundToInt(Position.X);
		const int32 Y = FMath::RoundToInt(Position.Y);
		const int32 W = FMath::RoundToInt(PixelSize.X);
		const int32 H = FMath::RoundToInt(PixelSize.Y);
		HDC Screen = ::GetDC(nullptr);
		HDC Memory = ::CreateCompatibleDC(Screen);
		HBITMAP Bitmap = ::CreateCompatibleBitmap(Screen, W, H);
		HGDIOBJ Previous = ::SelectObject(Memory, Bitmap);
		::BitBlt(Memory, 0, 0, W, H, Screen, X, Y, SRCCOPY);
		BITMAPINFO Info = {};
		Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		Info.bmiHeader.biWidth = W;
		Info.bmiHeader.biHeight = -H;
		Info.bmiHeader.biPlanes = 1;
		Info.bmiHeader.biBitCount = 32;
		Info.bmiHeader.biCompression = BI_RGB;
		TArray<FColor> Pixels;
		Pixels.SetNumUninitialized(W * H);
		::GetDIBits(Memory, Bitmap, 0, H, Pixels.GetData(), &Info, DIB_RGB_COLORS);
		::SelectObject(Memory, Previous);
		::DeleteObject(Bitmap);
		::DeleteDC(Memory);
		::ReleaseDC(nullptr, Screen);
		return SavePixels(Pixels, W, H, Name, false, Test);
#else
		Test->AddWarning(FString::Printf(TEXT("MANUALSHOT %s: reading the desktop is only written for Windows"), *Name));
		return false;
#endif
	}

	/**
	 * The game view at Density times its pixels: <Name>_scene.png from HighResShot (written on the next rendered frame)
	 * and <Name>_ui.png, the game's interface layer with transparency.
	 */
	inline bool SaveGameView(UGameViewportClient* Client, const FString& Name, float Density, FAutomationTestBase* Test)
	{
		const TSharedPtr<SViewport> ViewportWidget = Client ? Client->GetGameViewportWidget() : nullptr;
		const TSharedPtr<SWidget> Content = ViewportWidget.IsValid() ? ViewportWidget->GetContent() : nullptr;
		if (!Content.IsValid() || !Client->GetWorld())
		{
			Test->AddWarning(FString::Printf(TEXT("MANUALSHOT %s: no game view"), *Name));
			return true;
		}
		const bool bSave = Primed(Name);
		if (bSave)
		{
			SaveGameScene(Client, Name, Density, Test);
		}

		const FGeometry& Geometry = ViewportWidget->GetTickSpaceGeometry();
		const float Scale = Geometry.GetAccumulatedLayoutTransform().GetScale() * Density;
		const FVector2D DrawSize = FVector2D(Geometry.GetLocalSize()) * Scale;
		const TSharedRef<SWidget> Layer = Content.ToSharedRef();
		SaveOffscreen(DrawSize, Name + TEXT("_ui"), true, bSave, Test, [&Layer, Scale, DrawSize](FWidgetRenderer& Renderer, UTextureRenderTarget2D* Target)
		{
			Renderer.DrawWidget(Target, Layer, Scale, DrawSize, 0.0f);
		});
		return bSave;
	}
}

#endif
