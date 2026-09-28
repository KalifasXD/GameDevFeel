// Copyright 2026 Billo. All Rights Reserved.

#include "FeelFrameOutput.h"

#include "Engine/Scene.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/Package.h"

namespace FeelFrameOutput
{
	/** Lowest dilation requested, matching the engine's default minimum global time dilation. */
	constexpr float MinDilation = 0.0001f;
}

void FFeelTimeRequest::Merge(float OtherDilation, int32 OtherPriority)
{
	if (!bActive || OtherPriority > Priority || (OtherPriority == Priority && OtherDilation < Dilation))
	{
		bActive = true;
		Dilation = OtherDilation;
		Priority = OtherPriority;
	}
}

void FFeelFrameOutput::Reset()
{
	*this = FFeelFrameOutput();
}

void FFeelFrameOutput::ApplyToView(FVector& InOutLocation, FRotator& InOutRotation, float& InOutFieldOfView) const
{
	InOutLocation += InOutRotation.RotateVector(CameraLocationOffset);
	InOutRotation = (InOutRotation.Quaternion() * CameraRotationOffset.Quaternion()).Rotator();
	InOutFieldOfView = FMath::Clamp(InOutFieldOfView + FieldOfViewOffset, 5.0f, 170.0f);
}

FLinearColor FFeelFrameOutput::GetOverlayColor() const
{
	return FLinearColor(FlashColor.R, FlashColor.G, FlashColor.B, FMath::Clamp(FlashAlpha, 0.0f, 1.0f));
}

namespace FeelFrameOutput
{
	/** Composites Color at Alpha over an existing overlay: two stacked lerps collapse into one. */
	void CompositeOver(FLinearColor& InOutOverlayColor, const FLinearColor& Color, float Alpha)
	{
		const float OverAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		if (OverAlpha <= 0.0f)
		{
			return;
		}

		const float UnderAlpha = FMath::Clamp(InOutOverlayColor.A, 0.0f, 1.0f);
		const float UnderWeight = UnderAlpha * (1.0f - OverAlpha);
		const float OutAlpha = 1.0f - (1.0f - UnderAlpha) * (1.0f - OverAlpha);

		InOutOverlayColor = FLinearColor(
			(InOutOverlayColor.R * UnderWeight + Color.R * OverAlpha) / OutAlpha,
			(InOutOverlayColor.G * UnderWeight + Color.G * OverAlpha) / OutAlpha,
			(InOutOverlayColor.B * UnderWeight + Color.B * OverAlpha) / OutAlpha,
			OutAlpha);
	}
}

void FFeelFrameOutput::ApplyOverlay(FLinearColor& InOutOverlayColor) const
{
	// Existing overlay (such as a camera fade), then FeelKit fades, then flashes on top.
	FeelFrameOutput::CompositeOver(InOutOverlayColor, FadeColor, FadeAlpha);
	FeelFrameOutput::CompositeOver(InOutOverlayColor, FlashColor, FlashAlpha);
}

void FFeelFrameOutput::ForEachPostProcessBlend(TFunctionRef<void(FPostProcessSettings& Settings, float Weight)> Blend, FFeelPostProcessMaterialInstances* MaterialInstances) const
{
	for (int32 Index = 0; Index < NumPostProcessParameters; ++Index)
	{
		const FFeelPostProcessContribution& Contribution = PostProcess[Index];
		if (Contribution.Weight <= 0.0f)
		{
			continue;
		}

		FPostProcessSettings Settings;
		switch (static_cast<EFeelPostProcessParameter>(Index))
		{
		case EFeelPostProcessParameter::VignetteIntensity:
			Settings.bOverride_VignetteIntensity = true;
			Settings.VignetteIntensity = Contribution.Value;
			break;
		case EFeelPostProcessParameter::ChromaticAberration:
			Settings.bOverride_SceneFringeIntensity = true;
			Settings.SceneFringeIntensity = Contribution.Value;
			break;
		case EFeelPostProcessParameter::Saturation:
			Settings.bOverride_ColorSaturation = true;
			Settings.ColorSaturation = FVector4(Contribution.Value, Contribution.Value, Contribution.Value, 1.0f);
			break;
		default:
			continue;
		}
		Blend(Settings, FMath::Clamp(Contribution.Weight, 0.0f, 1.0f));
	}

	if (TintWeight > 0.0f)
	{
		FPostProcessSettings Settings;
		Settings.bOverride_SceneColorTint = true;
		Settings.SceneColorTint = TintColor;
		Blend(Settings, FMath::Clamp(TintWeight, 0.0f, 1.0f));
	}

	for (const FFeelPostProcessMaterial& Material : PostProcessMaterials)
	{
		if (!Material.Material || Material.Weight <= 0.0f)
		{
			continue;
		}

		FPostProcessSettings Settings;
		const float Weight = FMath::Clamp(Material.Weight, 0.0f, 1.0f);
		UMaterialInstanceDynamic* Instance = MaterialInstances && !Material.WeightParameter.IsNone()
			? MaterialInstances->Get(Material.Material, Material.WeightParameter, Weight)
			: nullptr;
		if (Instance)
		{
			// The material fades itself through the parameter; the blend itself is full.
			Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, Instance));
			Blend(Settings, 1.0f);
		}
		else
		{
			Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, Material.Material));
			Blend(Settings, Weight);
		}
	}
}

