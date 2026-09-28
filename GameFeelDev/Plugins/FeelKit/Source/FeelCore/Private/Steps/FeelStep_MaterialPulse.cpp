// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_MaterialPulse.h"

#include "FeelOutputSink.h"
#include "FeelTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_MaterialPulse)

UFeelStep_MaterialPulse::UFeelStep_MaterialPulse()
{
	Shape = EFeelMotionShape::Smooth;
	Parameters.AddDefaulted();
}

void UFeelStep_MaterialPulse::Evaluate(const FFeelStepEvalContext& Context, IFeelOutputSink& Sink) const
{
	const float Motion = EvaluateMotion(Context) * Context.Intensity;
	const EFeelMaterialParameterRoute SinkRoute = Route == EFeelMaterialPulseRoute::CustomPrimitiveData
		? EFeelMaterialParameterRoute::CustomPrimitiveData
		: EFeelMaterialParameterRoute::MaterialInstance;

	for (const FFeelMaterialPulseParameter& Pulse : Parameters)
	{
		FFeelMaterialParameter Parameter;
		Parameter.Name = Pulse.ParameterName;
		Parameter.Route = SinkRoute;
		Parameter.bIsColor = Pulse.bIsColor;
		if (Pulse.bIsColor)
		{
			Parameter.Color = Pulse.Color;
			Parameter.ColorWeight = FMath::Clamp(Motion, 0.0f, 1.0f);
		}
		else
		{
			Parameter.ScalarOffset = Pulse.ScalarAmount * Motion;
		}
		Sink.AddMaterialParameter(Parameter);
	}
}

#if WITH_EDITOR
void UFeelStep_MaterialPulse::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (Parameters.Num() == 0)
	{
		OutWarnings.Add(NSLOCTEXT("FeelKit", "MaterialPulseNoParameters", "Material Parameter Pulse has no parameters, so it does nothing."));
	}

	for (const FFeelMaterialPulseParameter& Parameter : Parameters)
	{
		if (Parameter.ParameterName.IsNone())
		{
			OutErrors.Add(NSLOCTEXT("FeelKit", "MaterialPulseUnnamedParameter", "Material Parameter Pulse has a parameter without a name."));
			break;
		}
	}
}
#endif

FGameplayTag UFeelStep_MaterialPulse::GetDefaultChannel_Implementation() const
{
	return FeelTags::Actor_Material;
}
