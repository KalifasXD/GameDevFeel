// Copyright 2026 Billo. All Rights Reserved.

#include "FeelActorDelivery.h"

#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "FeelFrameOutput.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/Package.h"

void FFeelActorDelivery::GetMotionComponents(USceneComponent* Component, bool bWholeActor, TArray<USceneComponent*, TInlineAllocator<4>>& OutComponents)
{
	OutComponents.Reset();
	if (!Component)
	{
		return;
	}

	const UPrimitiveComponent* CollisionRoot = bWholeActor ? Cast<UPrimitiveComponent>(Component) : nullptr;
	if (CollisionRoot && CollisionRoot->IsCollisionEnabled())
	{
		for (USceneComponent* Child : Component->GetAttachChildren())
		{
			if (Cast<UPrimitiveComponent>(Child) && !Child->IsEditorOnly())
			{
				OutComponents.Add(Child);
			}
		}
	}

	if (OutComponents.Num() == 0)
	{
		OutComponents.Add(Component);
	}
}

const FName FFeelActorDelivery::OverlayFlashColorParameter(TEXT("FlashColor"));
const FName FFeelActorDelivery::OverlayFlashAmountParameter(TEXT("FlashAmount"));

void FFeelActorDelivery::AddOverlayFlash(UMeshComponent* Component, UMaterialInterface* Material, const FLinearColor& Color, float Amount)
{
	if (!Component || !Material || Amount <= 0.0f)
	{
		return;
	}

	FOverlayFlashEntry* Entry = OverlayFlashes.Find(FObjectKey(Component));
	if (!Entry)
	{
		Entry = &OverlayFlashes.Add(FObjectKey(Component));
		Entry->Component = Component;
		Entry->OriginalOverlay = Component->GetOverlayMaterial();
	}
	if (!Entry->bRequested || Amount > Entry->Amount)
	{
		Entry->Source = Material;
		Entry->Color = Color;
		Entry->Amount = FMath::Clamp(Amount, 0.0f, 1.0f);
	}
	Entry->bRequested = true;
}

void FFeelActorDelivery::RestoreOverlay(FOverlayFlashEntry& Entry)
{
	UMeshComponent* Component = Entry.Component.Get();
	if (Component && Entry.Instance.IsValid() && Component->GetOverlayMaterial() == Entry.Instance.Get())
	{
		Component->SetOverlayMaterial(Entry.OriginalOverlay.Get());
	}
}

void FFeelActorDelivery::BeginFrame()
{
	for (TPair<FObjectKey, FOverlayFlashEntry>& Pair : OverlayFlashes)
	{
		Pair.Value.Amount = 0.0f;
		Pair.Value.bRequested = false;
	}

	for (TPair<FObjectKey, FScaleEntry>& Pair : ScaledComponents)
	{
		Pair.Value.Delta = FVector::ZeroVector;
		Pair.Value.bRequested = false;
	}

	for (TPair<FMaterialKey, FMaterialEntry>& Pair : MaterialParameters)
	{
		Pair.Value.Parameter.ScalarOffset = 0.0f;
		Pair.Value.Parameter.ColorWeight = 0.0f;
		Pair.Value.bRequested = false;
	}

	for (TPair<FObjectKey, FTransformEntry>& Pair : Transforms)
	{
		Pair.Value.RequestedLocation = FVector::ZeroVector;
		Pair.Value.RequestedRotation = FQuat::Identity;
		Pair.Value.bRequested = false;
	}

	for (TPair<FObjectKey, FLightEntry>& Pair : Lights)
	{
		Pair.Value.IntensityDelta = 0.0f;
		Pair.Value.ColorWeight = 0.0f;
		Pair.Value.bRequested = false;
	}
}

void FFeelActorDelivery::AddTransform(USceneComponent* Component, const FVector& LocationOffset, const FRotator& RotationOffset)
{
	if (!Component || (LocationOffset.IsZero() && RotationOffset.IsZero()))
	{
		return;
	}

	const FObjectKey Key(Component);
	FTransformEntry* Entry = Transforms.Find(Key);
	if (!Entry)
	{
		Entry = &Transforms.Add(Key);
		Entry->Component = Component;
	}
	Entry->RequestedLocation += LocationOffset;
	Entry->RequestedRotation = Entry->RequestedRotation * RotationOffset.Quaternion();
	Entry->bRequested = true;
}