void FFeelFrameOutput::MergeSoundClassAdjust(TArray<FFeelSoundClassAdjust, TInlineAllocator<1>>& Into, const FFeelSoundClassAdjust& Adjust)
{
	if (!Adjust.SoundClass)
	{
		return;
	}

	for (FFeelSoundClassAdjust& Existing : Into)
	{
		if (Existing.SoundClass == Adjust.SoundClass)
		{
			Existing.Volume = FMath::Min(Existing.Volume, Adjust.Volume);
			if (FMath::Abs(Adjust.Pitch - 1.0f) > FMath::Abs(Existing.Pitch - 1.0f))
			{
				Existing.Pitch = Adjust.Pitch;
			}
			Existing.LowPassFrequency = FMath::Min(Existing.LowPassFrequency, Adjust.LowPassFrequency);
			return;
		}
	}
	Into.Add(Adjust);
}

void FFeelFrameOutput::MergePostProcessMaterial(TArray<FFeelPostProcessMaterial, TInlineAllocator<1>>& Into, const FFeelPostProcessMaterial& Material)
{
	if (!Material.Material || Material.Weight <= 0.0f)
	{
		return;
	}

	for (FFeelPostProcessMaterial& Existing : Into)
	{
		if (Existing.Material == Material.Material && Existing.WeightParameter == Material.WeightParameter)
		{
			Existing.Weight = FMath::Max(Existing.Weight, FMath::Clamp(Material.Weight, 0.0f, 1.0f));
			return;
		}
	}
	Into.Add({ Material.Material, FMath::Clamp(Material.Weight, 0.0f, 1.0f), Material.WeightParameter });
}

FVector FFeelFrameOutput::ApplyToScale(const FVector& BaseScale) const
{
	return BaseScale * (FVector::OneVector + TargetScaleDelta);
}

bool FFeelFrameOutput::IsSameMaterialParameter(const FFeelMaterialParameter& A, const FFeelMaterialParameter& B)
{
	return A.Name == B.Name && A.Route == B.Route && A.bIsColor == B.bIsColor;
}

void FFeelFrameOutput::MergeMaterialParameter(FFeelMaterialParameter& Into, const FFeelMaterialParameter& From)
{
	if (From.bIsColor)
	{
		const float Weight = FMath::Clamp(From.ColorWeight, 0.0f, 1.0f);
		if (Weight > Into.ColorWeight)
		{
			Into.ColorWeight = Weight;
			Into.Color = From.Color;
		}
	}
	else
	{
		Into.ScalarOffset += From.ScalarOffset;
	}
}

void FFeelOutputAccumulator::AddCameraOffset(const FVector& LocationOffset, const FRotator& RotationOffset)
{
	Output.CameraLocationOffset += LocationOffset;
	Output.CameraRotationOffset += RotationOffset;
}

void FFeelOutputAccumulator::AddFieldOfViewOffset(float Degrees)
{
	Output.FieldOfViewOffset += Degrees;
}

void FFeelOutputAccumulator::AddScreenFlash(const FLinearColor& Color, float Alpha)
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	if (ClampedAlpha > Output.FlashAlpha)
	{
		Output.FlashAlpha = ClampedAlpha;
		Output.FlashColor = Color;
	}
}

void FFeelOutputAccumulator::AddTargetScale(const FVector& ScaleDelta)
{
	Output.TargetScaleDelta += ScaleDelta;
}

void FFeelOutputAccumulator::AddTimeDilation(EFeelTimeScope Scope, float Dilation, int32 Priority)
{
	const float ClampedDilation = FMath::Max(Dilation, FeelFrameOutput::MinDilation);
	FFeelTimeRequest& Request = Scope == EFeelTimeScope::Global ? Output.GlobalTimeDilation : Output.TargetTimeDilation;
	Request.Merge(ClampedDilation, Priority);
}

void FFeelOutputAccumulator::AddPostProcess(EFeelPostProcessParameter Parameter, float Value, float Weight)
{
	const int32 Index = static_cast<int32>(Parameter);
	if (Index < 0 || Index >= FFeelFrameOutput::NumPostProcessParameters)
	{
		return;
	}

	const float ClampedWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
	FFeelPostProcessContribution& Contribution = Output.PostProcess[Index];
	if (ClampedWeight > Contribution.Weight)
	{
		Contribution.Weight = ClampedWeight;
		Contribution.Value = Value;
	}
}

