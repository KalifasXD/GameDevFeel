// Copyright 2026 Billo. All Rights Reserved.

#include "SFeelPreviewViewport.h"

#include "AdvancedPreviewScene.h"
#include "Components/AudioComponent.h"
#include "UObject/UObjectIterator.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EditorViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "FeelActorDelivery.h"
#include "FeelAudioDelivery.h"
#include "FeelGifWriter.h"
#include "FeelRecipe.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "FeelRecipeEditorState.h"
#include "Materials/MaterialInterface.h"
#include "SceneView.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

/** Preview sink: applies the evaluator output exactly like the runtime would, to the editor preview scene. */
class FFeelPreviewViewportClient : public FEditorViewportClient
{
public:
	FFeelPreviewViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SEditorViewport>& InViewportWidget, const TSharedRef<FFeelRecipeEditorState>& InState);
	virtual ~FFeelPreviewViewportClient() override;

	using FEditorViewportClient::Draw;

	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	virtual void SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) override;
	virtual void OverridePostProcessSettings(FSceneView& View) override;
	virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;

private:
	/** Shows the recipe's preview mesh (static or skeletal) or the default cube, standing on the floor and framed. */
	void RefreshPreviewMesh();

	UPrimitiveComponent* GetActivePreviewComponent() const;

	/**
	 * GIF capture: the recipe is played frame by frame twice, once with its output bypassed (Off) and once normally (On).
	 * Each tick reads the frame drawn for the previous tick's time, then moves to the next frame.
	 */
	void BeginGifCapture();
	void StepGifCapture();
	void FinishGifCapture();
	TArray<FColor> ReadDownscaledFrame(int32& OutWidth, int32& OutHeight) const;

	static constexpr int32 GifFramesPerSecond = 20;
	static constexpr float GifMaxSeconds = 6.0f;
	static constexpr int32 GifHalfWidth = 480;

	struct FGifCapture
	{
		bool bActive = false;
		bool bOnPass = false;
		bool bAwaitingFrame = false;
		int32 FrameIndex = 0;
		int32 NumFrames = 0;
		int32 FrameWidth = 0;
		int32 FrameHeight = 0;
		TArray<TArray<FColor>> OffFrames;
		TArray<TArray<FColor>> OnFrames;
	};
	FGifCapture GifCapture;
	FDelegateHandle GifRequestHandle;

	TWeakPtr<FFeelRecipeEditorState> State;
	TObjectPtr<UStaticMeshComponent> PreviewMesh = nullptr;
	TObjectPtr<USkeletalMeshComponent> PreviewSkeletalMesh = nullptr;
	FSoftObjectPath ShownPreviewMeshPath;
	bool bShowingSkeletalMesh = false;
	FFeelActorDelivery ActorDelivery;
	FFeelAudioDelivery AudioDelivery;
	FFeelPostProcessMaterialInstances PostProcessMaterialInstances;
};

FFeelPreviewViewportClient::FFeelPreviewViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SEditorViewport>& InViewportWidget, const TSharedRef<FFeelRecipeEditorState>& InState)
	: FEditorViewportClient(nullptr, &InPreviewScene, InViewportWidget)
	, State(InState)
{
	SetRealtime(true);
	EngineShowFlags.SetGrid(false);
	EngineShowFlags.SetVignette(true);
	ViewFOV = 90.0f;

	PreviewMesh = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	InPreviewScene.AddComponent(PreviewMesh, FTransform::Identity);

	PreviewSkeletalMesh = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	PreviewSkeletalMesh->SetVisibility(false);
	InPreviewScene.AddComponent(PreviewSkeletalMesh, FTransform::Identity);

	RefreshPreviewMesh();

	GifRequestHandle = InState->OnCaptureGifRequested.AddRaw(this, &FFeelPreviewViewportClient::BeginGifCapture);
}

