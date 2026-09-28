// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelOutputSink.h"

#include "UObject/ObjectKey.h"
#include "UObject/StrongObjectPtr.h"

class UMaterialInstanceDynamic;
struct FPostProcessSettings;

/**
 * Material instances for post-process materials driven through a weight parameter. One per output (runtime camera,
 * editor preview), so each view keeps its own instances.
 */
class FEELCORE_API FFeelPostProcessMaterialInstances
{
public:
	FFeelPostProcessMaterialInstances();
	~FFeelPostProcessMaterialInstances();
	FFeelPostProcessMaterialInstances(const FFeelPostProcessMaterialInstances&) = delete;
	FFeelPostProcessMaterialInstances& operator=(const FFeelPostProcessMaterialInstances&) = delete;

	/** An instance of Material with Parameter set to Weight, created on first use. */
	UMaterialInstanceDynamic* Get(UMaterialInterface* Material, FName Parameter, float Weight);

	void Reset();

private:
	TMap<TPair<FObjectKey, FName>, TStrongObjectPtr<UMaterialInstanceDynamic>> Instances;
};

/** A time dilation request. Inactive until a step asks for one. */
struct FEELCORE_API FFeelTimeRequest
{
	bool bActive = false;
	float Dilation = 1.0f;
	int32 Priority = 0;

	/** Keeps the winning request: higher priority, or equal priority with the lower dilation. */
	void Merge(float OtherDilation, int32 OtherPriority);
};

/** One post-process parameter blended over the scene. */
struct FEELCORE_API FFeelPostProcessContribution
{
	float Value = 0.0f;
	float Weight = 0.0f;
};

/** Combined contributions of all active tracks for one moment. */
struct FEELCORE_API FFeelFrameOutput
{
	static constexpr int32 NumPostProcessParameters = static_cast<int32>(EFeelPostProcessParameter::Count);

	FVector CameraLocationOffset = FVector::ZeroVector;
	FRotator CameraRotationOffset = FRotator::ZeroRotator;
	float FieldOfViewOffset = 0.0f;
	FLinearColor FlashColor = FLinearColor::White;
	float FlashAlpha = 0.0f;
	FVector TargetScaleDelta = FVector::ZeroVector;
	FFeelTimeRequest GlobalTimeDilation;
	FFeelTimeRequest TargetTimeDilation;
	FFeelPostProcessContribution PostProcess[NumPostProcessParameters];
	TArray<FFeelMaterialParameter, TInlineAllocator<2>> MaterialParameters;
	FFeelForceFeedbackValues ForceFeedback;

	FLinearColor TintColor = FLinearColor::White;
	float TintWeight = 0.0f;
	FLinearColor FadeColor = FLinearColor::Black;
	float FadeAlpha = 0.0f;
	TArray<FFeelPostProcessMaterial, TInlineAllocator<1>> PostProcessMaterials;

	FVector TargetLocationOffset = FVector::ZeroVector;
	FRotator TargetRotationOffset = FRotator::ZeroRotator;

	float LightIntensityDelta = 0.0f;
	FLinearColor LightColor = FLinearColor::White;
	float LightColorWeight = 0.0f;

	TArray<FFeelSoundClassAdjust, TInlineAllocator<1>> SoundClassAdjusts;

	FVector2D WidgetTranslation = FVector2D::ZeroVector;
	FVector2D WidgetScaleDelta = FVector2D::ZeroVector;
	float WidgetAngle = 0.0f;
	FLinearColor WidgetColor = FLinearColor::White;
	float WidgetColorWeight = 0.0f;

	/** Overlay flash on the target's meshes (see IFeelOutputSink::AddOverlayFlash). */
	UMaterialInterface* OverlayFlashMaterial = nullptr;
	FLinearColor OverlayFlashColor = FLinearColor::White;
	float OverlayFlashAmount = 0.0f;

	void Reset();

	/** Keeps the stronger adjustment per sound class (see IFeelOutputSink::AddSoundClassAdjust). */
	static void MergeSoundClassAdjust(TArray<FFeelSoundClassAdjust, TInlineAllocator<1>>& Into, const FFeelSoundClassAdjust& Adjust);