void FFeelActorDelivery::AddLight(USceneComponent* Component, float IntensityDelta, const FLinearColor& Color, float ColorWeight)
{
	if (!Component || (FMath::IsNearlyZero(IntensityDelta) && ColorWeight <= 0.0f))
	{
		return;
	}

	TArray<ULightComponent*, TInlineAllocator<4>> TargetLights;
	if (ULightComponent* Light = Cast<ULightComponent>(Component))
	{
		TargetLights.Add(Light);
	}
	else if (AActor* Owner = Component->GetOwner())
	{
		TInlineComponentArray<ULightComponent*> OwnerLights(Owner);
		TargetLights.Append(OwnerLights);
	}

	for (ULightComponent* Light : TargetLights)
	{
		const FObjectKey Key(Light);
		FLightEntry* Entry = Lights.Find(Key);
		if (!Entry)
		{
			Entry = &Lights.Add(Key);
			Entry->Light = Light;
			Entry->BaseIntensity = Light->Intensity;
			Entry->BaseColor = Light->GetLightColor();
		}
		Entry->IntensityDelta += IntensityDelta;
		const float ClampedWeight = FMath::Clamp(ColorWeight, 0.0f, 1.0f);
		if (ClampedWeight > Entry->ColorWeight)
		{
			Entry->ColorWeight = ClampedWeight;
			Entry->Color = Color;
		}
		Entry->bRequested = true;
	}
}

bool FFeelActorDelivery::IsOwnedByPhysics(const USceneComponent& Component)
{
	// A body that physics simulates (a ragdoll, a physics prop) is moved by the physics scene; moving it from here would
	// fight the simulation and Unreal warns about it in Play In Editor.
	const UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(&Component);
	return Primitive && Primitive->IsSimulatingPhysics();
}

void FFeelActorDelivery::ApplyTransform(USceneComponent& Component, FTransformEntry& Entry, bool bRestore)
{
	// Remove last frame's offset from wherever the component is now, then apply this frame's.
	const FQuat BaseRotation = Component.GetRelativeRotation().Quaternion() * Entry.AppliedRotation.Inverse();
	const FVector BaseLocation = Component.GetRelativeLocation() - Entry.AppliedLocation;

	Entry.AppliedRotation = bRestore ? FQuat::Identity : Entry.RequestedRotation;
	Entry.AppliedLocation = bRestore ? FVector::ZeroVector : BaseRotation.RotateVector(Entry.RequestedLocation);
	Component.SetRelativeLocationAndRotation(BaseLocation + Entry.AppliedLocation, BaseRotation * Entry.AppliedRotation);
}

void FFeelActorDelivery::AddScale(USceneComponent* Component, const FVector& ScaleDelta)
{
	if (!Component || ScaleDelta.IsZero())
	{
		return;
	}

	const FObjectKey Key(Component);
	FScaleEntry* Entry = ScaledComponents.Find(Key);
	if (!Entry)
	{
		Entry = &ScaledComponents.Add(Key);
		Entry->Component = Component;
		Entry->BaseScale = Component->GetRelativeScale3D();
	}
	Entry->Delta += ScaleDelta;
	Entry->bRequested = true;
}

void FFeelActorDelivery::AddMaterialParameters(UPrimitiveComponent* Component, TConstArrayView<FFeelMaterialParameter> Parameters)
{
	if (!Component)
	{
		return;
	}

	for (const FFeelMaterialParameter& Parameter : Parameters)
	{
		if (Parameter.Name.IsNone())
		{
			continue;
		}

		const FMaterialKey Key = MakeMaterialKey(Component, Parameter);
		FMaterialEntry* Entry = MaterialParameters.Find(Key);
		if (!Entry)
		{
			Entry = &MaterialParameters.Add(Key);
			Entry->Component = Component;
			Entry->Parameter = Parameter;
			Entry->Parameter.ScalarOffset = 0.0f;
			Entry->Parameter.ColorWeight = 0.0f;
			CaptureBase(*Component, *Entry);
		}
		FFeelFrameOutput::MergeMaterialParameter(Entry->Parameter, Parameter);
		Entry->bRequested = true;
	}
}

