// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FeelActorDelivery.h"
#include "FeelArbiters.h"
#include "FeelFrameOutput.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Steps/FeelStep_ForceFeedbackCurve.h"
#include "Steps/FeelStep_MaterialPulse.h"
#include "UObject/Package.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelMaterialHapticsTests
{
	FFeelStepEvalContext MakeContext(float LocalTime, float Duration, float Intensity = 1.0f)
	{
		FFeelStepEvalContext Context;
		Context.LocalTime = LocalTime;
		Context.Duration = Duration;
		Context.Alpha = Duration > 0.0f ? FMath::Clamp(LocalTime / Duration, 0.0f, 1.0f) : 0.0f;
		Context.Intensity = Intensity;
		return Context;
	}

	FFeelFrameOutput EvaluateStep(const UFeelStep* Step, const FFeelStepEvalContext& Context)
	{
		FFeelOutputAccumulator Accumulator;
		Step->Evaluate(Context, Accumulator);
		return Accumulator.Output;
	}

	/** A minimal game world, destroyed at the end of the scope. */
	class FScopedWorld
	{
	public:
		FScopedWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UWorld* World = nullptr;
	};

	bool ReadColor(const UPrimitiveComponent* Component, FName Name, FLinearColor& OutColor)
	{
		const UMaterialInterface* Material = Component ? Component->GetMaterial(0) : nullptr;
		return Material && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(Name), OutColor);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMaterialPulseStepTest, "FeelKit.Steps.MaterialPulse", FEEL_TEST_FLAGS)
bool FFeelMaterialPulseStepTest::RunTest(const FString& Parameters)
{
	using namespace FeelMaterialHapticsTests;

	UFeelStep_MaterialPulse* Pulse = NewObject<UFeelStep_MaterialPulse>(GetTransientPackage());
	if (!TestEqual(TEXT("Pulses one color parameter by default"), Pulse->Parameters.Num(), 1))
	{
		return false;
	}

	FFeelFrameOutput Output = EvaluateStep(Pulse, MakeContext(0.5f, 1.0f));
	if (!TestEqual(TEXT("One material contribution"), Output.MaterialParameters.Num(), 1))
	{
		return false;
	}
	TestEqual(TEXT("Default parameter name"), Output.MaterialParameters[0].Name, FName(TEXT("Color")));
	TestTrue(TEXT("Color contribution"), Output.MaterialParameters[0].bIsColor);
	TestEqual(TEXT("Color fully blended at the peak"), Output.MaterialParameters[0].ColorWeight, 1.0f, 0.001f);
	TestEqual(TEXT("Intensity scales the color blend"), EvaluateStep(Pulse, MakeContext(0.5f, 1.0f, 0.4f)).MaterialParameters[0].ColorWeight, 0.4f, 0.001f);

	Pulse->Parameters[0].bIsColor = false;
	Pulse->Parameters[0].ParameterName = TEXT("Glow");
	Pulse->Parameters[0].ScalarAmount = 2.0f;
	Pulse->Route = EFeelMaterialPulseRoute::CustomPrimitiveData;
	Output = EvaluateStep(Pulse, MakeContext(0.5f, 1.0f));
	TestEqual(TEXT("Scalar offset at the peak"), Output.MaterialParameters[0].ScalarOffset, 2.0f, 0.001f);
	TestTrue(TEXT("Route reaches the sink"), Output.MaterialParameters[0].Route == EFeelMaterialParameterRoute::CustomPrimitiveData);

	FFeelOutputAccumulator Accumulator;
	FFeelMaterialParameter Scalar;
	Scalar.Name = TEXT("Glow");
	Scalar.ScalarOffset = 1.0f;
	Accumulator.AddMaterialParameter(Scalar);
	Accumulator.AddMaterialParameter(Scalar);
	FFeelMaterialParameter WeakRed;
	WeakRed.Name = TEXT("Tint");
	WeakRed.bIsColor = true;
	WeakRed.Color = FLinearColor::Red;
	WeakRed.ColorWeight = 0.3f;
	FFeelMaterialParameter StrongBlue = WeakRed;
	StrongBlue.Color = FLinearColor::Blue;
	StrongBlue.ColorWeight = 0.9f;
	Accumulator.AddMaterialParameter(WeakRed);
	Accumulator.AddMaterialParameter(StrongBlue);
	TestEqual(TEXT("One entry per parameter"), Accumulator.Output.MaterialParameters.Num(), 2);
	TestEqual(TEXT("Scalars with the same name add"), Accumulator.Output.MaterialParameters[0].ScalarOffset, 2.0f);
	TestEqual(TEXT("Strongest color wins"), Accumulator.Output.MaterialParameters[1].Color, FLinearColor::Blue);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelForceFeedbackStepTest, "FeelKit.Steps.ForceFeedbackCurve", FEEL_TEST_FLAGS)
