// Copyright 2026 Billo. All Rights Reserved.

#include "FeelEvaluator.h"

#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelStep.h"
#include "FeelTrack.h"
#include "HAL/PlatformProperties.h"
#include "Steps/FeelStep_Meta.h"

namespace FeelEvaluatorPrivate
{
	/** Salts that keep the random choices of one track independent. */
	constexpr uint32 ChanceSalt = 0;
	constexpr uint32 IntensitySalt = 0x51ED27A3;
	constexpr uint32 DurationSalt = 0x2C1B3C6D;
	constexpr uint32 ChoiceSalt = 0x68E31DA4;
	constexpr int32 NestedSalt = 0x1B56C4E9;
}

int32 FFeelEvaluator::Evaluate(const UFeelRecipe& Recipe, float Time, const FFeelEvalParams& Params, IFeelOutputSink& Sink)
{
	int32 EvaluatedCount = 0;
	for (int32 TrackIndex = 0; TrackIndex < Recipe.Tracks.Num(); ++TrackIndex)
	{
		const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
		if (!ShouldEvaluateTrack(Recipe, TrackIndex, Params) || !IsTrackActiveAt(Recipe, TrackIndex, Time, Params))
		{
			continue;
		}

		float ComfortScale = 1.0f;
		const UFeelStep* Step = ResolveTrackStep(Recipe, TrackIndex, Params, ComfortScale);
		if (!Step)
		{
			continue;
		}

		// A Play Recipe track evaluates the inner recipe's tracks at the track's local time.
		if (const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(Step))
		{
			FFeelEvalParams NestedParams;
			if (MakeNestedParams(Recipe, TrackIndex, Time, Params, *RecipeStep, NestedParams))
			{
				EvaluatedCount += Evaluate(*RecipeStep->Recipe, Time - Track.StartTime, NestedParams, Sink);
			}
			continue;
		}

		const float Intensity = ComputeTrackIntensity(Recipe, TrackIndex, Time, Params) * ComfortScale;
		if (FMath::IsNearlyZero(Intensity))
		{
			continue;
		}

		FFeelStepEvalContext Context;
		Context.LocalTime = Time - Track.StartTime;
		Context.Duration = GetTrackDuration(Recipe, TrackIndex, Params);
		Context.Alpha = GetTrackAlpha(Recipe, TrackIndex, Time, Params);
		Context.Intensity = Intensity;
		Context.Seed = static_cast<int32>(HashCombineFast(GetTypeHash(Track.Seed), GetTypeHash(Params.InstanceSeed)));
		Context.Direction = Params.WorldDirection;
		Context.ViewDirection = Params.ViewDirection;
		Context.ViewDirectionFromLocation = Params.ViewDirectionFromLocation;

		Step->Evaluate(Context, Sink);
		++EvaluatedCount;
	}
	return EvaluatedCount;
}

bool FFeelEvaluator::IsTrackActiveAt(const FFeelTrack& Track, float Time)
{
	return Track.Duration > 0.0f && Time >= Track.StartTime && Time <= Track.GetEndTime();
}

