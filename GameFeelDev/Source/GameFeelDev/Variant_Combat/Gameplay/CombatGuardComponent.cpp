#include "CombatGuardComponent.h"

#include "Engine/StaticMesh.h"
#include "FeelBlueprintLibrary.h"
#include "FeelParameters.h"
#include "FeelTypes.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"
#include "UObject/ConstructorHelpers.h"

namespace CombatGuard
{
	const FName Raise(TEXT("Raise"));
	const FName Color(TEXT("ShieldColor"));
	const FName Swing(TEXT("Swing"));
	const FName Charged(TEXT("Charged"));
	const FName Power(TEXT("Power"));

	/** How long the shield takes to appear or fade. */
	constexpr float FadeSeconds = 0.08f;
	/** Hits from one attacker closer together than this belong to the same swing. */
	constexpr double SameSwingSeconds = 0.25;
}

UCombatGuardComponent::UCombatGuardComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Sphere.Succeeded())
	{
		SetStaticMesh(Sphere.Object);
	}
	ShieldMaterialAsset = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/FeelKitDemos/ActionRPG/M_FK_GuardShield.M_FK_GuardShield")));

	// A flat oval in front of the chest, as tall as the character's upper body.
	SetRelativeLocation(FVector(60.0, 0.0, 10.0));
	SetRelativeScale3D(FVector(0.1, 0.95, 1.25));
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCastShadow(false);
	SetCanEverAffectNavigation(false);
	SetHiddenInGame(false);
	SetVisibility(false);
}

void UCombatGuardComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInterface* Material = ShieldMaterialAsset.LoadSynchronous())
	{
		ShieldMaterial = CreateDynamicMaterialInstance(0, Material);
	}
	if (ShieldMaterial)
	{
		ShieldMaterial->SetVectorParameterValue(CombatGuard::Color, ShieldColor);
		ShieldMaterial->SetScalarParameterValue(CombatGuard::Raise, 0.0f);
	}
	SetVisibility(false);
}

void UCombatGuardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Real time, so the shield keeps its pace through a hitstop.
	const float Target = IsGuardUp() ? 1.0f : 0.0f;
	const float Next = FMath::FInterpConstantTo(Shown, Target, static_cast<float>(FApp::GetDeltaTime()), 1.0f / CombatGuard::FadeSeconds);
	if (Next == Shown)
	{
		return;
	}
	Shown = Next;
	if (ShieldMaterial)
	{
		ShieldMaterial->SetScalarParameterValue(CombatGuard::Raise, Shown);
	}
	SetVisibility(Shown > 0.001f);
}

double UCombatGuardComponent::Now() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetRealTimeSeconds() : 0.0;
}

void UCombatGuardComponent::SetGuardRaised(bool bRaise)
{
	if (bRaise && !bWantRaised)
	{
		RaisedAt = Now();
	}
	bWantRaised = bRaise;
}

void UCombatGuardComponent::SetGuardSuppressed(bool bSuppress)
{
	bSuppressed = bSuppress;
}

bool UCombatGuardComponent::IsGuardUp() const
{
	return bWantRaised && !bSuppressed && !IsStaggered();
}

bool UCombatGuardComponent::IsStaggered() const
{
	return Now() < StaggeredUntil;
}

void UCombatGuardComponent::Stagger(float Seconds)
{
	StaggeredUntil = FMath::Max(StaggeredUntil, Now() + Seconds);
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->StopAnimMontage();
	}
}

void UCombatGuardComponent::Push(AActor* Actor, const FVector& Away, float Speed) const
{
	if (ACharacter* Character = Cast<ACharacter>(Actor))
	{
		Character->LaunchCharacter(Away * Speed, true, false);
	}
}

void UCombatGuardComponent::Play(UFeelRecipe* Recipe, bool bOnShield, AActor* Attacker, const FVector& ImpactPoint, float Power)
{
	AActor* Owner = GetOwner();
	if (!Recipe || !Owner)
	{
		return;
	}
	FFeelPlayContext Context;
	Context.Location = ImpactPoint;
	Context.Instigator = Attacker;
	Context.Direction = (Owner->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal();
	Context.Parameters.Add(CombatGuard::Power, Power);
	const FFeelTarget Target = bOnShield ? FFeelTarget::FromComponent(this) : FFeelTarget::FromActor(Owner);
	UFeelBlueprintLibrary::PlayFeelWithContext(this, Recipe, Target, Context);
}

ECombatGuardResult UCombatGuardComponent::TryBlock(AActor* Attacker, const FVector& ImpactPoint)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Attacker)
	{
		return ECombatGuardResult::None;
	}

	const double Time = Now();
	if (LastAttacker.Get() == Attacker && Time - LastAnswerTime < CombatGuard::SameSwingSeconds)
	{
		return LastAnswer;
	}

	ECombatGuardResult Result = ECombatGuardResult::None;
	const FVector Away = (Owner->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
	const bool bFromFront = FVector::DotProduct(Owner->GetActorForwardVector().GetSafeNormal2D(), -Away) >= FrontCosine;
	if (IsGuardUp() && bFromFront)
	{
		// How heavy the swing is comes from the attack animations' Set Feel Value markers, the same values the hit recipes read.
		const float Swing = UFeelBlueprintLibrary::GetFeelAccumulator(this, CombatGuard::Swing, Attacker);
		const float Charged = UFeelBlueprintLibrary::GetFeelAccumulator(this, CombatGuard::Charged, Attacker);
		const float Power = Swing > 0.0f ? FMath::Clamp(Swing, 0.0f, 1.0f) : 0.5f;

		if (bCanParry && Time - RaisedAt <= ParryWindow)
		{
			Result = ECombatGuardResult::Parried;
			if (UCombatGuardComponent* AttackerGuard = Attacker->FindComponentByClass<UCombatGuardComponent>())
			{
				AttackerGuard->Stagger(ParryStagger);
			}
			else if (ACharacter* AttackerCharacter = Cast<ACharacter>(Attacker))
			{
				AttackerCharacter->StopAnimMontage();
			}
			Push(Attacker, -Away, ParryPush);
			Play(ParryFeel, false, Attacker, ImpactPoint, 1.0f);
		}
		else if (Charged >= 0.5f)
		{
			Result = ECombatGuardResult::Broken;
			Stagger(BreakTime);
			Push(Owner, Away, BreakPush);
			Play(BreakFeel, false, Attacker, ImpactPoint, 1.0f);
		}
		else
		{
			Result = ECombatGuardResult::Blocked;
			Push(Owner, Away, BlockPush * FMath::Lerp(0.35f, 1.0f, Power));
			Play(BlockFeel, true, Attacker, ImpactPoint, Power);
		}
	}

	LastAttacker = Attacker;
	LastAnswerTime = Time;
	LastAnswer = Result;
	return Result;
}
