// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelOutputSink.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ObjectKey.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/WeakObjectPtr.h"

class UMaterialInterface;
class UMeshComponent;
class UPrimitiveComponent;
class USceneComponent;

/**
 * Applies actor contributions (scale, material parameters) and restores the original values once nothing requests
 * them. Shared by the runtime subsystem and the editor preview, so actor effects look identical in both.
 * Each frame: BeginFrame, then Add calls for every instance, then EndFrame.
 */
class FEELCORE_API FFeelActorDelivery
{
public:
	/**
	 * The components that actor motion (scale, location and rotation offsets) applies to. A scene component target is used
	 * as it is. For a whole actor whose root takes part in collision and has visible child components (a character's capsule
	 * with its mesh), the effect goes to those children: changing the root would change the actor's collision and movement,
	 * which in a networked game also replicates to other machines. Any other actor uses its root.
	 */
	static void GetMotionComponents(USceneComponent* Component, bool bWholeActor, TArray<USceneComponent*, TInlineAllocator<4>>& OutComponents);

	/** Clears this frame's requests. */
	void BeginFrame();

	/** Requests a relative scale change. Deltas for the same component add. */
	void AddScale(USceneComponent* Component, const FVector& ScaleDelta);

	/** Requests material parameter changes. Scalars add; colors keep the strongest weight. */
	void AddMaterialParameters(UPrimitiveComponent* Component, TConstArrayView<FFeelMaterialParameter> Parameters);

	/**
	 * Requests a location and rotation offset relative to the component's own orientation. Applied as a change on top of
	 * wherever the component currently is, so components that move on their own (such as a character) keep moving.
	 * A component whose body physics simulates (a ragdoll, a physics prop) is left to physics and its offset is dropped.
	 */
	void AddTransform(USceneComponent* Component, const FVector& LocationOffset, const FRotator& RotationOffset);

	/** True when the physics scene simulates this component's body, so FeelKit leaves its position and rotation alone. */
	static bool IsOwnedByPhysics(const USceneComponent& Component);

	/**
	 * Requests a light change on Component when it is a light, or else on every light component of its owner:
	 * intensity multiplied by (1 + IntensityDelta), color blended toward Color by ColorWeight.
	 */
	void AddLight(USceneComponent* Component, float IntensityDelta, const FLinearColor& Color, float ColorWeight);

	/** Parameters an overlay flash material reads. */
	static const FName OverlayFlashColorParameter;
	static const FName OverlayFlashAmountParameter;

	/**
	 * Requests an overlay flash on a mesh: an instance of Material goes into the mesh's overlay material slot with its
	 * FlashColor and FlashAmount parameters set. The mesh's own overlay material is put back once nothing requests it.
	 * Requests for the same mesh keep the strongest amount.
	 */
	void AddOverlayFlash(UMeshComponent* Component, UMaterialInterface* Material, const FLinearColor& Color, float Amount);

	/** Applies this frame's requests and restores everything no longer requested. */
	void EndFrame();

	/** Restores every modified component immediately and forgets all state. */
	void RestoreAll();

	/** True while at least one component is modified. */
	bool IsModifyingAny() const { return ScaledComponents.Num() > 0 || MaterialParameters.Num() > 0 || Transforms.Num() > 0 || Lights.Num() > 0 || OverlayFlashes.Num() > 0; }

private:
	struct FTransformEntry
	{
		TWeakObjectPtr<USceneComponent> Component;
		FVector RequestedLocation = FVector::ZeroVector;
		FQuat RequestedRotation = FQuat::Identity;
		FVector AppliedLocation = FVector::ZeroVector;
		FQuat AppliedRotation = FQuat::Identity;
		bool bRequested = false;
	};

	struct FLightEntry
	{
		TWeakObjectPtr<class ULightComponent> Light;
		float BaseIntensity = 0.0f;
		FLinearColor BaseColor = FLinearColor::White;
		float IntensityDelta = 0.0f;
		FLinearColor Color = FLinearColor::White;
		float ColorWeight = 0.0f;
		bool bRequested = false;
	};

	static void ApplyTransform(USceneComponent& Component, FTransformEntry& Entry, bool bRestore);

	struct FScaleEntry
	{
		TWeakObjectPtr<USceneComponent> Component;
		FVector BaseScale = FVector::OneVector;
		FVector Delta = FVector::ZeroVector;
		bool bRequested = false;
	};

	struct FMaterialEntry
	{
		TWeakObjectPtr<UPrimitiveComponent> Component;
		FFeelMaterialParameter Parameter;
		float BaseScalar = 0.0f;
		FLinearColor BaseColor = FLinearColor::Black;
		bool bHasBase = false;
		bool bRequested = false;
	};

	/** Component, parameter name, and route and kind packed together. */
	using FMaterialKey = TTuple<FObjectKey, FName, uint8>;

	static FMaterialKey MakeMaterialKey(const UPrimitiveComponent* Component, const FFeelMaterialParameter& Parameter);
	static void CaptureBase(const UPrimitiveComponent& Component, FMaterialEntry& Entry);
	static void ApplyMaterial(UPrimitiveComponent& Component, const FMaterialEntry& Entry, bool bRestore);

	TMap<FObjectKey, FScaleEntry> ScaledComponents;
	TMap<FMaterialKey, FMaterialEntry> MaterialParameters;
	TMap<FObjectKey, FTransformEntry> Transforms;
	TMap<FObjectKey, FLightEntry> Lights;

	struct FOverlayFlashEntry
	{
		TWeakObjectPtr<UMeshComponent> Component;
		/** The mesh's own overlay material, put back afterwards. */
		TWeakObjectPtr<UMaterialInterface> OriginalOverlay;
		TWeakObjectPtr<UMaterialInterface> Source;
		TStrongObjectPtr<UMaterialInstanceDynamic> Instance;
		FLinearColor Color = FLinearColor::White;
		float Amount = 0.0f;
		bool bRequested = false;
	};

	static void RestoreOverlay(FOverlayFlashEntry& Entry);

	TMap<FObjectKey, FOverlayFlashEntry> OverlayFlashes;
};