FFeelPreviewViewportClient::~FFeelPreviewViewportClient()
{
	if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin())
	{
		PinnedState->OnCaptureGifRequested.Remove(GifRequestHandle);
		if (GifCapture.bActive)
		{
			PinnedState->SetOutputBypassed(false);
		}
	}
}

void FFeelPreviewViewportClient::BeginGifCapture()
{
	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	if (!PinnedState.IsValid() || !PinnedState->GetRecipe() || GifCapture.bActive || !Viewport)
	{
		return;
	}

	PinnedState->Stop();
	GifCapture = FGifCapture();
	GifCapture.bActive = true;
	const float Length = FMath::Clamp(PinnedState->GetPlaybackLength() + 0.25f, 0.5f, GifMaxSeconds);
	GifCapture.NumFrames = FMath::CeilToInt32(Length * GifFramesPerSecond);
	PinnedState->SetOutputBypassed(true);
	PinnedState->SetTime(0.0f);
	GifCapture.bAwaitingFrame = true;
}

TArray<FColor> FFeelPreviewViewportClient::ReadDownscaledFrame(int32& OutWidth, int32& OutHeight) const
{
	TArray<FColor> Source;
	const FIntPoint SourceSize = Viewport ? Viewport->GetSizeXY() : FIntPoint::ZeroValue;
	if (SourceSize.X <= 0 || SourceSize.Y <= 0 || !Viewport->ReadPixels(Source) || Source.Num() != SourceSize.X * SourceSize.Y)
	{
		return TArray<FColor>();
	}

	// Box filter down to the capture width.
	OutWidth = FMath::Min(GifHalfWidth, SourceSize.X);
	OutHeight = FMath::Max(1, FMath::RoundToInt32(static_cast<float>(SourceSize.Y) * OutWidth / SourceSize.X));
	TArray<FColor> Result;
	Result.SetNumUninitialized(OutWidth * OutHeight);
	for (int32 Y = 0; Y < OutHeight; ++Y)
	{
		const int32 SourceY0 = Y * SourceSize.Y / OutHeight;
		const int32 SourceY1 = FMath::Max(SourceY0 + 1, (Y + 1) * SourceSize.Y / OutHeight);
		for (int32 X = 0; X < OutWidth; ++X)
		{
			const int32 SourceX0 = X * SourceSize.X / OutWidth;
			const int32 SourceX1 = FMath::Max(SourceX0 + 1, (X + 1) * SourceSize.X / OutWidth);
			uint32 Red = 0;
			uint32 Green = 0;
			uint32 Blue = 0;
			uint32 Count = 0;
			for (int32 SourceY = SourceY0; SourceY < SourceY1; ++SourceY)
			{
				for (int32 SourceX = SourceX0; SourceX < SourceX1; ++SourceX)
				{
					const FColor& Pixel = Source[SourceY * SourceSize.X + SourceX];
					Red += Pixel.R;
					Green += Pixel.G;
					Blue += Pixel.B;
					++Count;
				}
			}
			Result[Y * OutWidth + X] = FColor(static_cast<uint8>(Red / Count), static_cast<uint8>(Green / Count), static_cast<uint8>(Blue / Count));
		}
	}
	return Result;
}

