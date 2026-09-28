// Copyright 2026 Billo. All Rights Reserved.

#include "FeelAnimNotifies.h"

#include "Components/SkeletalMeshComponent.h"
#include "FeelPlaybackClock.h"
#include "FeelRecipe.h"
#include "FeelSubsystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelAnimNotifies)

void FFeelNotifyPlaySettings::MakePlay(USkeletalMeshComponent* MeshComp, FFeelTarget& OutTarget, FFeelPlayContext& OutContext) const
{
	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	OutTarget = Target == EFeelNotifyTarget::OwningActor && Owner ? FFeelTarget::FromActor(Owner) : FFeelTarget::FromComponent(MeshComp);
	OutContext.Parameters = Parameters;
	OutContext.ContextTags = ContextTags;
}

void UAnimNotify_PlayFeel::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UFeelSubsystem* Subsystem = MeshComp ? UFeelSubsystem::Get(MeshComp) : nullptr;
	if (!Subsystem || !Recipe)
	{
		return;
	}

	FFeelTarget Target;
	FFeelPlayContext Context;
	Settings.MakePlay(MeshComp, Target, Context);
	Subsystem->PlayFeel(Recipe, Target, Settings.Intensity, Context);
}

FString UAnimNotify_PlayFeel::GetNotifyName_Implementation() const
{
	return Recipe ? FString::Printf(TEXT("Feel: %s"), *Recipe->GetName()) : TEXT("Play Feel");
}

void UAnimNotify_SendFeelEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UFeelSubsystem* Subsystem = MeshComp ? UFeelSubsystem::Get(MeshComp) : nullptr;
	if (!Subsystem || !Event.IsValid())
	{
		return;
	}

	FFeelTarget Target;
	FFeelPlayContext Context;
	Settings.MakePlay(MeshComp, Target, Context);
	Subsystem->SendFeelEvent(Event, Target, Settings.Intensity, Context);
}

FString UAnimNotify_SendFeelEvent::GetNotifyName_Implementation() const
{
	return Event.IsValid() ? FString::Printf(TEXT("Feel Event: %s"), *Event.ToString()) : TEXT("Send Feel Event");
}

void UAnimNotify_SetFeelValue::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UFeelSubsystem* Subsystem = MeshComp ? UFeelSubsystem::Get(MeshComp) : nullptr;
	if (!Subsystem)
	{
		return;
	}

	const AActor* Actor = Scope == EFeelValueScope::OwningActor ? MeshComp->GetOwner() : nullptr;
	for (const TPair<FName, float>& Pair : Values)
	{
		if (!Pair.Key.IsNone())
		{
			if (Mode == EFeelValueMode::Add)
			{
				Subsystem->AddToAccumulator(Pair.Key, Pair.Value, Actor);
			}
			else
			{
				Subsystem->SetAccumulator(Pair.Key, Pair.Value, Actor);
			}
		}
	}
}

FString UAnimNotify_SetFeelValue::GetNotifyName_Implementation() const
{
	TArray<FString> Parts;
	for (const TPair<FName, float>& Pair : Values)
	{
		Parts.Add(Mode == EFeelValueMode::Add
			? FString::Printf(TEXT("%s %+.2f"), *Pair.Key.ToString(), Pair.Value)
			: FString::Printf(TEXT("%s %.2f"), *Pair.Key.ToString(), Pair.Value));
	}
	return Parts.Num() > 0 ? FString::Printf(TEXT("Feel Value: %s"), *FString::Join(Parts, TEXT(", "))) : TEXT("Set Feel Value");
}

void UAnimNotifyState_PlayFeel::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UFeelSubsystem* Subsystem = MeshComp ? UFeelSubsystem::Get(MeshComp) : nullptr;
	if (!Subsystem || !Recipe)
	{
		return;
	}

	FFeelTarget Target;
	FFeelPlayContext Context;
	Settings.MakePlay(MeshComp, Target, Context);
	const FFeelHandle Handle = Subsystem->PlayFeel(Recipe, Target, Settings.Intensity, Context);
	if (Handle.IsValid())
	{
		ActivePlays.Add(MeshComp, Handle);
	}
}

void UAnimNotifyState_PlayFeel::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	FFeelHandle Handle;
	if (!ActivePlays.RemoveAndCopyValue(MeshComp, Handle))
	{
		return;
	}

	// Clean up entries of meshes that no longer exist.
	for (auto It = ActivePlays.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}

	UFeelSubsystem* Subsystem = MeshComp ? UFeelSubsystem::Get(MeshComp) : nullptr;
	if (!Subsystem)
	{
		return;
	}

	if (Recipe && FFeelPlaybackClock::HasSustain(*Recipe))
	{
		Subsystem->ReleaseFeel(Handle);
	}
	else if (!bLetNonSustainedRecipeFinish)
	{
		Subsystem->StopFeel(Handle, true);
	}
}

FString UAnimNotifyState_PlayFeel::GetNotifyName_Implementation() const
{
	return Recipe ? FString::Printf(TEXT("Feel: %s"), *Recipe->GetName()) : TEXT("Play Feel (Window)");
}
