// Copyright 2026 Billo. All Rights Reserved.

#include "Steps/FeelStep_Spawn.h"

#include "Components/DecalComponent.h"
#include "Components/SceneComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Math/RandomStream.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelStep_Spawn)

namespace FeelStepSpawnPrivate
{
	FVector ResolveLocation(EFeelSpawnLocation Source, const FFeelContext& Context)
	{
		if (Source == EFeelSpawnLocation::PlayLocationOrTarget && !Context.PlayContext.Location.IsZero())
		{
			return Context.PlayContext.Location;
		}
		return Context.TargetLocation;
	}

	/** Deterministic per play and track. */
	int32 MakeSeed(const FFeelContext& Context)
	{
		return static_cast<int32>(HashCombineFast(GetTypeHash(Context.InstanceId), GetTypeHash(Context.TrackIndex)));
	}
}

void UFeelStep_NumberPop::OnStart_Implementation(const FFeelContext& Context)
{
	UFeelSubsystem* Subsystem = Context.World ? Context.World->GetSubsystem<UFeelSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	FText Shown = Text;
	if (!ValueParameter.IsNone())
	{
		const float* Value = Context.PlayContext.Parameters.Find(ValueParameter);
		FNumberFormattingOptions Formatting;
		Formatting.MinimumFractionalDigits = Decimals;
		Formatting.MaximumFractionalDigits = Decimals;
		Shown = FText::AsNumber(Value ? *Value : 0.0f, &Formatting);
	}

	const FRandomStream Stream(FeelStepSpawnPrivate::MakeSeed(Context));

	FFeelNumberPop Pop;
	Pop.Text = FText::FromString(Prefix + Shown.ToString() + Suffix);
	Pop.WorldLocation = FeelStepSpawnPrivate::ResolveLocation(Location, Context) + WorldOffset;
	Pop.Color = Color;
	Pop.FontSize = FontSize * (bScaleWithIntensity ? FMath::Clamp(Context.Intensity, 0.25f, 4.0f) : 1.0f);
	Pop.Lifetime = Context.TrackDuration > 0.0f ? Context.TrackDuration : DefaultLifetime;
	Pop.RiseDistance = RiseDistance;
	Pop.ScreenOffset = FVector2D(Stream.FRandRange(-Spread, Spread), 0.0);
	Pop.PopScale = PopScale;
	Pop.PlayerController = Context.PlayerController;
	Subsystem->AddNumberPop(Pop);
}

FGameplayTag UFeelStep_NumberPop::GetDefaultChannel_Implementation() const
{
	return FeelTags::UI;
}

void UFeelStep_SpawnDecal::OnStart_Implementation(const FFeelContext& Context)
{
	if (!DecalMaterial || !Context.World)
	{
		return;
	}

	// Decals project along their X axis, so X points into the surface.
	FVector Normal = Context.PlayContext.Normal.IsNearlyZero() ? FVector::UpVector : Context.PlayContext.Normal.GetSafeNormal();
	FVector SpawnLocation = FeelStepSpawnPrivate::ResolveLocation(Location, Context);

	if (bFindSurface && SurfaceSearchDistance > 0.0f)
	{
		FCollisionQueryParams Query(SCENE_QUERY_STAT(FeelSpawnDecal), false);
		if (Context.Target)
		{
			Query.AddIgnoredActor(Context.Target);
		}
		if (Context.Instigator)
		{
			Query.AddIgnoredActor(Context.Instigator);
		}
		// Measured from the target's outer edge along the projection, so a character's capsule (whose location is its
		// middle) does not use up the search distance.
		float TargetDepth = 0.0f;
		if (Context.TargetComponent && FeelStepSpawnPrivate::ResolveLocation(Location, Context).Equals(Context.TargetLocation))
		{
			const FVector Extent = Context.TargetComponent->Bounds.BoxExtent;
			TargetDepth = static_cast<float>(FMath::Abs(Extent.X * Normal.X) + FMath::Abs(Extent.Y * Normal.Y) + FMath::Abs(Extent.Z * Normal.Z));
		}
		FHitResult Hit;
		const FVector TraceStart = SpawnLocation + Normal * 10.0f;
		const FVector TraceEnd = SpawnLocation - Normal * (SurfaceSearchDistance + TargetDepth);
		if (Context.World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Query))
		{
			SpawnLocation = Hit.ImpactPoint;
			Normal = Hit.ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER, Normal);
		}
	}

	FRotator Rotation = (-Normal).Rotation();
	if (bRandomRotation)
	{
		const FRandomStream Stream(FeelStepSpawnPrivate::MakeSeed(Context));
		Rotation.Roll = Stream.FRandRange(0.0f, 360.0f);
	}

	if (UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(Context.World, DecalMaterial, DecalSize, SpawnLocation, Rotation, Lifetime + FadeOutTime))
	{
		if (FadeOutTime > 0.0f)
		{
			// Never destroy the owner: decals spawned at a location belong to the world settings actor.
			Decal->SetFadeOut(Lifetime, FadeOutTime, false);
		}
	}
}

FGameplayTag UFeelStep_SpawnDecal::GetDefaultChannel_Implementation() const
{
	return FeelTags::Spawn;
}

#if WITH_EDITOR
void UFeelStep_SpawnDecal::ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const
{
	if (!DecalMaterial)
	{
		OutErrors.Add(NSLOCTEXT("FeelKit", "SpawnDecalMissingMaterial", "Spawn Decal has no decal material."));
	}
}
#endif
