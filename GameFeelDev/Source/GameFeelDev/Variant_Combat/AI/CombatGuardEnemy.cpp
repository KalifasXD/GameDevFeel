#include "CombatGuardEnemy.h"

#include "CombatGuardComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACombatGuardEnemy::ACombatGuardEnemy()
{
	Guard = CreateDefaultSubobject<UCombatGuardComponent>(TEXT("Guard"));
	Guard->SetupAttachment(RootComponent);
	Guard->ShieldColor = FLinearColor(1.0f, 0.35f, 0.1f);
	// Long enough to punish a broken guard after the player's own charged strike has recovered.
	Guard->BreakTime = 2.5f;

	// No template AI: this one only guards, turns and attacks when the player is close.
	AutoPossessAI = EAutoPossessAI::Disabled;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	MaxHP = 6.0f;
}

void ACombatGuardEnemy::BeginPlay()
{
	Super::BeginPlay();

	StartTransform = GetActorTransform();
	NextAttackTime = GetWorld()->GetTimeSeconds() + MinAttackInterval;
	OnEnemyDied.AddDynamic(this, &ACombatGuardEnemy::HandleDied);
	Guard->SetGuardRaised(true);
}

void ACombatGuardEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentHP <= 0.0f)
	{
		Guard->SetGuardRaised(false);
		return;
	}

	// The guard is up whenever it is not attacking.
	Guard->SetGuardSuppressed(bIsAttacking);

	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	const double Distance = ToPlayer.Size2D();
	if (Distance > WatchDistance || bIsAttacking || Guard->IsStaggered())
	{
		return;
	}

	const FRotator Facing(0.0, ToPlayer.Rotation().Yaw, 0.0);
	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), Facing, DeltaSeconds, TurnRate));

	const double Now = GetWorld()->GetTimeSeconds();
	if (bAttacksOnItsOwn && Distance <= AttackDistance && Now >= NextAttackTime)
	{
		NextAttackTime = Now + FMath::FRandRange(MinAttackInterval, MaxAttackInterval);
		if (FMath::FRand() < ChargedShare)
		{
			DoAIChargedAttack();
		}
		else
		{
			DoAIComboAttack();
		}
		Guard->SetGuardSuppressed(true);
	}
}

void ACombatGuardEnemy::HandleDied()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	UClass* Class = GetClass();
	const FTransform Where = StartTransform;
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(World, [World, Class, Where]()
	{
		FActorSpawnParameters Parameters;
		Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		World->SpawnActor<AActor>(Class, Where, Parameters);
	}), ReplaceTime, false);
}