float FFeelEvaluator::GetTrackAlpha(const FFeelTrack& Track, float Time)
{
	if (Track.Duration <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp((Time - Track.StartTime) / Track.Duration, 0.0f, 1.0f);
}

float FFeelEvaluator::GetTrackDuration(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params)
{
	if (!Recipe.Tracks.IsValidIndex(TrackIndex))
	{
		return 0.0f;
	}

	const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
	const float BaseDuration = FMath::Max(Track.Duration, 0.0f);
	const FFloatInterval& Range = Track.RandomDurationScale;
	if (BaseDuration <= 0.0f || (FMath::IsNearlyEqual(Range.Min, 1.0f) && FMath::IsNearlyEqual(Range.Max, 1.0f)))
	{
		return BaseDuration;
	}

	const float Roll = GetTrackRoll(Track, TrackIndex, Params.InstanceSeed, FeelEvaluatorPrivate::DurationSalt);
	const float Scale = FMath::Lerp(FMath::Max(Range.Min, 0.0f), FMath::Max(Range.Max, Range.Min), Roll);
	return BaseDuration * FMath::Max(Scale, 0.0f);
}

float FFeelEvaluator::GetTrackEndTime(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params)
{
	return Recipe.Tracks.IsValidIndex(TrackIndex) ? Recipe.Tracks[TrackIndex].StartTime + GetTrackDuration(Recipe, TrackIndex, Params) : 0.0f;
}

float FFeelEvaluator::GetRecipeDuration(const UFeelRecipe& Recipe, const FFeelEvalParams& Params)
{
	float Duration = 0.0f;
	for (int32 TrackIndex = 0; TrackIndex < Recipe.Tracks.Num(); ++TrackIndex)
	{
		Duration = FMath::Max(Duration, GetTrackEndTime(Recipe, TrackIndex, Params));
	}
	return Duration;
}

bool FFeelEvaluator::IsTrackActiveAt(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params)
{
	if (!Recipe.Tracks.IsValidIndex(TrackIndex))
	{
		return false;
	}
	const float Duration = GetTrackDuration(Recipe, TrackIndex, Params);
	const float StartTime = Recipe.Tracks[TrackIndex].StartTime;
	return Duration > 0.0f && Time >= StartTime && Time <= StartTime + Duration;
}

float FFeelEvaluator::GetTrackAlpha(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params)
{
	const float Duration = GetTrackDuration(Recipe, TrackIndex, Params);
	if (Duration <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp((Time - Recipe.Tracks[TrackIndex].StartTime) / Duration, 0.0f, 1.0f);
}

float FFeelEvaluator::ComputeTrackIntensity(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params)
{
	if (!Recipe.Tracks.IsValidIndex(TrackIndex))
	{
		return 0.0f;
	}

	const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
	const float TrackScale = Params.TrackScales.IsValidIndex(TrackIndex) ? Params.TrackScales[TrackIndex] : 1.0f;
	return Params.Intensity
		* Recipe.DefaultIntensity
		* Track.EvaluateIntensityCurve(GetTrackAlpha(Recipe, TrackIndex, Time, Params))
		* ComputeParameterScale(Recipe, Track, Params)
		* GetRandomIntensityScale(Track, TrackIndex, Params)
		* TrackScale;
}

float FFeelEvaluator::GetParameterValue(const FFeelRecipeParameter& Parameter, const FFeelEvalParams& Params)
{
	const float* Value = Params.ParameterValues ? Params.ParameterValues->Find(Parameter.Name) : nullptr;
	const float RawValue = Value ? *Value : Parameter.DefaultValue;
	return Parameter.MaxValue > Parameter.MinValue ? FMath::Clamp(RawValue, Parameter.MinValue, Parameter.MaxValue) : RawValue;
}

bool FFeelEvaluator::IsReleaseParameterReached(const UFeelRecipe& Recipe, const FFeelEvalParams& Params)
{
	if (Recipe.ReleaseParameter.IsNone() || !FFeelPlaybackClock::HasSustain(Recipe))
	{
		return false;
	}
	const FFeelRecipeParameter* Parameter = Recipe.FindParameter(Recipe.ReleaseParameter);
	if (!Parameter)
	{
		return false;
	}
	const float Reached = Parameter->Normalize(GetParameterValue(*Parameter, Params));
	return Reached >= FMath::Clamp(Recipe.ReleaseAt, 0.0f, 1.0f) - UE_KINDA_SMALL_NUMBER;
}

float FFeelEvaluator::ComputeParameterScale(const UFeelRecipe& Recipe, const FFeelTrack& Track, const FFeelEvalParams& Params)
{
	float Scale = 1.0f;
	for (const FFeelParameterMapping& Mapping : Track.ParameterMappings)
	{
		if (const FFeelRecipeParameter* Parameter = Recipe.FindParameter(Mapping.Parameter))
		{
			Scale *= Mapping.Evaluate(Parameter->Normalize(GetParameterValue(*Parameter, Params)));
		}
	}
	return Scale;
}

float FFeelEvaluator::GetRandomIntensityScale(const FFeelTrack& Track, int32 TrackIndex, const FFeelEvalParams& Params)
{
	const FFloatInterval& Range = Track.RandomIntensity;
	if (FMath::IsNearlyEqual(Range.Min, Range.Max))
	{
		return FMath::Max(Range.Min, 0.0f);
	}

	const float Roll = GetTrackRoll(Track, TrackIndex, Params.InstanceSeed, FeelEvaluatorPrivate::IntensitySalt);
	return FMath::Max(FMath::Lerp(Range.Min, FMath::Max(Range.Max, Range.Min), Roll), 0.0f);
}

bool FFeelEvaluator::ShouldEvaluateTrack(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params)
{
	if (!Recipe.Tracks.IsValidIndex(TrackIndex))
	{
		return false;
	}

	const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
	if (!Track.bEnabled || !Track.Step)
	{
		return false;
	}
	if (Params.TargetFilter.IsSet() && Track.AppliesTo != Params.TargetFilter.GetValue())
	{
		return false;
	}
	if (Track.AppliesTo == EFeelTrackTarget::Instigator && !Params.bHasInstigator)
	{
		return false;
	}
#if WITH_EDITORONLY_DATA
	if (Params.bRespectSolo && !Track.bSolo && Recipe.HasSoloTracks())
	{
		return false;
	}
#endif
	return PassesConditions(Track, TrackIndex, Params);
}

bool FFeelEvaluator::PassesConditions(const FFeelTrack& Track, int32 TrackIndex, const FFeelEvalParams& Params)
{
	const FFeelConditions& Conditions = Track.Conditions;

	if (Conditions.Platforms.Num() > 0 && !Conditions.Platforms.Contains(FName(FPlatformProperties::IniPlatformName())))
	{
		return false;
	}

	if (Conditions.bLocalPlayerOnly && !Params.bTargetIsLocalPlayer)
	{
		return false;
	}

	if (Conditions.MaxDistance > 0.0f && Params.TargetDistance >= 0.0f && Params.TargetDistance > Conditions.MaxDistance)
	{
		return false;
	}

	if (!PassesReleaseCondition(Conditions.Release, Params))
	{
		return false;
	}

	if (Conditions.Chance < 1.0f)
	{
		return Conditions.Chance > 0.0f && GetChanceRoll(Track, TrackIndex, Params.InstanceSeed) < Conditions.Chance;
	}
	return true;
}

bool FFeelEvaluator::PassesReleaseCondition(EFeelReleaseCondition Condition, const FFeelEvalParams& Params)
{
	switch (Condition)
	{
	case EFeelReleaseCondition::WhenReleaseParameterReached:
		return Params.bReleaseReached;
	case EFeelReleaseCondition::WhenReleasedEarly:
		return Params.bReleased && !Params.bReleaseReached;
	default:
		return true;
	}
}

float FFeelEvaluator::GetChanceRoll(const FFeelTrack& Track, int32 TrackIndex, int32 InstanceSeed)
{
	return GetTrackRoll(Track, TrackIndex, InstanceSeed, FeelEvaluatorPrivate::ChanceSalt);
}

float FFeelEvaluator::GetTrackRoll(const FFeelTrack& Track, int32 TrackIndex, int32 InstanceSeed, uint32 Salt)
{
	uint32 Hash = HashCombineFast(GetTypeHash(InstanceSeed), GetTypeHash(TrackIndex));
	Hash = HashCombineFast(Hash, GetTypeHash(Track.Seed));
	Hash = HashCombineFast(Hash, Salt);

	// Finalizer so neighboring seeds give unrelated rolls.
	Hash ^= Hash >> 16;
	Hash *= 0x7feb352dU;
	Hash ^= Hash >> 15;
	Hash *= 0x846ca68bU;
	Hash ^= Hash >> 16;
	return static_cast<float>(Hash >> 8) / static_cast<float>(1 << 24);
}

bool FFeelEvaluator::HasInstigatorTracks(const UFeelRecipe& Recipe)
{
	// Direct tracks, and tracks of recipes played by Play Recipe tracks, up to the nesting limit.
	TArray<const UFeelRecipe*, TInlineAllocator<4>> Pending = { &Recipe };
	for (int32 Visited = 0; Visited < Pending.Num() && Visited < 16; ++Visited)
	{
		for (const FFeelTrack& Track : Pending[Visited]->Tracks)
		{
			if (Track.AppliesTo == EFeelTrackTarget::Instigator)
			{
				return true;
			}
			const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(Track.Step);
			if (RecipeStep && RecipeStep->Recipe && !Pending.Contains(RecipeStep->Recipe.Get()))
			{
				Pending.Add(RecipeStep->Recipe);
			}
		}
	}
	return false;
}

UFeelStep* FFeelEvaluator::ResolveStep(const FFeelTrack& Track, const FFeelComfortContext& Comfort, float& OutComfortScale)
{
	OutComfortScale = 1.0f;
	if (!Track.Step || !Comfort.Scales)
	{
		return Track.Step;
	}

	const float ChannelScale = Comfort.GetScale(Track.Channel);

	// Essential tracks without a substitute never drop below their floor, except on camera shake and motion,
	// where the player's setting always wins.
	const bool bUsesFloor = Track.bEssential && !Track.SubstituteStep && !FeelComfort::IsMotionGroup(Comfort.GetGroup(Track.Channel));

	if (!FMath::IsNearlyZero(ChannelScale))
	{
		OutComfortScale = bUsesFloor ? FMath::Max(ChannelScale, Track.EssentialFloor) : ChannelScale;
		return Track.Step;
	}

	// Non-essential tracks on a disabled channel are skipped entirely.
	if (!Track.bEssential)
	{
		return nullptr;
	}

	// The substitute plays instead, scaled by its own channel.
	if (Track.SubstituteStep)
	{
		OutComfortScale = Comfort.GetScale(Track.SubstituteStep->GetDefaultChannel());
		return FMath::IsNearlyZero(OutComfortScale) ? nullptr : Track.SubstituteStep.Get();
	}

	// No substitute, keep the essential information at the floor.
	OutComfortScale = bUsesFloor ? Track.EssentialFloor : 0.0f;
	return FMath::IsNearlyZero(OutComfortScale) ? nullptr : Track.Step.Get();
}

UFeelStep* FFeelEvaluator::ResolveTrackStep(const UFeelRecipe& Recipe, int32 TrackIndex, const FFeelEvalParams& Params, float& OutComfortScale)
{
	OutComfortScale = 1.0f;
	if (!Recipe.Tracks.IsValidIndex(TrackIndex))
	{
		return nullptr;
	}

	const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
	UFeelStep* Step = ResolveStep(Track, Params.Comfort, OutComfortScale);

	// Random choices pick one option per play; options may be choices themselves, up to a small depth.
	for (int32 Depth = 0; Depth < UFeelStep_Recipe::MaxNestingDepth; ++Depth)
	{
		const UFeelStep_RandomChoice* Choice = Cast<UFeelStep_RandomChoice>(Step);
		if (!Choice)
		{
			break;
		}
		Step = Choice->ChooseOption(GetTrackRoll(Track, TrackIndex, Params.InstanceSeed, FeelEvaluatorPrivate::ChoiceSalt + static_cast<uint32>(Depth)));
	}
	return Cast<UFeelStep_RandomChoice>(Step) ? nullptr : Step;
}

bool FFeelEvaluator::MakeNestedParams(const UFeelRecipe& Recipe, int32 TrackIndex, float Time, const FFeelEvalParams& Params, const UFeelStep_Recipe& Step, FFeelEvalParams& OutNestedParams)
{
	if (!Step.Recipe || Step.Recipe == &Recipe || Params.NestingDepth >= UFeelStep_Recipe::MaxNestingDepth)
	{
		return false;
	}

	OutNestedParams = Params;
	OutNestedParams.Intensity = ComputeTrackIntensity(Recipe, TrackIndex, Time, Params) * FMath::Max(Step.IntensityScale, 0.0f);
	OutNestedParams.InstanceSeed = static_cast<int32>(HashCombineFast(GetTypeHash(Params.InstanceSeed), GetTypeHash(TrackIndex + FeelEvaluatorPrivate::NestedSalt)));
	OutNestedParams.NestingDepth = Params.NestingDepth + 1;
	OutNestedParams.bRespectSolo = false;
	OutNestedParams.TrackScales = TConstArrayView<float>();
	return true;
}

namespace FeelEvaluatorPrivate
{
	/** Adds one recipe's per-channel intensity at Time into sample SampleIndex, including Play Recipe tracks. */
	void AccumulateChannelIntensities(const UFeelRecipe& Recipe, float Time, float SampleSpacing, int32 SampleIndex, int32 SampleCount, const FFeelEvalParams& Params, TArray<FFeelChannelIntensity>& InOutChannels)
	{
		for (int32 TrackIndex = 0; TrackIndex < Recipe.Tracks.Num(); ++TrackIndex)
		{
			const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
			if (!FFeelEvaluator::ShouldEvaluateTrack(Recipe, TrackIndex, Params))
			{
				continue;
			}

			float ComfortScale = 1.0f;
			const UFeelStep* Step = FFeelEvaluator::ResolveTrackStep(Recipe, TrackIndex, Params, ComfortScale);
			if (!Step)
			{
				continue;
			}

			if (const UFeelStep_Recipe* RecipeStep = Cast<UFeelStep_Recipe>(Step))
			{
				FFeelEvalParams NestedParams;
				if (FFeelEvaluator::IsTrackActiveAt(Recipe, TrackIndex, Time, Params) && FFeelEvaluator::MakeNestedParams(Recipe, TrackIndex, Time, Params, *RecipeStep, NestedParams))
				{
					AccumulateChannelIntensities(*RecipeStep->Recipe, Time - Track.StartTime, SampleSpacing, SampleIndex, SampleCount, NestedParams, InOutChannels);
				}
				continue;
			}

			float Value = 0.0f;
			if (FFeelEvaluator::IsTrackActiveAt(Recipe, TrackIndex, Time, Params))
			{
				Value = FFeelEvaluator::ComputeTrackIntensity(Recipe, TrackIndex, Time, Params) * ComfortScale;
			}
			else if (FFeelEvaluator::GetTrackDuration(Recipe, TrackIndex, Params) <= 0.0f && FMath::Abs(Time - Track.StartTime) <= SampleSpacing * 0.5f)
			{
				// Instant tracks show as a spike at the nearest sample.
				Value = FFeelEvaluator::ComputeTrackIntensity(Recipe, TrackIndex, Track.StartTime, Params) * ComfortScale;
			}

			FFeelChannelIntensity* Channel = InOutChannels.FindByPredicate([&Track](const FFeelChannelIntensity& Existing)
			{
				return Existing.Channel == Track.Channel;
			});
			if (!Channel)
			{
				Channel = &InOutChannels.AddDefaulted_GetRef();
				Channel->Channel = Track.Channel;
				Channel->Samples.SetNumZeroed(SampleCount);
			}
			Channel->Samples[SampleIndex] = FMath::Max(Channel->Samples[SampleIndex], Value);
		}
	}
}

void FFeelEvaluator::SampleChannelIntensities(const UFeelRecipe& Recipe, int32 NumSamples, const FFeelEvalParams& Params, TArray<FFeelChannelIntensity>& OutChannels)
{
	OutChannels.Reset();

	const int32 SampleCount = FMath::Max(NumSamples, 2);
	const float Length = GetRecipeDuration(Recipe, Params);
	const float SampleSpacing = Length / static_cast<float>(SampleCount - 1);

	for (int32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		const float Time = SampleSpacing * static_cast<float>(SampleIndex);
		FeelEvaluatorPrivate::AccumulateChannelIntensities(Recipe, Time, SampleSpacing, SampleIndex, SampleCount, Params, OutChannels);
	}

	OutChannels.Sort([](const FFeelChannelIntensity& A, const FFeelChannelIntensity& B)
	{
		return A.Channel.GetTagName().LexicalLess(B.Channel.GetTagName());
	});
}