bool FFeelForceFeedbackStepTest::RunTest(const FString& Parameters)
{
	using namespace FeelMaterialHapticsTests;

	UFeelStep_ForceFeedbackCurve* Rumble = NewObject<UFeelStep_ForceFeedbackCurve>(GetTransientPackage());
	FFeelForceFeedbackValues Values = EvaluateStep(Rumble, MakeContext(0.15f, 1.0f)).ForceFeedback;
	TestEqual(TEXT("Left large at the kick peak"), Values.LeftLarge, 1.0f, 0.001f);
	TestEqual(TEXT("Left small at the kick peak"), Values.LeftSmall, 0.4f, 0.001f);
	TestEqual(TEXT("Right large at the kick peak"), Values.RightLarge, 1.0f, 0.001f);
	TestTrue(TEXT("Silent at the end"), EvaluateStep(Rumble, MakeContext(1.0f, 1.0f)).ForceFeedback.IsZero());

	Rumble->Shape = EFeelMotionShape::Smooth;
	Rumble->RippleFrequency = 10.0f;
	Rumble->RippleDepth = 0.5f;
	TestEqual(TEXT("Ripple top keeps full strength"), EvaluateStep(Rumble, MakeContext(0.5f, 1.0f)).ForceFeedback.LeftLarge, 1.0f, 0.001f);
	TestEqual(TEXT("Ripple dip lowers strength by its depth"), EvaluateStep(Rumble, MakeContext(0.45f, 1.0f)).ForceFeedback.LeftLarge, FMath::Sin(0.45f * UE_PI) * 0.5f, 0.001f);

	FFeelFrameOutput A;
	A.ForceFeedback.LeftLarge = 0.8f;
	A.ForceFeedback.RightSmall = 0.1f;
	FFeelFrameOutput B;
	B.ForceFeedback.LeftLarge = 0.3f;
	B.ForceFeedback.RightSmall = 0.6f;
	const TArray<const FFeelFrameOutput*> Inputs = { &A, &B };
	FFeelFrameOutput Player;
	FeelArbiters::ResolveForceFeedback(Inputs, Player);
	TestEqual(TEXT("Strongest left large motor wins"), Player.ForceFeedback.LeftLarge, 0.8f);
	TestEqual(TEXT("Strongest right small motor wins"), Player.ForceFeedback.RightSmall, 0.6f);

	TestFalse(TEXT("Force feedback cannot preview"), Rumble->SupportsPreview());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelActorDeliveryTest, "FeelKit.Runtime.ActorDelivery", FEEL_TEST_FLAGS)
bool FFeelActorDeliveryTest::RunTest(const FString& Parameters)
{
	using namespace FeelMaterialHapticsTests;

	FScopedWorld TestWorld;
	AActor* Actor = TestWorld.World->SpawnActor<AActor>();
	UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Actor, TEXT("Mesh"));
	Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
	// The cube's default WorldGridMaterial has no parameters; BasicShapeMaterial exposes "Color".
	Mesh->SetMaterial(0, LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")));
	Actor->SetRootComponent(Mesh);
	Mesh->RegisterComponent();
	Mesh->SetRelativeScale3D(FVector(2.0));

	const FName ColorName(TEXT("Color"));
	FLinearColor BaseColor;
	if (!TestTrue(TEXT("BasicShapeMaterial has a Color parameter"), ReadColor(Mesh, ColorName, BaseColor)))
	{
		return false;
	}

	FFeelMaterialParameter Tint;
	Tint.Name = ColorName;
	Tint.bIsColor = true;
	Tint.Color = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	Tint.ColorWeight = 0.5f;

	FFeelActorDelivery Delivery;
	Delivery.BeginFrame();
	Delivery.AddScale(Mesh, FVector(0.25));
	Delivery.AddScale(Mesh, FVector(0.25));
	Delivery.AddMaterialParameters(Mesh, MakeArrayView(&Tint, 1));
	Delivery.EndFrame();

	TestTrue(TEXT("Scale deltas add on top of the base scale"), Mesh->GetRelativeScale3D().Equals(FVector(3.0), 0.0001));
	FLinearColor Pulsed;
	ReadColor(Mesh, ColorName, Pulsed);
	const FLinearColor Expected = BaseColor + (Tint.Color - BaseColor) * 0.5f;
	TestTrue(TEXT("Color blends halfway toward the pulse color"), FMath::IsNearlyEqual(Pulsed.R, Expected.R, 0.001f) && FMath::IsNearlyEqual(Pulsed.G, Expected.G, 0.001f));
	TestTrue(TEXT("Delivery reports modified components"), Delivery.IsModifyingAny());

	Delivery.BeginFrame();
	Delivery.EndFrame();
	FLinearColor Restored;
	ReadColor(Mesh, ColorName, Restored);
	TestTrue(TEXT("Scale restored when no longer requested"), Mesh->GetRelativeScale3D().Equals(FVector(2.0), 0.0001));
	TestTrue(TEXT("Color restored when no longer requested"), FMath::IsNearlyEqual(Restored.R, BaseColor.R, 0.001f) && FMath::IsNearlyEqual(Restored.G, BaseColor.G, 0.001f));
	TestFalse(TEXT("Nothing modified after restoring"), Delivery.IsModifyingAny());

	Delivery.BeginFrame();
	Delivery.AddScale(Mesh, FVector(1.0));
	Delivery.EndFrame();
	Delivery.RestoreAll();
	TestTrue(TEXT("RestoreAll puts the base scale back"), Mesh->GetRelativeScale3D().Equals(FVector(2.0), 0.0001));

	return true;
}

#undef FEEL_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