void FFeelActorDelivery::EndFrame()
{
	for (auto It = OverlayFlashes.CreateIterator(); It; ++It)
	{
		FOverlayFlashEntry& Entry = It.Value();
		UMeshComponent* Component = Entry.Component.Get();
		if (!Component)
		{
			It.RemoveCurrent();
			continue;
		}
		if (!Entry.bRequested || Entry.Amount <= 0.0f || !Entry.Source.IsValid())
		{
			RestoreOverlay(Entry);
			It.RemoveCurrent();
			continue;
		}
		if (!Entry.Instance.IsValid() || Entry.Instance->Parent != Entry.Source.Get())
		{
			Entry.Instance.Reset(UMaterialInstanceDynamic::Create(Entry.Source.Get(), GetTransientPackage()));
		}
		Entry.Instance->SetVectorParameterValue(OverlayFlashColorParameter, Entry.Color);
		Entry.Instance->SetScalarParameterValue(OverlayFlashAmountParameter, Entry.Amount);
		if (Component->GetOverlayMaterial() != Entry.Instance.Get())
		{
			Component->SetOverlayMaterial(Entry.Instance.Get());
		}
	}

	for (auto It = ScaledComponents.CreateIterator(); It; ++It)
	{
		const FScaleEntry& Entry = It.Value();
		USceneComponent* Component = Entry.Component.Get();
		if (!Component)
		{
			It.RemoveCurrent();
			continue;
		}

		if (!Entry.bRequested)
		{
			Component->SetRelativeScale3D(Entry.BaseScale);
			It.RemoveCurrent();
			continue;
		}
		Component->SetRelativeScale3D(Entry.BaseScale * (FVector::OneVector + Entry.Delta));
	}

	for (auto It = MaterialParameters.CreateIterator(); It; ++It)
	{
		const FMaterialEntry& Entry = It.Value();
		UPrimitiveComponent* Component = Entry.Component.Get();
		if (!Component)
		{
			It.RemoveCurrent();
			continue;
		}

		const bool bRestore = !Entry.bRequested;
		ApplyMaterial(*Component, Entry, bRestore);
		if (bRestore)
		{
			It.RemoveCurrent();
		}
	}

	for (auto It = Transforms.CreateIterator(); It; ++It)
	{
		FTransformEntry& Entry = It.Value();
		USceneComponent* Component = Entry.Component.Get();
		if (!Component || IsOwnedByPhysics(*Component))
		{
			// Physics took over (for example a character turned into a ragdoll): its motion replaces the offset.
			It.RemoveCurrent();
			continue;
		}

		const bool bRestore = !Entry.bRequested;
		ApplyTransform(*Component, Entry, bRestore);
		if (bRestore)
		{
			It.RemoveCurrent();
		}
	}

	for (auto It = Lights.CreateIterator(); It; ++It)
	{
		const FLightEntry& Entry = It.Value();
		ULightComponent* Light = Entry.Light.Get();
		if (!Light)
		{
			It.RemoveCurrent();
			continue;
		}

		if (!Entry.bRequested)
		{
			Light->SetIntensity(Entry.BaseIntensity);
			Light->SetLightColor(Entry.BaseColor);
			It.RemoveCurrent();
			continue;
		}
		Light->SetIntensity(FMath::Max(Entry.BaseIntensity * (1.0f + Entry.IntensityDelta), 0.0f));
		Light->SetLightColor(FMath::Lerp(Entry.BaseColor, Entry.Color, Entry.ColorWeight));
	}
}

void FFeelActorDelivery::RestoreAll()
{
	for (TPair<FObjectKey, FOverlayFlashEntry>& Pair : OverlayFlashes)
	{
		RestoreOverlay(Pair.Value);
	}
	OverlayFlashes.Reset();

	for (const TPair<FObjectKey, FScaleEntry>& Pair : ScaledComponents)
	{
		if (USceneComponent* Component = Pair.Value.Component.Get())
		{
			Component->SetRelativeScale3D(Pair.Value.BaseScale);
		}
	}

	for (const TPair<FMaterialKey, FMaterialEntry>& Pair : MaterialParameters)
	{
		if (UPrimitiveComponent* Component = Pair.Value.Component.Get())
		{
			ApplyMaterial(*Component, Pair.Value, true);
		}
	}

	for (TPair<FObjectKey, FTransformEntry>& Pair : Transforms)
	{
		USceneComponent* Component = Pair.Value.Component.Get();
		if (Component && !IsOwnedByPhysics(*Component))
		{
			ApplyTransform(*Component, Pair.Value, true);
		}
	}

	for (const TPair<FObjectKey, FLightEntry>& Pair : Lights)
	{
		if (ULightComponent* Light = Pair.Value.Light.Get())
		{
			Light->SetIntensity(Pair.Value.BaseIntensity);
			Light->SetLightColor(Pair.Value.BaseColor);
		}
	}

	ScaledComponents.Reset();
	MaterialParameters.Reset();
	Transforms.Reset();
	Lights.Reset();
}

