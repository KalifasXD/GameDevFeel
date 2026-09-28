// Copyright 2026 Billo. All Rights Reserved.

#include "FeelComfortAudit.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "FeelComfortTypes.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelStep.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/UObjectToken.h"
#include "Modules/ModuleManager.h"
#include "Steps/FeelStep_ScreenFlash.h"

#define LOCTEXT_NAMESPACE "FeelComfortAudit"

namespace FeelComfortAuditPrivate
{
	const FName LogName(TEXT("FeelKitComfortAudit"));

	void Add(TArray<FFeelComfortAuditFinding>& Out, bool bWarning, const FText& Message, const UFeelRecipe* Recipe = nullptr, int32 TrackIndex = INDEX_NONE)
	{
		FFeelComfortAuditFinding& Finding = Out.AddDefaulted_GetRef();
		Finding.bWarning = bWarning;
		Finding.Message = Message;
		Finding.Recipe = Recipe;
		Finding.TrackIndex = TrackIndex;
	}

	FText GetGroupName(EFeelComfortGroup Group)
	{
		return StaticEnum<EFeelComfortGroup>()->GetDisplayNameTextByValue(static_cast<int64>(Group));
	}

	FText DescribeTrack(const UFeelRecipe& Recipe, int32 TrackIndex)
	{
		const UFeelStep* Step = Recipe.Tracks[TrackIndex].Step;
		return FText::Format(LOCTEXT("TrackDescription", "track {0} ({1})"), FText::AsNumber(TrackIndex + 1),
			Step ? Step->GetClass()->GetDisplayNameText() : LOCTEXT("NoStep", "no step"));
	}
}

bool FFeelComfortAudit::IsSaturatedRed(const FLinearColor& Color)
{
	const float Sum = FMath::Max(Color.R, 0.0f) + FMath::Max(Color.G, 0.0f) + FMath::Max(Color.B, 0.0f);
	return Sum > KINDA_SMALL_NUMBER && Color.R / Sum >= 0.8f;
}