void FFeelOutputAccumulator::AddMaterialParameter(const FFeelMaterialParameter& Parameter)
{
	if (Parameter.Name.IsNone())
	{
		return;
	}

	for (FFeelMaterialParameter& Existing : Output.MaterialParameters)
	{
		if (FFeelFrameOutput::IsSameMaterialParameter(Existing, Parameter))
		{
			FFeelFrameOutput::MergeMaterialParameter(Existing, Parameter);
			return;
		}
	}

	FFeelMaterialParameter& Added = Output.MaterialParameters.Add_GetRef(Parameter);
	Added.ColorWeight = FMath::Clamp(Added.ColorWeight, 0.0f, 1.0f);
}

void FFeelOutputAccumulator::AddForceFeedback(const FFeelForceFeedbackValues& Values)
{
	Output.ForceFeedback.KeepStrongest(Values);
}

void FFeelOutputAccumulator::AddScreenTint(const FLinearColor& Color, float Weight)
{
	const float ClampedWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
	if (ClampedWeight > Output.TintWeight)
	{
		Output.TintWeight = ClampedWeight;
		Output.TintColor = Color;
	}
}

void FFeelOutputAccumulator::AddScreenFade(const FLinearColor& Color, float Alpha)
{
	const float ClampedAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
	if (ClampedAlpha > Output.FadeAlpha)
	{
		Output.FadeAlpha = ClampedAlpha;
		Output.FadeColor = Color;
	}
}

void FFeelOutputAccumulator::AddPostProcessMaterial(UMaterialInterface* Material, float Weight, FName WeightParameter)
{
	FFeelFrameOutput::MergePostProcessMaterial(Output.PostProcessMaterials, { Material, Weight, WeightParameter });
}

void FFeelOutputAccumulator::AddTargetTransform(const FVector& LocationOffset, const FRotator& RotationOffset)
{
	Output.TargetLocationOffset += LocationOffset;
	Output.TargetRotationOffset += RotationOffset;
}

void FFeelOutputAccumulator::AddLight(float IntensityDelta, const FLinearColor& Color, float ColorWeight)
{
	Output.LightIntensityDelta += IntensityDelta;
	const float ClampedWeight = FMath::Clamp(ColorWeight, 0.0f, 1.0f);
	if (ClampedWeight > Output.LightColorWeight)
	{
		Output.LightColorWeight = ClampedWeight;
		Output.LightColor = Color;
	}
}

void FFeelOutputAccumulator::AddSoundClassAdjust(const FFeelSoundClassAdjust& Adjust)
{
	FFeelFrameOutput::MergeSoundClassAdjust(Output.SoundClassAdjusts, Adjust);
}

void FFeelOutputAccumulator::AddWidgetTransform(const FVector2D& Translation, const FVector2D& ScaleDelta, float AngleDegrees)
{
	Output.WidgetTranslation += Translation;
	Output.WidgetScaleDelta += ScaleDelta;
	Output.WidgetAngle += AngleDegrees;
}

void FFeelOutputAccumulator::AddWidgetColor(const FLinearColor& Color, float Weight)
{
	const float ClampedWeight = FMath::Clamp(Weight, 0.0f, 1.0f);
	if (ClampedWeight > Output.WidgetColorWeight)
	{
		Output.WidgetColorWeight = ClampedWeight;
		Output.WidgetColor = Color;
	}
}

FFeelPostProcessMaterialInstances::FFeelPostProcessMaterialInstances() = default;
FFeelPostProcessMaterialInstances::~FFeelPostProcessMaterialInstances() = default;

void FFeelPostProcessMaterialInstances::Reset()
{
	Instances.Reset();
}

UMaterialInstanceDynamic* FFeelPostProcessMaterialInstances::Get(UMaterialInterface* Material, FName Parameter, float Weight)
{
	if (!Material || Parameter.IsNone())
	{
		return nullptr;
	}

	TStrongObjectPtr<UMaterialInstanceDynamic>& Instance = Instances.FindOrAdd(TPair<FObjectKey, FName>(FObjectKey(Material), Parameter));
	if (!Instance.IsValid())
	{
		Instance.Reset(UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));
	}
	if (Instance.IsValid())
	{
		Instance->SetScalarParameterValue(Parameter, Weight);
	}
	return Instance.Get();
}

void FFeelOutputAccumulator::AddOverlayFlash(UMaterialInterface* Material, const FLinearColor& Color, float Amount)
{
	const float Clamped = FMath::Clamp(Amount, 0.0f, 1.0f);
	if (Material && Clamped > Output.OverlayFlashAmount)
	{
		Output.OverlayFlashMaterial = Material;
		Output.OverlayFlashColor = Color;
		Output.OverlayFlashAmount = Clamped;
	}
}
