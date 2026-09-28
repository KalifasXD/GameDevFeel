// Copyright 2026 Billo. All Rights Reserved.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FeelActorDelivery.h"
#include "FeelAnimNotifies.h"
#include "FeelFrameOutput.h"
#include "FeelMap.h"
#include "FeelRecipe.h"
#include "FeelSettings.h"
#include "FeelSubsystem.h"
#include "FeelTags.h"
#include "FeelTriggerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"
#include "Steps/FeelStep_HitFlash.h"
#include "Steps/FeelStep_ScalePunch.h"
#include "Steps/FeelStep_ScreenFlash.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

#define FEEL_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

namespace FeelDemoFeatureTests
{
	/** A minimal game world with a Feel subsystem, destroyed at the end of the scope. */
	class FScopedWorld
	{
	public:
		FScopedWorld()
			: PreviousDeltaTime(FApp::GetDeltaTime())
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			World->BeginPlay();
		}

		~FScopedWorld()
		{
			FApp::SetDeltaTime(PreviousDeltaTime);
			World->RemoveFromRoot();
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UFeelSubsystem* Subsystem() const { return World->GetSubsystem<UFeelSubsystem>(); }

		AActor* SpawnActorWithMesh()
		{
			AActor* Actor = World->SpawnActor<AActor>();
			UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Actor, TEXT("Mesh"));
			Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
			Actor->SetRootComponent(Mesh);
			Mesh->RegisterComponent();
			return Actor;
		}

		void Step(float Seconds, int32 Count = 1)
		{
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FApp::SetDeltaTime(Seconds);
				Subsystem()->Tick(Seconds);
			}
		}

		UWorld* World = nullptr;

	private:
		double PreviousDeltaTime = 0.0;
	};

	/** Recipe with one constant, full-opacity flash track whose strength follows a parameter read from an accumulator. */
	UFeelRecipe* MakeAccumulatorFlashRecipe(FName Accumulator)
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_ScreenFlash* Flash = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Flash->MaxOpacity = 1.0f;
		Track.Step = Flash;
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 5.0f;
		Track.IntensityCurve.GetRichCurve()->Reset();

		const FName ParameterName(TEXT("Weight"));
		FFeelRecipeParameter& Parameter = Recipe->Parameters.AddDefaulted_GetRef();
		Parameter.Name = ParameterName;
		Parameter.MaxValue = 1.0f;
		Parameter.Accumulator = Accumulator;
		Track.ParameterMappings.AddDefaulted_GetRef().Parameter = ParameterName;
		return Recipe;
	}
}