	/** Keeps the stronger weight per material. */
	static void MergePostProcessMaterial(TArray<FFeelPostProcessMaterial, TInlineAllocator<1>>& Into, const FFeelPostProcessMaterial& Material);

	const FFeelPostProcessContribution& GetPostProcess(EFeelPostProcessParameter Parameter) const { return PostProcess[static_cast<int32>(Parameter)]; }

	/**
	 * Applies the camera contributions to a view. The runtime camera modifier and the editor preview both
	 * call this, so camera effects look identical in both.
	 */
	void ApplyToView(FVector& InOutLocation, FRotator& InOutRotation, float& InOutFieldOfView) const;

	/** Overlay color for FSceneView::OverlayColor; alpha is the flash strength. */
	FLinearColor GetOverlayColor() const;

	/**
	 * Layers the flash over an existing view overlay (for example a camera fade), as if the fade were drawn first
	 * and the flash on top. Shared by the runtime view extension and the editor preview.
	 */
	void ApplyOverlay(FLinearColor& InOutOverlayColor) const;

	/**
	 * Calls Blend once per active post-process contribution, with settings that override only that parameter.
	 * Shared by the runtime camera manager blends and the editor preview, so screen effects look identical in both.
	 */
	void ForEachPostProcessBlend(TFunctionRef<void(FPostProcessSettings& Settings, float Weight)> Blend, FFeelPostProcessMaterialInstances* MaterialInstances = nullptr) const;

	/** Returns BaseScale with the accumulated relative scale change applied. */
	FVector ApplyToScale(const FVector& BaseScale) const;

	/** True when both contributions drive the same parameter the same way (name, route, scalar or color). */
	static bool IsSameMaterialParameter(const FFeelMaterialParameter& A, const FFeelMaterialParameter& B);

	/** Combines a contribution into one for the same parameter: scalars add, colors keep the strongest weight. */
	static void MergeMaterialParameter(FFeelMaterialParameter& Into, const FFeelMaterialParameter& From);
};

/** Sink that combines one instance's contributions. Offsets add; flashes, time, post-process and motors keep the strongest. */
class FEELCORE_API FFeelOutputAccumulator : public IFeelOutputSink
{
public:
	FFeelFrameOutput Output;

	virtual void AddCameraOffset(const FVector& LocationOffset, const FRotator& RotationOffset) override;
	virtual void AddFieldOfViewOffset(float Degrees) override;
	virtual void AddScreenFlash(const FLinearColor& Color, float Alpha) override;
	virtual void AddTargetScale(const FVector& ScaleDelta) override;
	virtual void AddTimeDilation(EFeelTimeScope Scope, float Dilation, int32 Priority) override;
	virtual void AddPostProcess(EFeelPostProcessParameter Parameter, float Value, float Weight) override;
	virtual void AddMaterialParameter(const FFeelMaterialParameter& Parameter) override;
	virtual void AddForceFeedback(const FFeelForceFeedbackValues& Values) override;
	virtual void AddScreenTint(const FLinearColor& Color, float Weight) override;
	virtual void AddScreenFade(const FLinearColor& Color, float Alpha) override;
	virtual void AddPostProcessMaterial(UMaterialInterface* Material, float Weight, FName WeightParameter = NAME_None) override;
	virtual void AddTargetTransform(const FVector& LocationOffset, const FRotator& RotationOffset) override;
	virtual void AddLight(float IntensityDelta, const FLinearColor& Color, float ColorWeight) override;
	virtual void AddSoundClassAdjust(const FFeelSoundClassAdjust& Adjust) override;
	virtual void AddWidgetTransform(const FVector2D& Translation, const FVector2D& ScaleDelta, float AngleDegrees) override;
	virtual void AddWidgetColor(const FLinearColor& Color, float Weight) override;
	virtual void AddOverlayFlash(UMaterialInterface* Material, const FLinearColor& Color, float Amount) override;
};