void FFeelComfortAudit::AuditRecipe(const UFeelRecipe& Recipe, const UFeelSettings& Settings, TArray<FFeelComfortAuditFinding>& OutFindings)
{
	using namespace FeelComfortAuditPrivate;
	const TConstArrayView<FFeelChannelComfortMapping> Mappings = Settings.ChannelComfortGroups;

	TArray<float> FlashStarts;
	for (int32 TrackIndex = 0; TrackIndex < Recipe.Tracks.Num(); ++TrackIndex)
	{
		const FFeelTrack& Track = Recipe.Tracks[TrackIndex];
		if (!Track.Step || !Track.bEnabled)
		{
			continue;
		}

		const EFeelComfortGroup Group = FeelComfort::FindGroup(Track.Channel, Mappings);
		const EFeelComfortGroup StepGroup = FeelComfort::FindGroup(Track.Step->GetDefaultChannel(), Mappings);

		// A track moved to a channel outside its step's comfort group escapes the player's setting for that kind of effect.
		if (StepGroup != EFeelComfortGroup::None && Group != StepGroup)
		{
			Add(OutFindings, true, FText::Format(LOCTEXT("ChannelEscapesGroup", "{0}: channel {1} is in comfort group {2}, but this kind of effect belongs to {3}. Players who turn {3} down will still get it. Use a channel under the step's default channel, or map this channel to {3} in Project Settings > Plugins > FeelKit."),
				DescribeTrack(Recipe, TrackIndex), FText::FromString(Track.Channel.ToString()), GetGroupName(Group), GetGroupName(StepGroup)), &Recipe, TrackIndex);
		}

		if (Group == EFeelComfortGroup::Flashes)
		{
			FlashStarts.Add(Track.StartTime);

			// Essential flashes without a substitute still flash for players who turned flashes off.
			if (Track.bEssential && !Track.SubstituteStep && Track.EssentialFloor > 0.0f)
			{
				Add(OutFindings, true, FText::Format(LOCTEXT("EssentialFlashFloor", "{0}: essential flash with a floor of {1} and no substitute. Players who turn flashes off still see it at that strength. Add a substitute that is not a flash, such as a sound or a vignette."),
					DescribeTrack(Recipe, TrackIndex), FText::AsNumber(Track.EssentialFloor)), &Recipe, TrackIndex);
			}
		}

		if (const UFeelStep_ScreenFlash* Flash = Cast<UFeelStep_ScreenFlash>(Track.Step))
		{
			if (IsSaturatedRed(Flash->Color) && Flash->MaxOpacity > 0.5f)
			{
				Add(OutFindings, true, FText::Format(LOCTEXT("RedFlash", "{0}: saturated red flash at opacity {1}. Red flashes are the most likely to trigger photosensitive reactions. Lower the opacity to 0.5 or less, or use a less saturated color."),
					DescribeTrack(Recipe, TrackIndex), FText::AsNumber(Flash->MaxOpacity)), &Recipe, TrackIndex);
			}
		}
	}

	// More than three flashes starting within any one second of a single play. Nested recipes are audited on their own.
	FlashStarts.Sort();
	for (int32 First = 0; First < FlashStarts.Num(); ++First)
	{
		int32 Last = First;
		while (Last + 1 < FlashStarts.Num() && FlashStarts[Last + 1] - FlashStarts[First] < 1.0f)
		{
			++Last;
		}
		const int32 Count = Last - First + 1;
		if (Count > MaxFlashesPerSecond)
		{
			Add(OutFindings, true, FText::Format(LOCTEXT("FlashRate", "{0} flashes start within one second (from {1} s). Guidance allows no more than {2} flashes in any second. Spread them out or remove some; the flash limiter reduces extra flashes for players who keep it on."),
				FText::AsNumber(Count), FText::AsNumber(FlashStarts[First]), FText::AsNumber(MaxFlashesPerSecond)), &Recipe);
			break;
		}
	}

	// A sustain loop repeats its flashes for as long as the recipe is held.
	if (FFeelPlaybackClock::HasSustain(Recipe))
	{
		const float LoopLength = Recipe.SustainEnd - Recipe.SustainStart;
		int32 LoopFlashes = 0;
		for (float Start : FlashStarts)
		{
			LoopFlashes += (Start >= Recipe.SustainStart && Start < Recipe.SustainEnd) ? 1 : 0;
		}
		if (LoopLength > KINDA_SMALL_NUMBER && LoopFlashes / LoopLength > MaxFlashesPerSecond)
		{
			Add(OutFindings, true, FText::Format(LOCTEXT("SustainFlashRate", "The sustain loop repeats {0} flashes every {1} s, more than {2} per second while held. Lengthen the loop or remove flashes from it."),
				FText::AsNumber(LoopFlashes), FText::AsNumber(LoopLength), FText::AsNumber(MaxFlashesPerSecond)), &Recipe);
		}
	}
}

void FFeelComfortAudit::AuditSettings(const UFeelSettings& Settings, TArray<FFeelComfortAuditFinding>& OutFindings)
{
	using namespace FeelComfortAuditPrivate;

	if (!Settings.DefaultComfortScales.bLimitFlashes)
	{
		Add(OutFindings, true, LOCTEXT("DefaultLimiterOff", "The flash limiter is off in the default comfort scales, so new players get no flash rate protection. Turn Limit Flashes on in Project Settings > Plugins > FeelKit > Default Comfort Scales."));
	}
	if (!Settings.ReducedFlashingPreset.bLimitFlashes || Settings.ReducedFlashingPreset.Flashes >= 1.0f)
	{
		Add(OutFindings, true, LOCTEXT("ReducedFlashingWeak", "The Reduced Flashing preset does not reduce flashes (its flash limiter is off or its Flashes scale is 1). Players who choose it expect fewer or weaker flashes."));
	}
	if (Settings.ReducedMotionPreset.CameraShake >= 1.0f && Settings.ReducedMotionPreset.CameraMotion >= 1.0f)
	{
		Add(OutFindings, true, LOCTEXT("ReducedMotionWeak", "The Reduced Motion preset keeps Camera Shake and Camera Motion at 1, so it does not reduce motion."));
	}
	if (!Settings.bApplyComfortToEngineCameraShakes)
	{
		Add(OutFindings, false, LOCTEXT("EngineShakesOff", "Comfort does not scale the engine's own camera shakes. Shakes the game plays without FeelKit ignore players' Camera Shake setting."));
	}
	if (!Settings.bApplyComfortToEngineForceFeedback)
	{
		Add(OutFindings, false, LOCTEXT("EngineForceFeedbackOff", "Comfort does not scale the engine's own force feedback. Vibration the game plays without FeelKit ignores players' Haptics setting."));
	}
}

