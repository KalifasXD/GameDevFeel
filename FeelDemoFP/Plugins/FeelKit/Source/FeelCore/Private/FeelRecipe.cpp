// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipe.h"

#include "UObject/AssetRegistryTagsContext.h"

#include "FeelSettings.h"

#include "FeelStep.h"
#include "Steps/FeelStep_Meta.h"

#if WITH_EDITOR
#include "FeelComfortTypes.h"
#include "Misc/DataValidation.h"
#endif

#include "UObject/ObjectSaveContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelRecipe)

#define LOCTEXT_NAMESPACE "FeelRecipe"

const FName UFeelRecipe::FeelingTagName(TEXT("FeelFeeling"));
const FName UFeelRecipe::GenresTagName(TEXT("FeelGenres"));
const FName UFeelRecipe::DescriptionTagName(TEXT("FeelDescription"));
const FName UFeelRecipe::ChannelsTagName(TEXT("FeelChannels"));
const FName UFeelRecipe::TrackCountTagName(TEXT("FeelTrackCount"));
const FName UFeelRecipe::LengthTagName(TEXT("FeelLength"));
const FName UFeelRecipe::SustainedTagName(TEXT("FeelSustained"));

float UFeelRecipe::GetDuration() const
{
	float Duration = 0.0f;
	for (const FFeelTrack& Track : Tracks)
	{
		Duration = FMath::Max(Duration, Track.GetEndTime());
	}
	return Duration;
}

const FFeelRecipeParameter* UFeelRecipe::FindParameter(FName ParameterName) const
{
	if (ParameterName.IsNone())
	{
		return nullptr;
	}
	return Parameters.FindByPredicate([ParameterName](const FFeelRecipeParameter& Parameter) { return Parameter.Name == ParameterName; });
}

TArray<FName> UFeelRecipe::GetParameterNames() const
{
	TArray<FName> Names;
	for (const FFeelRecipeParameter& Parameter : Parameters)
	{
		if (!Parameter.Name.IsNone())
		{
			Names.AddUnique(Parameter.Name);
		}
	}
	return Names;
}

TArray<FName> UFeelRecipe::GetAccumulatorNames() const
{
	TArray<FName> Names;
	Names.Add(NAME_None);
	for (const FFeelAccumulatorDefinition& Definition : GetDefault<UFeelSettings>()->Accumulators)
	{
		if (!Definition.Name.IsNone())
		{
			Names.AddUnique(Definition.Name);
		}
	}
	return Names;
}

void UFeelRecipe::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	SchemaVersion = CurrentSchemaVersion;
}

#if WITH_EDITORONLY_DATA
bool UFeelRecipe::HasSoloTracks() const
{
	return Tracks.ContainsByPredicate([](const FFeelTrack& Track) { return Track.bSolo; });
}
#endif

#if WITH_EDITOR
void UFeelRecipe::GetAssetRegistryTagMetadata(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const
{
	Super::GetAssetRegistryTagMetadata(OutMetadata);

	OutMetadata.Add(DescriptionTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("DescriptionTagLabel", "Description"))
		.SetTooltip(LOCTEXT("DescriptionTagTip", "What this recipe is for.")));
	OutMetadata.Add(FeelingTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("FeelingTagLabel", "Feeling"))
		.SetTooltip(LOCTEXT("FeelingTagTip", "What the recipe makes the player feel, such as Impact or Reward.")));
	OutMetadata.Add(GenresTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("GenresTagLabel", "Genres"))
		.SetTooltip(LOCTEXT("GenresTagTip", "Kinds of game this recipe suits.")));
	OutMetadata.Add(ChannelsTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("ChannelsTagLabel", "Channels"))
		.SetTooltip(LOCTEXT("ChannelsTagTip", "What the recipe affects: camera, screen, actor, time, audio, haptics, UI or spawned effects.")));
	OutMetadata.Add(TrackCountTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("TrackCountTagLabel", "Tracks")));
	OutMetadata.Add(LengthTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("LengthTagLabel", "Length"))
		.SetSuffix(LOCTEXT("LengthTagSuffix", "s")));
	OutMetadata.Add(SustainedTagName, FAssetRegistryTagMetadata()
		.SetDisplayName(LOCTEXT("SustainedTagLabel", "Sustained"))
		.SetTooltip(LOCTEXT("SustainedTagTip", "Whether the recipe keeps playing until it is released.")));
}

