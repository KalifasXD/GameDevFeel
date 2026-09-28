// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Which clock a time dilation request targets. */
enum class EFeelTimeScope : uint8
{
	/** The world's global time dilation. */
	Global,
	/** The target actor's custom time dilation. */
	Target,
};

class UMaterialInterface;
class USoundClass;

/** Post-process parameters steps can drive. */
enum class EFeelPostProcessParameter : uint8
{
	VignetteIntensity,
	ChromaticAberration,
	/** Color saturation multiplier: 1 is unchanged, 0 is grayscale. */
	Saturation,

	Count
};

/** Audio adjustment of one sound class. Volume and pitch multiply; the low-pass cutoff is in Hz (20000 means none). */
struct FFeelSoundClassAdjust
{
	USoundClass* SoundClass = nullptr;
	float Volume = 1.0f;
	float Pitch = 1.0f;
	float LowPassFrequency = 20000.0f;
};

/** A blendable post-process material over the scene. */
struct FFeelPostProcessMaterial
{
	UMaterialInterface* Material = nullptr;
	float Weight = 0.0f;

	/** Scalar parameter that receives Weight in a material instance. None blends the material itself. */
	FName WeightParameter;
};

/** How a material parameter reaches the target's materials. */
enum class EFeelMaterialParameterRoute : uint8
{
	/** Dynamic material instances on the target's mesh components. Works with any material parameter. */
	MaterialInstance,
	/** Custom primitive data: no material instances and better batching; the material must read the parameter that way. */
	CustomPrimitiveData,
};

/** A material parameter contribution. Scalars add to the original value; colors blend from the original toward Color. */
struct FFeelMaterialParameter
{
	FName Name;
	EFeelMaterialParameterRoute Route = EFeelMaterialParameterRoute::MaterialInstance;
	bool bIsColor = false;
	float ScalarOffset = 0.0f;
	FLinearColor Color = FLinearColor::White;
	float ColorWeight = 0.0f;
};

/** Strength of each controller motor, from 0 to 1. */
struct FFeelForceFeedbackValues
{
	float LeftLarge = 0.0f;
	float LeftSmall = 0.0f;
	float RightLarge = 0.0f;
	float RightSmall = 0.0f;

	bool IsZero() const
	{
		return LeftLarge <= 0.0f && LeftSmall <= 0.0f && RightLarge <= 0.0f && RightSmall <= 0.0f;
	}

	/** Keeps the stronger value per motor. */
	void KeepStrongest(const FFeelForceFeedbackValues& Other)
	{
		LeftLarge = FMath::Max(LeftLarge, Other.LeftLarge);
		LeftSmall = FMath::Max(LeftSmall, Other.LeftSmall);
		RightLarge = FMath::Max(RightLarge, Other.RightLarge);
		RightSmall = FMath::Max(RightSmall, Other.RightSmall);
	}
};

/**
 * Receives the contributions steps compute for one evaluated moment.
 * Steps never touch cameras, post process or actors directly; a sink decides how contributions are applied
 * (runtime camera modifier, editor preview, tests).
 */
class FEELCORE_API IFeelOutputSink
{
public:
	virtual ~IFeelOutputSink() = default;

	/** Camera-space offsets: location X forward, Y right, Z up (cm); rotation in degrees. */
	virtual void AddCameraOffset(const FVector& LocationOffset, const FRotator& RotationOffset) = 0;

	/** Field of view change in degrees. */
	virtual void AddFieldOfViewOffset(float Degrees) = 0;

	/** Full-screen color overlay. Alpha is in [0, 1]. */
	virtual void AddScreenFlash(const FLinearColor& Color, float Alpha) = 0;

	/** Relative scale change of the target: 0 leaves it unchanged, 0.2 makes it 20% larger. */
	virtual void AddTargetScale(const FVector& ScaleDelta) = 0;

	/** Requests a time dilation. Per scope, the highest priority wins; equal priorities keep the strongest (lowest) dilation. */
	virtual void AddTimeDilation(EFeelTimeScope Scope, float Dilation, int32 Priority) = 0;

	/** Blends a post-process parameter toward Value over the scene with Weight (0 to 1). Per parameter, the strongest weight wins. */
	virtual void AddPostProcess(EFeelPostProcessParameter Parameter, float Value, float Weight) = 0;

	/** Pulses a material parameter on the target. Scalars with the same name add; colors keep the strongest weight. */
	virtual void AddMaterialParameter(const FFeelMaterialParameter& Parameter) = 0;

	/** Vibrates the controller of the target's player. Per motor, the strongest value wins. */
	virtual void AddForceFeedback(const FFeelForceFeedbackValues& Values) = 0;

	/** Tints the scene toward Color with Weight (0 to 1). The strongest weight wins. */
	virtual void AddScreenTint(const FLinearColor& Color, float Weight) = 0;

	/** Fades the view toward Color. Drawn under flashes. The strongest alpha wins. */
	virtual void AddScreenFade(const FLinearColor& Color, float Alpha) = 0;

	/**
	 * Blends a post-process material over the scene with Weight (0 to 1). Per material, the strongest weight wins. Unreal
	 * applies a post-process material at full strength whenever its weight is above 0, so with a WeightParameter the
	 * weight is also written to that scalar parameter of a material instance, which the material uses to fade itself.
	 */
	virtual void AddPostProcessMaterial(UMaterialInterface* Material, float Weight, FName WeightParameter = NAME_None) = 0;

	/** Moves and rotates the target relative to where it is, in its local space. Offsets add. */
	virtual void AddTargetTransform(const FVector& LocationOffset, const FRotator& RotationOffset) = 0;

	/** Changes the target's lights: intensity multiplied by (1 + IntensityDelta), color blended toward Color by ColorWeight. */
	virtual void AddLight(float IntensityDelta, const FLinearColor& Color, float ColorWeight) = 0;

	/** Adjusts a sound class. Per class, the lowest volume, the pitch farthest from 1 and the lowest cutoff win. */
	virtual void AddSoundClassAdjust(const FFeelSoundClassAdjust& Adjust) = 0;

	/** Moves, scales and rotates a widget target through its render transform (slate units, degrees). Offsets add. */
	virtual void AddWidgetTransform(const FVector2D& Translation, const FVector2D& ScaleDelta, float AngleDegrees) = 0;

	/** Tints a widget target toward Color with Weight (0 to 1). The strongest weight wins. */
	virtual void AddWidgetColor(const FLinearColor& Color, float Weight) = 0;

	/**
	 * Flashes the target's meshes through their overlay material slot, so it works on any mesh whatever its own materials
	 * are. Material is a translucent overlay material that reads a FlashColor vector parameter and a FlashAmount scalar
	 * parameter; Amount goes from 0 to 1. The strongest amount wins.
	 */
	virtual void AddOverlayFlash(UMaterialInterface* Material, const FLinearColor& Color, float Amount) = 0;
};