void FFeelPreviewViewportClient::StepGifCapture()
{
	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	if (!PinnedState.IsValid())
	{
		GifCapture = FGifCapture();
		return;
	}

	if (GifCapture.bAwaitingFrame)
	{
		int32 Width = 0;
		int32 Height = 0;
		TArray<FColor> Frame = ReadDownscaledFrame(Width, Height);
		TArray<TArray<FColor>>& PassFrames = GifCapture.bOnPass ? GifCapture.OnFrames : GifCapture.OffFrames;
		if (Frame.Num() == 0 || (GifCapture.FrameWidth != 0 && (Width != GifCapture.FrameWidth || Height != GifCapture.FrameHeight)))
		{
			// The viewport was hidden or resized: start the capture again.
			const bool bWasOnPass = GifCapture.bOnPass;
			GifCapture.bOnPass = false;
			GifCapture.FrameIndex = 0;
			GifCapture.FrameWidth = 0;
			GifCapture.FrameHeight = 0;
			GifCapture.OffFrames.Reset();
			GifCapture.OnFrames.Reset();
			if (bWasOnPass)
			{
				PinnedState->SetOutputBypassed(true);
			}
		}
		else
		{
			GifCapture.FrameWidth = Width;
			GifCapture.FrameHeight = Height;
			PassFrames.Add(MoveTemp(Frame));
			++GifCapture.FrameIndex;
		}
	}

	if (GifCapture.FrameIndex >= GifCapture.NumFrames)
	{
		if (!GifCapture.bOnPass)
		{
			GifCapture.bOnPass = true;
			GifCapture.FrameIndex = 0;
			PinnedState->Stop();
			PinnedState->SetOutputBypassed(false);
		}
		else
		{
			FinishGifCapture();
			return;
		}
	}

	PinnedState->SetTime(static_cast<float>(GifCapture.FrameIndex) / GifFramesPerSecond);
	GifCapture.bAwaitingFrame = true;
}

void FFeelPreviewViewportClient::FinishGifCapture()
{
	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	FGifCapture Finished = MoveTemp(GifCapture);
	GifCapture = FGifCapture();
	if (!PinnedState.IsValid() || !PinnedState->GetRecipe())
	{
		return;
	}
	PinnedState->SetOutputBypassed(false);
	PinnedState->Stop();

	// Without the recipe on the left, with it on the right, separated by a thin white divider.
	constexpr int32 Divider = 4;
	const int32 Width = Finished.FrameWidth * 2 + Divider;
	const int32 Height = Finished.FrameHeight;
	const int32 NumFrames = FMath::Min(Finished.OffFrames.Num(), Finished.OnFrames.Num());
	TArray<TArray<FColor>> Combined;
	Combined.Reserve(NumFrames);
	for (int32 FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex)
	{
		TArray<FColor>& Frame = Combined.AddDefaulted_GetRef();
		Frame.Init(FColor::White, Width * Height);
		for (int32 Y = 0; Y < Height; ++Y)
		{
			FMemory::Memcpy(&Frame[Y * Width], &Finished.OffFrames[FrameIndex][Y * Finished.FrameWidth], Finished.FrameWidth * sizeof(FColor));
			FMemory::Memcpy(&Frame[Y * Width + Finished.FrameWidth + Divider], &Finished.OnFrames[FrameIndex][Y * Finished.FrameWidth], Finished.FrameWidth * sizeof(FColor));
		}
	}

	const FString Directory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("FeelKit") / TEXT("Captures"));
	const FString FileName = FString::Printf(TEXT("%s_%s.gif"), *PinnedState->GetRecipe()->GetName(), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
	const FString FilePath = Directory / FileName;

	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Directory);
	const bool bSaved = NumFrames > 0 && FFileHelper::SaveArrayToFile(FFeelGifWriter::Encode(Width, Height, Combined, 100 / GifFramesPerSecond), *FilePath);

	FNotificationInfo Info(bSaved
		? FText::Format(LOCTEXT("GifSaved", "Saved {0} (left: without the recipe, right: with it)"), FText::FromString(FileName))
		: LOCTEXT("GifFailed", "GIF capture failed. Keep the preview viewport visible while capturing."));
	Info.ExpireDuration = 8.0f;
	if (bSaved)
	{
		Info.Hyperlink = FSimpleDelegate::CreateLambda([FilePath]()
		{
			FPlatformProcess::ExploreFolder(*FilePath);
		});
		Info.HyperlinkText = LOCTEXT("GifShowInFolder", "Show in Explorer");
	}
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FFeelPreviewViewportClient::RefreshPreviewMesh()
{
	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	const UFeelRecipe* Recipe = PinnedState.IsValid() ? PinnedState->GetRecipe() : nullptr;
	ShownPreviewMeshPath = Recipe ? Recipe->PreviewMesh.ToSoftObjectPath() : FSoftObjectPath();

	// Effects on the previous mesh go back to its original values before switching.
	ActorDelivery.RestoreAll();

	UObject* Asset = ShownPreviewMeshPath.IsValid() ? ShownPreviewMeshPath.TryLoad() : nullptr;
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(Asset);
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset);

	FBoxSphereBounds Bounds(FVector::ZeroVector, FVector(50.0), 86.6);
	bShowingSkeletalMesh = SkeletalMesh != nullptr;
	if (bShowingSkeletalMesh)
	{
		PreviewSkeletalMesh->SetSkeletalMeshAsset(SkeletalMesh);
		Bounds = SkeletalMesh->GetBounds();
	}
	else
	{
		if (StaticMesh)
		{
			PreviewMesh->SetStaticMesh(StaticMesh);
			PreviewMesh->SetMaterial(0, nullptr);
		}
		else
		{
			// The cube's own material has no parameters; this one exposes "Color", so material pulses are visible out of the box.
			PreviewMesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			PreviewMesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")));
		}
		if (const UStaticMesh* ShownMesh = PreviewMesh->GetStaticMesh())
		{
			Bounds = ShownMesh->GetBounds();
		}
	}
	PreviewMesh->SetVisibility(!bShowingSkeletalMesh);
	PreviewSkeletalMesh->SetVisibility(bShowingSkeletalMesh);

	// Stand the mesh on the floor at the origin and frame it.
	UPrimitiveComponent* Active = GetActivePreviewComponent();
	const FVector StandLocation(0.0, 0.0, Bounds.BoxExtent.Z - Bounds.Origin.Z);
	Active->SetWorldLocation(StandLocation);

	const FVector Center = StandLocation + Bounds.Origin;
	const double Radius = FMath::Max(Bounds.SphereRadius, 50.0);
	const FVector CameraLocation = Center + FVector(-Radius * 4.0, 0.0, Radius * 0.6);
	SetViewLocation(CameraLocation);
	SetViewRotation((Center - CameraLocation).Rotation());

	if (PinnedState.IsValid())
	{
		PinnedState->SetPreviewScene(PreviewScene->GetWorld(), Active);
	}
}