/**
 * The Set Feel Value anim notify sets accumulators on the animated actor or globally, and a recipe played on another actor
 * reads the value of its instigator when its target has none (Phase 6B, Action/RPG kit).
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelSetValueAndInstigatorTest, "FeelKit.Runtime.SetFeelValueAndInstigator", FEEL_TEST_FLAGS)
bool FFeelSetValueAndInstigatorTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const TArray<FFeelAccumulatorDefinition> SavedAccumulators = Settings->Accumulators;
	const FName Swing(TEXT("FeelKitTestSwing"));
	FFeelAccumulatorDefinition& Definition = Settings->Accumulators.AddDefaulted_GetRef();
	Definition.Name = Swing;
	Definition.MaxValue = 1.0f;
	Definition.DecayPerSecond = 0.0f;

	{
		FScopedWorld TestWorld;
		UFeelSubsystem* Subsystem = TestWorld.Subsystem();
		AActor* Attacker = TestWorld.SpawnActorWithMesh();
		AActor* Victim = TestWorld.SpawnActorWithMesh();
		USkeletalMeshComponent* AttackerMesh = NewObject<USkeletalMeshComponent>(Attacker);
		AttackerMesh->SetupAttachment(Attacker->GetRootComponent());
		AttackerMesh->RegisterComponent();

		TStrongObjectPtr<UAnimNotify_SetFeelValue> Notify(NewObject<UAnimNotify_SetFeelValue>(GetTransientPackage()));
		Notify->Values.Add(Swing, 0.65f);
		Notify->Notify(AttackerMesh, nullptr, FAnimNotifyEventReference());
		TestEqual(TEXT("The notify sets the animated actor's value"), Subsystem->GetAccumulator(Swing, Attacker), 0.65f, 0.0001f);
		TestEqual(TEXT("The global value is untouched"), Subsystem->GetAccumulator(Swing), 0.0f, 0.0001f);
		TestTrue(TEXT("The notify names its values"), Notify->GetNotifyName().Contains(TEXT("0.65")));

		Notify->Scope = EFeelValueScope::Global;
		Notify->Values.Add(Swing, 0.3f);
		Notify->Notify(AttackerMesh, nullptr, FAnimNotifyEventReference());
		TestEqual(TEXT("Global scope sets the global value"), Subsystem->GetAccumulator(Swing), 0.3f, 0.0001f);
		TestEqual(TEXT("Global scope leaves the actor's value"), Subsystem->GetAccumulator(Swing, Attacker), 0.65f, 0.0001f);

		Notify->Mode = EFeelValueMode::Add;
		Notify->Values.Add(Swing, 0.25f);
		Notify->Notify(AttackerMesh, nullptr, FAnimNotifyEventReference());
		TestEqual(TEXT("Add mode adds to the value"), Subsystem->GetAccumulator(Swing), 0.55f, 0.0001f);
		TestTrue(TEXT("Add mode names its values with a sign"), Notify->GetNotifyName().Contains(TEXT("+0.25")));
		Notify->Mode = EFeelValueMode::Set;
		Notify->Values.Add(Swing, 0.3f);
		Notify->Notify(AttackerMesh, nullptr, FAnimNotifyEventReference());

		// A reaction played on the victim, caused by the attacker, reads the attacker's value.
		TStrongObjectPtr<UFeelRecipe> Recipe(MakeAccumulatorFlashRecipe(Swing));
		FFeelPlayContext Context;
		Context.Instigator = Attacker;
		const FFeelHandle Handle = Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Victim), 1.0f, Context);
		TestWorld.Step(0.01f);
		TestEqual(TEXT("Without a value of its own, the target reads the instigator's value"), Subsystem->GetScreenOutput().FlashAlpha, 0.65f, 0.02f);

		Subsystem->SetAccumulator(Swing, 0.2f, Victim);
		TestWorld.Step(0.01f);
		TestEqual(TEXT("The target's own value comes first"), Subsystem->GetScreenOutput().FlashAlpha, 0.2f, 0.02f);
		Subsystem->StopFeel(Handle, false);
		TestWorld.Step(0.01f);

		// Without an instigator, the global value is used.
		AActor* Bystander = TestWorld.SpawnActorWithMesh();
		Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Bystander));
		TestWorld.Step(0.01f);
		TestEqual(TEXT("With neither, the global value is used"), Subsystem->GetScreenOutput().FlashAlpha, 0.3f, 0.02f);
	}

	Settings->Accumulators = SavedAccumulators;
	return true;
}

/** Hit Flash: the overlay material slot gets a flash instance while the track plays and the mesh's own overlay comes back after. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelHitFlashTest, "FeelKit.Runtime.HitFlash", FEEL_TEST_FLAGS)
bool FFeelHitFlashTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	UMaterialInterface* FlashSource = UMaterial::GetDefaultMaterial(MD_Surface);
	UMaterialInterface* OwnOverlay = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!TestNotNull(TEXT("Engine materials load"), FlashSource) || !TestNotNull(TEXT("Engine materials load"), OwnOverlay))
	{
		return false;
	}

	// The sink keeps the strongest flash.
	{
		FFeelOutputAccumulator Accumulator;
		Accumulator.AddOverlayFlash(FlashSource, FLinearColor::Red, 0.3f);
		Accumulator.AddOverlayFlash(FlashSource, FLinearColor::Blue, 0.7f);
		Accumulator.AddOverlayFlash(nullptr, FLinearColor::Green, 1.0f);
		TestEqual(TEXT("The strongest flash wins"), Accumulator.Output.OverlayFlashAmount, 0.7f, 0.0001f);
		TestTrue(TEXT("with its color"), Accumulator.Output.OverlayFlashColor.Equals(FLinearColor::Blue));
	}

	FScopedWorld TestWorld;
	AActor* Actor = TestWorld.SpawnActorWithMesh();
	UStaticMeshComponent* Mesh = CastChecked<UStaticMeshComponent>(Actor->GetRootComponent());
	Mesh->SetOverlayMaterial(OwnOverlay);

	// Delivery: flash while requested, the mesh's own overlay back afterwards.
	{
		FFeelActorDelivery Delivery;
		Delivery.BeginFrame();
		Delivery.AddOverlayFlash(Mesh, FlashSource, FLinearColor::White, 0.5f);
		Delivery.EndFrame();
		const UMaterialInstanceDynamic* Flash = Cast<UMaterialInstanceDynamic>(Mesh->GetOverlayMaterial());
		TestTrue(TEXT("The overlay slot holds an instance of the flash material"), Flash && Flash->Parent == FlashSource);
		TestTrue(TEXT("Delivery reports the flash"), Delivery.IsModifyingAny());

		Delivery.BeginFrame();
		Delivery.EndFrame();
		TestTrue(TEXT("The mesh's own overlay comes back when the flash ends"), Mesh->GetOverlayMaterial() == OwnOverlay);
		TestFalse(TEXT("Nothing is modified afterwards"), Delivery.IsModifyingAny());

		Delivery.BeginFrame();
		Delivery.AddOverlayFlash(Mesh, FlashSource, FLinearColor::White, 1.0f);
		Delivery.EndFrame();
		Delivery.RestoreAll();
		TestTrue(TEXT("RestoreAll puts the own overlay back"), Mesh->GetOverlayMaterial() == OwnOverlay);
	}

	// Runtime: a recipe with a Hit Flash track flashes the target actor's meshes, then restores them.
	{
		UFeelSubsystem* Subsystem = TestWorld.Subsystem();
		TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		UFeelStep_HitFlash* Step = NewObject<UFeelStep_HitFlash>(Recipe.Get());
		Step->FlashMaterial = FlashSource;
		Track.Step = Step;
		Track.Channel = FeelTags::Actor_Material;
		Track.Duration = 0.4f;
		Track.IntensityCurve.GetRichCurve()->Reset();

		Subsystem->PlayFeel(Recipe.Get(), FFeelTarget::FromActor(Actor));
		TestWorld.Step(0.1f);
		const UMaterialInstanceDynamic* Flash = Cast<UMaterialInstanceDynamic>(Mesh->GetOverlayMaterial());
		TestTrue(TEXT("While the track plays the target flashes"), Flash && Flash->Parent == FlashSource);
		TestWorld.Step(0.1f, 6);
		TestTrue(TEXT("After the track the own overlay is back"), Mesh->GetOverlayMaterial() == OwnOverlay);
	}

	return true;
}

/**
 * Once physics simulates a body (a character that became a ragdoll mid-reaction), actor motion stops moving it and drops
 * its offset, so FeelKit never fights the simulation (Unreal warns "Attempting to move a fully simulated skeletal mesh").
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelMotionLeavesPhysicsBodiesTest, "FeelKit.Runtime.MotionLeavesPhysicsBodies", FEEL_TEST_FLAGS)
bool FFeelMotionLeavesPhysicsBodiesTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	FScopedWorld TestWorld;
	AActor* Actor = TestWorld.SpawnActorWithMesh();
	UStaticMeshComponent* Mesh = CastChecked<UStaticMeshComponent>(Actor->GetRootComponent());
	const FVector Start = Mesh->GetComponentLocation();

	FFeelActorDelivery Delivery;
	Delivery.BeginFrame();
	Delivery.AddTransform(Mesh, FVector(0.0, 0.0, 30.0), FRotator(0.0, 0.0, 10.0));
	Delivery.EndFrame();
	TestTrue(TEXT("Without physics the offset moves the mesh"), !Mesh->GetComponentLocation().Equals(Start, 1.0));
	TestFalse(TEXT("Delivery reports the mesh as physics free"), FFeelActorDelivery::IsOwnedByPhysics(*Mesh));

	// Physics takes over mid-motion.
	Mesh->SetSimulatePhysics(true);
	TestTrue(TEXT("The mesh now simulates physics"), FFeelActorDelivery::IsOwnedByPhysics(*Mesh));
	const FVector Taken = Mesh->GetComponentLocation();
	const FQuat TakenRotation = Mesh->GetComponentQuat();

	Delivery.BeginFrame();
	Delivery.AddTransform(Mesh, FVector(0.0, 0.0, 80.0), FRotator(0.0, 0.0, 25.0));
	Delivery.EndFrame();
	TestTrue(TEXT("A simulated mesh is not moved"), Mesh->GetComponentLocation().Equals(Taken, 0.01));
	TestTrue(TEXT("nor rotated"), Mesh->GetComponentQuat().Equals(TakenRotation, 0.0001));
	TestFalse(TEXT("and its offset is dropped"), Delivery.IsModifyingAny());

	Delivery.BeginFrame();
	Delivery.EndFrame();
	Delivery.RestoreAll();
	TestTrue(TEXT("Ending the motion does not pull the body back either"), Mesh->GetComponentLocation().Equals(Taken, 0.01));
	return true;
}

/** Feel Trigger jump and launch events: ground jump, air jump, late jump off a ledge, launch; ticks before character movement. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTriggerJumpLaunchTest, "FeelKit.Runtime.TriggerJumpAndLaunch", FEEL_TEST_FLAGS)
bool FFeelTriggerJumpLaunchTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	auto MakeRecipe = []()
	{
		UFeelRecipe* Recipe = NewObject<UFeelRecipe>(GetTransientPackage());
		FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
		Track.Step = NewObject<UFeelStep_ScreenFlash>(Recipe);
		Track.Channel = FeelTags::Screen_Flash;
		Track.Duration = 0.1f;
		return TStrongObjectPtr<UFeelRecipe>(Recipe);
	};
	TStrongObjectPtr<UFeelRecipe> JumpRecipe = MakeRecipe();
	TStrongObjectPtr<UFeelRecipe> AirRecipe = MakeRecipe();
	TStrongObjectPtr<UFeelRecipe> LaunchRecipe = MakeRecipe();

	TArray<UFeelRecipe*> Started;
	const FDelegateHandle StartedHandle = Subsystem->OnFeelStarted.AddLambda([&Started](FFeelHandle, UFeelRecipe* Recipe) { Started.Add(Recipe); });

	ACharacter* Character = TestWorld.World->SpawnActor<ACharacter>();
	UFeelTriggerComponent* Trigger = NewObject<UFeelTriggerComponent>(Character, TEXT("FeelTrigger"));
	for (const TPair<EFeelTriggerEvent, UFeelRecipe*>& Pair : TArray<TPair<EFeelTriggerEvent, UFeelRecipe*>>{ { EFeelTriggerEvent::Jumped, JumpRecipe.Get() }, { EFeelTriggerEvent::AirJumped, AirRecipe.Get() }, { EFeelTriggerEvent::Launched, LaunchRecipe.Get() } })
	{
		FFeelTriggerEntry& Entry = Trigger->Triggers.AddDefaulted_GetRef();
		Entry.Event = Pair.Key;
		Entry.Recipe = Pair.Value;
	}
	Trigger->RegisterComponent();
	// This bare test world has no game mode, so start the actor's play the way a level does.
	Character->DispatchBeginPlay();
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

	TestTrue(TEXT("Jump and launch entries make the component tick"), Trigger->IsComponentTickEnabled());
	TestTrue(TEXT("Character movement waits for the trigger, so launches are seen before they are applied"),
		Movement->PrimaryComponentTick.GetPrerequisites().ContainsByPredicate([Trigger](const FTickPrerequisite& Prerequisite) { return Prerequisite.Get() == &Trigger->PrimaryComponentTick; }));

	auto TickTrigger = [Trigger]() { Trigger->TickComponent(0.016f, LEVELTICK_All, &Trigger->PrimaryComponentTick); };

	TickTrigger();
	TestEqual(TEXT("Nothing while standing"), Started.Num(), 0);

	Character->JumpCurrentCount = 1;
	TickTrigger();
	TestTrue(TEXT("A jump from the ground fires Jumped"), Started.Num() == 1 && Started.Last() == JumpRecipe.Get());

	Character->JumpCurrentCount = 2;
	TickTrigger();
	TestTrue(TEXT("A jump in the air fires Air Jumped"), Started.Num() == 2 && Started.Last() == AirRecipe.Get());

	TickTrigger();
	TestEqual(TEXT("Holding the jump fires nothing more"), Started.Num(), 2);

	Character->JumpCurrentCount = 0;
	TickTrigger();
	TestEqual(TEXT("Landing (count reset) fires no jump"), Started.Num(), 2);

	Character->JumpCurrentCount = 2;
	TickTrigger();
	TestTrue(TEXT("A late jump just off a ledge (the engine counts it twice) is a ground jump"), Started.Num() == 3 && Started.Last() == JumpRecipe.Get());

	Movement->PendingLaunchVelocity = FVector(800.0, 0.0, 900.0);
	TickTrigger();
	TestTrue(TEXT("A pending launch fires Launched"), Started.Num() == 4 && Started.Last() == LaunchRecipe.Get());
	Movement->PendingLaunchVelocity = FVector::ZeroVector;

	Subsystem->OnFeelStarted.Remove(StartedHandle);
	Subsystem->StopAllFeel();
	return true;
}

/** Feel Trigger entries that play on the other actor of the event (a jump pad acting on whoever steps on it), and the player-only filter. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelTriggerPlayOnOtherTest, "FeelKit.Runtime.TriggerPlayOnOther", FEEL_TEST_FLAGS)
bool FFeelTriggerPlayOnOtherTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	FScopedWorld TestWorld;
	UFeelSubsystem* Subsystem = TestWorld.Subsystem();
	TStrongObjectPtr<UFeelRecipe> Recipe(NewObject<UFeelRecipe>(GetTransientPackage()));
	FFeelTrack& Track = Recipe->Tracks.AddDefaulted_GetRef();
	UFeelStep_ScalePunch* Punch = NewObject<UFeelStep_ScalePunch>(Recipe.Get());
	Punch->Amount = FVector(0.5);
	Punch->Bounces = 0;
	Track.Step = Punch;
	Track.Channel = FeelTags::Actor_Transform;
	Track.Duration = 0.4f;

	AActor* Pad = TestWorld.SpawnActorWithMesh();
	AActor* Visitor = TestWorld.SpawnActorWithMesh();
	UFeelTriggerComponent* Trigger = NewObject<UFeelTriggerComponent>(Pad, TEXT("FeelTrigger"));
	FFeelTriggerEntry& Entry = Trigger->Triggers.AddDefaulted_GetRef();
	Entry.Event = EFeelTriggerEvent::BeginOverlap;
	Entry.Recipe = Recipe.Get();
	Entry.PlayOn = EFeelTriggerPlayOn::OtherActor;
	Entry.bTargetSkeletalMesh = false;
	Trigger->RegisterComponent();

	Trigger->FireEvent(EFeelTriggerEvent::BeginOverlap, 0.0f, Visitor);
	TestWorld.Step(0.1f);
	TestTrue(TEXT("Play On Other Actor changes the actor that stepped on it"), !Visitor->GetRootComponent()->GetRelativeScale3D().Equals(FVector::OneVector, 0.01));
	TestTrue(TEXT("and not the owner"), Pad->GetRootComponent()->GetRelativeScale3D().Equals(FVector::OneVector, 0.001));
	Subsystem->StopAllFeel();
	TestWorld.Step(0.1f, 6);

	Trigger->Triggers[0].bOnlyForPlayers = true;
	const int32 Before = Subsystem->GetNumActiveInstances();
	Trigger->FireEvent(EFeelTriggerEvent::BeginOverlap, 0.0f, Visitor);
	TestEqual(TEXT("Only For Players ignores an actor no player controls"), Subsystem->GetNumActiveInstances(), Before);
	TestFalse(TEXT("A plain actor is not a player"), UFeelTriggerComponent::IsPlayerActor(Visitor));
	return true;
}

/** Sending an event that no Feel Map handles says so, once per event, instead of silently playing nothing. */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFeelUnmatchedEventWarningTest, "FeelKit.Runtime.UnmatchedEventWarns", FEEL_TEST_FLAGS)
bool FFeelUnmatchedEventWarningTest::RunTest(const FString& Parameters)
{
	using namespace FeelDemoFeatureTests;

	UFeelSettings* Settings = GetMutableDefault<UFeelSettings>();
	const TArray<TSoftObjectPtr<UFeelMap>> SavedMaps = Settings->FeelMaps;
	Settings->FeelMaps.Reset();

	AddExpectedMessage(TEXT("Send Feel Event Feel.Event.Death: no Feel Map entry matches"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	{
		FScopedWorld TestWorld;
		AActor* Actor = TestWorld.SpawnActorWithMesh();
		const FFeelHandle First = TestWorld.Subsystem()->SendFeelEvent(FeelTags::Event_Death, FFeelTarget::FromActor(Actor));
		const FFeelHandle Second = TestWorld.Subsystem()->SendFeelEvent(FeelTags::Event_Death, FFeelTarget::FromActor(Actor));
		TestFalse(TEXT("Nothing plays without a Feel Map entry"), First.IsValid() || Second.IsValid());
	}

	Settings->FeelMaps = SavedMaps;
	return true;
}

#undef FEEL_TEST_FLAGS

#endif