EDataValidationResult UFeelRecipe::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	bool bHasErrors = Result == EDataValidationResult::Invalid;

	if (Tracks.Num() == 0)
	{
		Context.AddWarning(LOCTEXT("NoTracks", "Recipe has no tracks."));
	}

	if (bSustain)
	{
		if (SustainEnd - SustainStart < MinSustainLength)
		{
			Context.AddError(LOCTEXT("SustainRegionEmpty", "Sustain is on, but Sustain End is not after Sustain Start, so nothing loops."));
			bHasErrors = true;
		}
		else if (SustainEnd > GetDuration() + UE_KINDA_SMALL_NUMBER)
		{
			Context.AddWarning(LOCTEXT("SustainRegionPastEnd", "Sustain End is after the last track ends, so part of the loop is silent."));
		}
	}

	TSet<FName> SeenParameterNames;
	for (int32 ParameterIndex = 0; ParameterIndex < Parameters.Num(); ++ParameterIndex)
	{
		const FFeelRecipeParameter& Parameter = Parameters[ParameterIndex];
		const FText ParameterLabel = Parameter.Name.IsNone() ? FText::Format(LOCTEXT("UnnamedParameter", "Parameter {0}"), FText::AsNumber(ParameterIndex + 1)) : FText::FromName(Parameter.Name);

		if (Parameter.Name.IsNone())
		{
			Context.AddError(FText::Format(LOCTEXT("ParameterNoName", "{0} has no name, so games cannot set it and tracks cannot use it."), ParameterLabel));
			bHasErrors = true;
		}
		else if (SeenParameterNames.Contains(Parameter.Name))
		{
			Context.AddError(FText::Format(LOCTEXT("ParameterDuplicate", "Parameter {0} is declared more than once."), ParameterLabel));
			bHasErrors = true;
		}
		SeenParameterNames.Add(Parameter.Name);

		if (Parameter.MaxValue <= Parameter.MinValue)
		{
			Context.AddError(FText::Format(LOCTEXT("ParameterRange", "Parameter {0} has a max value that is not above its min value."), ParameterLabel));
			bHasErrors = true;
		}
		else if (Parameter.DefaultValue < Parameter.MinValue || Parameter.DefaultValue > Parameter.MaxValue)
		{
			Context.AddWarning(FText::Format(LOCTEXT("ParameterDefaultOutside", "Parameter {0} has a default value outside its range, so it is clamped."), ParameterLabel));
		}

		if (!Parameter.Accumulator.IsNone() && !GetDefault<UFeelSettings>()->FindAccumulator(Parameter.Accumulator))
		{
			Context.AddWarning(FText::Format(LOCTEXT("ParameterUnknownAccumulator", "Parameter {0} reads accumulator {1}, which is not defined in Project Settings > Plugins > FeelKit, so it uses its default value."), ParameterLabel, FText::FromName(Parameter.Accumulator)));
		}
	}

	for (int32 TrackIndex = 0; TrackIndex < Tracks.Num(); ++TrackIndex)
	{
		const FFeelTrack& Track = Tracks[TrackIndex];
		const FText StepName = Track.Step ? Track.Step->GetClass()->GetDisplayNameText() : LOCTEXT("NoStepName", "No Step");

		auto AddError = [&](const FText& Message)
		{
			Context.AddError(FText::Format(LOCTEXT("TrackError", "Track {0} ({1}): {2}"), FText::AsNumber(TrackIndex + 1), StepName, Message));
			bHasErrors = true;
		};
		auto AddWarning = [&](const FText& Message)
		{
			Context.AddWarning(FText::Format(LOCTEXT("TrackWarning", "Track {0} ({1}): {2}"), FText::AsNumber(TrackIndex + 1), StepName, Message));
		};

		// A missing step would be skipped at runtime.
		if (!Track.Step)
		{
			AddError(LOCTEXT("MissingStep", "has no step, so it is skipped."));
			continue;
		}

		if (Track.Duration <= 0.0f && Track.Step->RequiresDuration())
		{
			AddError(LOCTEXT("ZeroLength", "has a length of 0, but this step needs a length to produce output."));
		}

		if (const FRichCurve* Curve = Track.IntensityCurve.GetRichCurveConst())
		{
			if (Curve->GetNumKeys() > 0)
			{
				bool bKeysOutsideRange = false;
				bool bNegativeValues = false;
				float MaxValue = -UE_BIG_NUMBER;
				for (const FRichCurveKey& Key : Curve->GetConstRefOfKeys())
				{
					bKeysOutsideRange |= Key.Time < -UE_KINDA_SMALL_NUMBER || Key.Time > 1.0f + UE_KINDA_SMALL_NUMBER;
					bNegativeValues |= Key.Value < 0.0f;
					MaxValue = FMath::Max(MaxValue, Key.Value);
				}

				if (bKeysOutsideRange)
				{
					AddWarning(LOCTEXT("CurveOutsideRange", "intensity curve has keys outside 0 to 1. The curve is read over normalized track time, so those keys never play."));
				}
				if (bNegativeValues)
				{
					AddWarning(LOCTEXT("CurveNegative", "intensity curve goes below 0."));
				}
				if (MaxValue <= 0.0f)
				{
					AddWarning(LOCTEXT("CurveSilent", "intensity curve never rises above 0, so the track produces no output."));
				}
			}
		}

		// Comfort settings can remove essential information completely.
		const TConstArrayView<FFeelChannelComfortMapping> ComfortMappings = GetDefault<UFeelSettings>()->ChannelComfortGroups;
		const bool bMotionChannel = FeelComfort::IsMotionGroup(FeelComfort::FindGroup(Track.Channel, ComfortMappings));
		if (Track.bEssential && !Track.SubstituteStep && bMotionChannel)
		{
			AddWarning(LOCTEXT("EssentialMotionWithoutSubstitute", "is essential on a camera shake or camera motion channel without a substitute step. Players who turn that motion off get no motion at all, so the essential floor is ignored here. Add a substitute on another channel, such as a flash or haptics."));
		}
		else if (Track.bEssential && !Track.SubstituteStep && Track.EssentialFloor <= 0.0f)
		{
			AddWarning(LOCTEXT("EssentialWithoutFallback", "is essential but has no substitute step and an essential floor of 0, so comfort settings can remove it completely."));
		}
		if (Track.bEssential && Track.SubstituteStep && bMotionChannel
			&& FeelComfort::IsMotionGroup(FeelComfort::FindGroup(Track.SubstituteStep->GetDefaultChannel(), ComfortMappings)))
		{
			AddWarning(LOCTEXT("MotionSubstituteForMotion", "replaces camera motion with a substitute that also moves the camera, so players who turn motion off get nothing. Use a substitute on another channel."));
		}

		if (Track.Conditions.Chance <= 0.0f)
		{
			AddWarning(LOCTEXT("ChanceZero", "has a chance of 0, so it never plays."));
		}

		if (const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(Track.Step))
		{
			if (RecipeStep->Recipe && RecipeStep->Recipe != this && Track.Duration + UE_KINDA_SMALL_NUMBER < RecipeStep->Recipe->GetDuration())
			{
				AddWarning(FText::Format(LOCTEXT("RecipeStepTooShort", "is shorter ({0} s) than the recipe it plays ({1} s), so the end of that recipe is cut off."),
					FText::AsNumber(Track.Duration), FText::AsNumber(RecipeStep->Recipe->GetDuration())));
			}
		}

		for (const FFeelParameterMapping& Mapping : Track.ParameterMappings)
		{
			if (Mapping.Parameter.IsNone())
			{
				AddError(LOCTEXT("MappingNoParameter", "has a parameter mapping without a parameter."));
			}
			else if (!FindParameter(Mapping.Parameter))
			{
				AddError(FText::Format(LOCTEXT("MappingUnknownParameter", "maps parameter {0}, which the recipe does not declare, so the mapping is ignored."), FText::FromName(Mapping.Parameter)));
			}
		}

		auto CheckRange = [&](const FFloatInterval& Range, const FText& RangeName)
		{
			if (Range.Min < 0.0f || Range.Max < Range.Min)
			{
				AddError(FText::Format(LOCTEXT("RandomRangeInvalid", "{0} needs 0 <= Min <= Max."), RangeName));
			}
		};
		CheckRange(Track.RandomIntensity, LOCTEXT("RandomIntensityName", "Random Intensity"));
		CheckRange(Track.RandomDurationScale, LOCTEXT("RandomDurationName", "Random Duration Scale"));
		if (Track.Duration > 0.0f && Track.RandomDurationScale.Min <= 0.0f && Track.RandomDurationScale.Max > 0.0f)
		{
			AddWarning(LOCTEXT("RandomDurationZero", "Random Duration Scale can reach 0, which turns the track into an instant track for that play."));
		}
		if (Track.SubstituteStep && !Track.bEssential)
		{
			AddWarning(LOCTEXT("SubstituteNotEssential", "has a substitute step but is not essential, so the substitute never plays."));
		}
		if (!Track.Channel.IsValid())
		{
			AddWarning(LOCTEXT("NoChannel", "has no channel, so only the Master comfort scale applies to it."));
		}

		TArray<FText> StepErrors;
		TArray<FText> StepWarnings;
		Track.Step->ValidateStep(StepErrors, StepWarnings);
		if (Track.SubstituteStep)
		{
			Track.SubstituteStep->ValidateStep(StepErrors, StepWarnings);
		}
		for (const FText& Error : StepErrors)
		{
			AddError(Error);
		}
		for (const FText& Warning : StepWarnings)
		{
			AddWarning(Warning);
		}
	}

	return bHasErrors ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

