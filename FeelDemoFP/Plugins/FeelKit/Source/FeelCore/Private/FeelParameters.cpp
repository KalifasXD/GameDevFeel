// Copyright 2026 Billo. All Rights Reserved.

#include "FeelParameters.h"

#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelParameters)

float FFeelRecipeParameter::Normalize(float Value) const
{
	if (MaxValue <= MinValue)
	{
		return 1.0f;
	}
	return FMath::Clamp((Value - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
}

FFeelParameterMapping::FFeelParameterMapping()
{
	// Default: intensity grows linearly with the parameter.
	FRichCurve* RichCurve = Curve.GetRichCurve();
	RichCurve->SetKeyInterpMode(RichCurve->AddKey(0.0f, 0.0f), RCIM_Linear);
	RichCurve->SetKeyInterpMode(RichCurve->AddKey(1.0f, 1.0f), RCIM_Linear);
}

float FFeelParameterMapping::Evaluate(float NormalizedValue) const
{
	const FRichCurve* RichCurve = Curve.GetRichCurveConst();
	if (!RichCurve || RichCurve->GetNumKeys() == 0)
	{
		return NormalizedValue;
	}
	return FMath::Max(RichCurve->Eval(NormalizedValue, NormalizedValue), 0.0f);
}