void FFeelComfortAudit::RunProjectAudit()
{
	const UFeelSettings* Settings = GetDefault<UFeelSettings>();
	TArray<FFeelComfortAuditFinding> Findings;
	FFeelComfortAudit::AuditSettings(*Settings, Findings);

	TArray<FAssetData> Assets;
	IAssetRegistry::GetChecked().GetAssetsByClass(UFeelRecipe::StaticClass()->GetClassPathName(), Assets, true);

	FScopedSlowTask SlowTask(static_cast<float>(Assets.Num()), LOCTEXT("Auditing", "Auditing FeelKit recipes for comfort"));
	SlowTask.MakeDialogDelayed(0.5f);
	for (const FAssetData& Asset : Assets)
	{
		SlowTask.EnterProgressFrame(1.0f, FText::FromName(Asset.AssetName));
		if (const UFeelRecipe* Recipe = Cast<UFeelRecipe>(Asset.GetAsset()))
		{
			FFeelComfortAudit::AuditRecipe(*Recipe, *Settings, Findings);
		}
	}

	FMessageLog Log(FeelComfortAuditPrivate::LogName);
	Log.NewPage(FText::Format(LOCTEXT("AuditPage", "Comfort Audit {0}"), FText::AsDateTime(FDateTime::Now())));
	Log.Info(LOCTEXT("AuditDisclaimer", "FeelKit comfort audit: a readiness helper that catches likely comfort problems early. It is not a photosensitivity certification; test the finished game with an analysis tool."));

	int32 NumWarnings = 0;
	for (const FFeelComfortAuditFinding& Finding : Findings)
	{
		const TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(Finding.bWarning ? EMessageSeverity::Warning : EMessageSeverity::Info);
		if (const UFeelRecipe* Recipe = Finding.Recipe.Get())
		{
			Message->AddToken(FUObjectToken::Create(Recipe));
		}
		Message->AddToken(FTextToken::Create(Finding.Message));
		Log.AddMessage(Message);
		NumWarnings += Finding.bWarning ? 1 : 0;
	}

	Log.Info(FText::Format(LOCTEXT("AuditSummary", "Audited {0} recipes: {1} warnings, {2} notes."),
		FText::AsNumber(Assets.Num()), FText::AsNumber(NumWarnings), FText::AsNumber(Findings.Num() - NumWarnings)));
	Log.Open(EMessageSeverity::Info, true);
}

void FFeelComfortAudit::Startup()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
	FMessageLogInitializationOptions Options;
	Options.bShowPages = true;
	Options.bAllowClear = true;
	MessageLogModule.RegisterLogListing(FeelComfortAuditPrivate::LogName, LOCTEXT("AuditLogLabel", "FeelKit Comfort Audit"), Options);
}

void FFeelComfortAudit::Shutdown()
{
	if (FMessageLogModule* MessageLogModule = FModuleManager::GetModulePtr<FMessageLogModule>(TEXT("MessageLog")))
	{
		MessageLogModule->UnregisterLogListing(FeelComfortAuditPrivate::LogName);
	}
}

#undef LOCTEXT_NAMESPACE