#undef LOCTEXT_NAMESPACE

const FName UFeelRecipe::ParametersTagName(TEXT("FeelParameters"));

void UFeelRecipe::GatherChannels(TSet<FGameplayTag>& OutChannels) const
{
	GatherChannels(OutChannels, 0);
}

void UFeelRecipe::GatherChannels(TSet<FGameplayTag>& OutChannels, int32 Depth) const
{
	if (Depth > UFeelStep_Recipe::MaxNestingDepth)
	{
		return;
	}

	for (const FFeelTrack& Track : Tracks)
	{
		if (!Track.bEnabled || !Track.Step)
		{
			continue;
		}

		OutChannels.Add(Track.Channel.IsValid() ? Track.Channel : Track.Step->GetDefaultChannel());

		// Steps that hold other steps contribute the channels of what they play.
		if (const UFeelStep_Recipe* Nested = Cast<UFeelStep_Recipe>(Track.Step); Nested && Nested->Recipe && Nested->Recipe != this)
		{
			Nested->Recipe->GatherChannels(OutChannels, Depth + 1);
		}
		else if (const UFeelStep_RandomChoice* Choice = Cast<UFeelStep_RandomChoice>(Track.Step))
		{
			for (const FFeelRandomChoiceOption& Option : Choice->Options)
			{
				if (Option.Step)
				{
					OutChannels.Add(Option.Step->GetDefaultChannel());
				}
			}
		}
	}

	OutChannels.Remove(FGameplayTag());
}