FFeelActorDelivery::FMaterialKey FFeelActorDelivery::MakeMaterialKey(const UPrimitiveComponent* Component, const FFeelMaterialParameter& Parameter)
{
	const uint8 Kind = static_cast<uint8>(static_cast<uint8>(Parameter.Route) * 2 + (Parameter.bIsColor ? 1 : 0));
	return FMaterialKey(FObjectKey(Component), Parameter.Name, Kind);
}

void FFeelActorDelivery::CaptureBase(const UPrimitiveComponent& Component, FMaterialEntry& Entry)
{
	const FFeelMaterialParameter& Parameter = Entry.Parameter;
	Entry.bHasBase = false;

	if (Parameter.Route == EFeelMaterialParameterRoute::CustomPrimitiveData)
	{
		const TArray<float>& Data = Component.GetCustomPrimitiveData().Data;
		auto ReadData = [&Data](int32 Index) { return Data.IsValidIndex(Index) ? Data[Index] : 0.0f; };

		const int32 DataIndex = Parameter.bIsColor
			? Component.GetCustomPrimitiveDataIndexForVectorParameter(Parameter.Name)
			: Component.GetCustomPrimitiveDataIndexForScalarParameter(Parameter.Name);
		if (DataIndex == INDEX_NONE)
		{
			return;
		}

		if (Parameter.bIsColor)
		{
			Entry.BaseColor = FLinearColor(ReadData(DataIndex), ReadData(DataIndex + 1), ReadData(DataIndex + 2), ReadData(DataIndex + 3));
		}
		else
		{
			Entry.BaseScalar = ReadData(DataIndex);
		}
		Entry.bHasBase = true;
		return;
	}

	const FHashedMaterialParameterInfo ParameterInfo(Parameter.Name);
	for (int32 MaterialIndex = 0; MaterialIndex < Component.GetNumMaterials(); ++MaterialIndex)
	{
		const UMaterialInterface* Material = Component.GetMaterial(MaterialIndex);
		if (!Material)
		{
			continue;
		}

		const bool bFound = Parameter.bIsColor
			? Material->GetVectorParameterValue(ParameterInfo, Entry.BaseColor)
			: Material->GetScalarParameterValue(ParameterInfo, Entry.BaseScalar);
		if (bFound)
		{
			Entry.bHasBase = true;
			return;
		}
	}
}

void FFeelActorDelivery::ApplyMaterial(UPrimitiveComponent& Component, const FMaterialEntry& Entry, bool bRestore)
{
	if (!Entry.bHasBase)
	{
		return;
	}

	const FFeelMaterialParameter& Parameter = Entry.Parameter;
	const bool bCustomPrimitiveData = Parameter.Route == EFeelMaterialParameterRoute::CustomPrimitiveData;
	UMeshComponent* Mesh = Cast<UMeshComponent>(&Component);

	if (Parameter.bIsColor)
	{
		const float Weight = bRestore ? 0.0f : FMath::Clamp(Parameter.ColorWeight, 0.0f, 1.0f);
		const FLinearColor Color = Entry.BaseColor + (Parameter.Color - Entry.BaseColor) * Weight;
		if (bCustomPrimitiveData)
		{
			Component.SetVectorParameterForCustomPrimitiveData(Parameter.Name, FVector4(Color.R, Color.G, Color.B, Color.A));
		}
		else if (Mesh)
		{
			Mesh->SetVectorParameterValueOnMaterials(Parameter.Name, FVector(Color.R, Color.G, Color.B));
		}
		return;
	}

	const float Scalar = bRestore ? Entry.BaseScalar : Entry.BaseScalar + Parameter.ScalarOffset;
	if (bCustomPrimitiveData)
	{
		Component.SetScalarParameterForCustomPrimitiveData(Parameter.Name, Scalar);
	}
	else if (Mesh)
	{
		Mesh->SetScalarParameterValueOnMaterials(Parameter.Name, Scalar);
	}
}
