// Copyright 2026 Billo. All Rights Reserved.

#include "FeelThumbnailLayout.h"

#include "FeelEditorColors.h"
#include "FeelLibrary.h"
#include "FeelRecipe.h"
#include "FeelStep.h"

namespace FeelThumbnailLayout
{
	FFeelThumbnailLayout Build(const UFeelRecipe& Recipe, int32 MaxBars, int32 SamplesPerBar)
	{
		FFeelThumbnailLayout Layout;
		Layout.Length = Recipe.GetDuration();
		Layout.bLibrary = FeelLibrary::IsLibraryRecipe(&Recipe);
#if WITH_EDITORONLY_DATA
		Layout.FeelingColor = FeelEditorColors::GetFeelingColor(Recipe.Feeling);
#else
		Layout.FeelingColor = FeelEditorColors::GetFeelingColor(FGameplayTag());
#endif

		// An instant track still needs a visible width, and a recipe of instant tracks still needs a scale.
		const float Scale = Layout.Length > UE_KINDA_SMALL_NUMBER ? Layout.Length : 1.0f;
		const float MinWidth = 0.02f;
		const int32 Samples = FMath::Max(SamplesPerBar, 2);

		for (const FFeelTrack& Track : Recipe.Tracks)
		{
			if (Layout.Bars.Num() >= FMath::Max(MaxBars, 1))
			{
				++Layout.HiddenTrackCount;
				continue;
			}

			FFeelThumbnailBar& Bar = Layout.Bars.AddDefaulted_GetRef();
			Bar.Color = FeelEditorColors::GetChannelColor(Track.Channel.IsValid() || !Track.Step ? Track.Channel : Track.Step->GetDefaultChannel());
			Bar.bFaded = !Track.bEnabled || !Track.Step;
			Bar.StartX = FMath::Clamp(Track.StartTime / Scale, 0.0f, 1.0f);
			Bar.EndX = FMath::Clamp(FMath::Max(Track.GetEndTime() / Scale, Bar.StartX + MinWidth), 0.0f, 1.0f);

			Bar.Heights.Reserve(Samples);
			for (int32 Index = 0; Index < Samples; ++Index)
			{
				const float Alpha = static_cast<float>(Index) / static_cast<float>(Samples - 1);
				Bar.Heights.Add(FMath::Clamp(Track.EvaluateIntensityCurve(Alpha), 0.0f, 1.0f));
			}
		}

		if (Recipe.bSustain)
		{
			Layout.bHasSustain = true;
			Layout.SustainStartX = FMath::Clamp(Recipe.SustainStart / Scale, 0.0f, 1.0f);
			Layout.SustainEndX = FMath::Clamp(Recipe.SustainEnd / Scale, Layout.SustainStartX, 1.0f);
		}

		return Layout;
	}
}