void UFeelRecipe::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	Super::GetAssetRegistryTags(Context);

	TSet<FGameplayTag> Channels;
	GatherChannels(Channels);
	TArray<FString> ChannelNames;
	ChannelNames.Reserve(Channels.Num());
	for (const FGameplayTag& Channel : Channels)
	{
		ChannelNames.Add(Channel.GetTagName().ToString());
	}
	ChannelNames.Sort();
	Context.AddTag(FAssetRegistryTag(ChannelsTagName, FString::Join(ChannelNames, TEXT(",")), FAssetRegistryTag::TT_Alphabetical));
	Context.AddTag(FAssetRegistryTag(TrackCountTagName, FString::FromInt(Tracks.Num()), FAssetRegistryTag::TT_Numerical));
	Context.AddTag(FAssetRegistryTag(LengthTagName, FString::Printf(TEXT("%.2f"), GetDuration()), FAssetRegistryTag::TT_Numerical));
	Context.AddTag(FAssetRegistryTag(SustainedTagName, bSustain ? TEXT("True") : TEXT("False"), FAssetRegistryTag::TT_Alphabetical));

#if WITH_EDITORONLY_DATA
	Context.AddTag(FAssetRegistryTag(FeelingTagName, Feeling.GetTagName().ToString(), FAssetRegistryTag::TT_Alphabetical));
	TArray<FString> GenreNames;
	for (const FGameplayTag& Genre : Genres)
	{
		GenreNames.Add(Genre.GetTagName().ToString());
	}
	GenreNames.Sort();
	Context.AddTag(FAssetRegistryTag(GenresTagName, FString::Join(GenreNames, TEXT(",")), FAssetRegistryTag::TT_Alphabetical));
	Context.AddTag(FAssetRegistryTag(DescriptionTagName, Description.ToString(), FAssetRegistryTag::TT_Alphabetical));
#endif

	FString Names;
	for (const FFeelRecipeParameter& Parameter : Parameters)
	{
		if (!Parameter.Name.IsNone())
		{
			Names += Names.IsEmpty() ? Parameter.Name.ToString() : TEXT(",") + Parameter.Name.ToString();
		}
	}
	Context.AddTag(FAssetRegistryTag(ParametersTagName, Names, FAssetRegistryTag::TT_Hidden));
}