UPrimitiveComponent* FFeelPreviewViewportClient::GetActivePreviewComponent() const
{
	return bShowingSkeletalMesh ? static_cast<UPrimitiveComponent*>(PreviewSkeletalMesh) : static_cast<UPrimitiveComponent*>(PreviewMesh);
}

void FFeelPreviewViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	if (GifCapture.bActive)
	{
		StepGifCapture();
	}

	if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin())
	{
		const UFeelRecipe* Recipe = PinnedState->GetRecipe();
		const FSoftObjectPath WantedPath = Recipe ? Recipe->PreviewMesh.ToSoftObjectPath() : FSoftObjectPath();
		if (WantedPath != ShownPreviewMeshPath)
		{
			RefreshPreviewMesh();
		}

		// Same actor delivery as the runtime: scale and material parameters, restored when they stop.
		const FFeelFrameOutput& Output = PinnedState->GetOutput();
		UPrimitiveComponent* Active = GetActivePreviewComponent();
		ActorDelivery.BeginFrame();
		ActorDelivery.AddScale(Active, Output.TargetScaleDelta);
		ActorDelivery.AddTransform(Active, Output.TargetLocationOffset, Output.TargetRotationOffset);
		ActorDelivery.AddLight(Active, Output.LightIntensityDelta, Output.LightColor, Output.LightColorWeight);
		ActorDelivery.AddMaterialParameters(Active, Output.MaterialParameters);
		ActorDelivery.AddOverlayFlash(Cast<UMeshComponent>(Active), Output.OverlayFlashMaterial, Output.OverlayFlashColor, Output.OverlayFlashAmount);
		ActorDelivery.EndFrame();

		// Same audio delivery as the runtime: sound class volume, pitch and low-pass.
		AudioDelivery.Apply(PreviewScene->GetWorld(), Output.SoundClassAdjusts);
	}

	PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

void FFeelPreviewViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	if (!PinnedState.IsValid())
	{
		FEditorViewportClient::Draw(InViewport, Canvas);
		return;
	}

	// Apply camera contributions on top of the user's camera for this frame only, then restore it.
	FViewportCameraTransform& CameraTransform = GetViewTransform();
	const FVector BaseLocation = CameraTransform.GetLocation();
	const FRotator BaseRotation = CameraTransform.GetRotation();
	const float BaseFieldOfView = ViewFOV;

	FVector Location = BaseLocation;
	FRotator Rotation = BaseRotation;
	float FieldOfView = BaseFieldOfView;
	PinnedState->GetOutput().ApplyToView(Location, Rotation, FieldOfView);

	CameraTransform.SetLocation(Location);
	CameraTransform.SetRotation(Rotation);
	ViewFOV = FieldOfView;

	FEditorViewportClient::Draw(InViewport, Canvas);

	CameraTransform.SetLocation(BaseLocation);
	CameraTransform.SetRotation(BaseRotation);
	ViewFOV = BaseFieldOfView;
}

void FFeelPreviewViewportClient::SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View)
{
	FEditorViewportClient::SetupViewForRendering(ViewFamily, View);

	if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin())
	{
		PinnedState->GetOutput().ApplyOverlay(View.OverlayColor);
	}
}

void FFeelPreviewViewportClient::OverridePostProcessSettings(FSceneView& View)
{
	FEditorViewportClient::OverridePostProcessSettings(View);

	if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin())
	{
		PinnedState->GetOutput().ForEachPostProcessBlend([&View](FPostProcessSettings& Settings, float Weight)
		{
			View.OverridePostProcessSettings(Settings, Weight);
		}, &PostProcessMaterialInstances);
	}
}

void FFeelPreviewViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);

	if (const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin())
	{
		const FText TimeText = FText::AsNumber(PinnedState->GetTime(), &FNumberFormattingOptions::DefaultNoGrouping());
		const FText Label = GifCapture.bActive
			? FText::Format(GifCapture.bOnPass ? LOCTEXT("PreviewHudOn", "With recipe  {0} s") : LOCTEXT("PreviewHudOff", "Without recipe  {0} s"), TimeText)
			: FText::Format(LOCTEXT("PreviewHud", "FeelKit Preview  {0} s"), TimeText);
		FCanvasTextItem Text(FVector2D(24.0, 20.0), Label, GEngine->GetSmallFont(), FLinearColor::White);
		Canvas.DrawItem(Text);
	}
}

void SFeelPreviewViewport::Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState)
{
	State = InState;

	// Audio plays in the preview.
	PreviewScene = MakeShared<FAdvancedPreviewScene>(FPreviewScene::ConstructionValues().AllowAudioPlayback(true));

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

SFeelPreviewViewport::~SFeelPreviewViewport()
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->Viewport = nullptr;
	}
}

TSharedRef<FEditorViewportClient> SFeelPreviewViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShared<FFeelPreviewViewportClient>(*PreviewScene, SharedThis(this), State.ToSharedRef());
	return ViewportClient.ToSharedRef();
}

UWorld* SFeelPreviewViewport::GetPreviewWorld() const
{
	return PreviewScene.IsValid() ? PreviewScene->GetWorld() : nullptr;
}

void SFeelPreviewViewport::SetAudioMuted(bool bMuted)
{
	UWorld* World = GetPreviewWorld();
	if (!World)
	{
		return;
	}

	// Play Sound spawns through UGameplayStatics, which starts nothing in a world that does not allow audio.
	World->bAllowAudioPlayback = !bMuted;
	if (bMuted)
	{
		for (TObjectIterator<UAudioComponent> It; It; ++It)
		{
			if (It->GetWorld() == World && It->IsPlaying())
			{
				It->Stop();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
