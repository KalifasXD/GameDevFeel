// Copyright 2026 Billo. All Rights Reserved.

#include "FeelTrack.h"

#include "FeelStep.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelTrack)

FFeelTrack::FFeelTrack()
{
	// Default envelope: full intensity at the start, fading linearly to zero.
	FRichCurve* Curve = IntensityCurve.GetRichCurve();
	Curve->SetKeyInterpMode(Curve->AddKey(0.0f, 1.0f), RCIM_Linear);
	Curve->SetKeyInterpMode(Curve->AddKey(1.0f, 0.0f), RCIM_Linear);
}

float FFeelTrack::EvaluateIntensityCurve(float Alpha) const
{
	const FRichCurve* Curve = IntensityCurve.GetRichCurveConst();
	if (!Curve || Curve->GetNumKeys() == 0)
	{
		return 1.0f;
	}
	return Curve->Eval(Alpha, 1.0f);
}
