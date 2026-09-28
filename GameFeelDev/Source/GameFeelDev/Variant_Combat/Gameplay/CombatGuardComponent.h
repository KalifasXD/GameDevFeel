// FeelKit demo kit (Action/RPG): a guard for the Combat template's characters.
// While raised it shows an energy shield in front of its owner and takes hits that arrive from the front: a block (no
// damage, the defender slides back, heavier swings push harder), a guard break (a charged strike shatters the shield and
// leaves the defender open for a moment) or, where allowed, a parry (the guard went up just before the hit, and the
// attacker staggers). Attack traces ask the guard before they deal damage (TryBlock). FeelKit plays the reactions;
// everything here also works with the feel switched off.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "CombatGuardComponent.generated.h"

class UFeelRecipe;
class UMaterialInstanceDynamic;

/** What a guard did with a hit. */
UENUM(BlueprintType)
enum class ECombatGuardResult : uint8
{
	/** The hit goes through: no guard up, the guard is broken, or the hit came from behind. */
	None,
	/** The guard held. */
	Blocked,
	/** A charged strike broke the guard. */
	Broken,
	/** The guard went up just before the hit. */
	Parried,
};

UCLASS(ClassGroup = Combat, meta = (BlueprintSpawnableComponent))
class UCombatGuardComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UCombatGuardComponent();

	/** Raises or lowers the guard. While broken or staggered the guard stays down until it recovers. */
	UFUNCTION(BlueprintCallable, Category = "Guard")
	void SetGuardRaised(bool bRaise);

	/** Keeps the guard down while true, without counting as lowering it (the owner is attacking). The parry window only opens when the guard is raised, not when this ends. */
	UFUNCTION(BlueprintCallable, Category = "Guard")
	void SetGuardSuppressed(bool bSuppress);

	/** True while the guard is up and able to take hits. */
	UFUNCTION(BlueprintPure, Category = "Guard")
	bool IsGuardUp() const;

	/** True while the owner is recovering from a broken guard or a parried attack. */
	UFUNCTION(BlueprintPure, Category = "Guard")
	bool IsStaggered() const;

	/** Called by an attack before it deals damage. Anything but None means the guard took the hit and no damage is dealt. */
	ECombatGuardResult TryBlock(AActor* Attacker, const FVector& ImpactPoint);

	/** Leaves the owner unable to guard for Seconds and interrupts what it was doing. */
	void Stagger(float Seconds);

	/** Material of the shield. It should read the parameters Raise (0 hidden, 1 fully up), ShieldColor and Glow. */
	UPROPERTY(EditAnywhere, Category = "Guard")
	TSoftObjectPtr<UMaterialInterface> ShieldMaterialAsset;

	/** Colour of the shield. */
	UPROPERTY(EditAnywhere, Category = "Guard")
	FLinearColor ShieldColor = FLinearColor(0.15f, 0.65f, 1.0f);

	/** Whether a guard raised just before a hit parries it. */
	UPROPERTY(EditAnywhere, Category = "Guard")
	bool bCanParry = false;

	/** How soon before the hit the guard must go up to parry. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (EditCondition = "bCanParry", ClampMin = "0", Units = "s"))
	float ParryWindow = 0.2f;

	/** How far round the front a hit still meets the guard, as the cosine of the angle: 0.2 is about 78 degrees to either side. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "-1", ClampMax = "1"))
	float FrontCosine = 0.2f;

	/** How hard the heaviest blocked swing pushes the defender back. Lighter swings push less. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "cm/s"))
	float BlockPush = 450.0f;

	/** How hard a guard break pushes the defender back. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "cm/s"))
	float BreakPush = 800.0f;

	/** How long a broken guard stays down. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "s"))
	float BreakTime = 1.6f;

	/** How long a parried attacker is staggered, if it has a guard of its own; others are only interrupted and pushed back. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "s"))
	float ParryStagger = 1.4f;

	/** How hard a parry pushes the attacker back. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "cm/s"))
	float ParryPush = 500.0f;

	/** Plays on the shield when the guard holds. Parameter Power: 0.35 first swing, 0.65 second, 1 finisher. */
	UPROPERTY(EditAnywhere, Category = "Feel")
	TObjectPtr<UFeelRecipe> BlockFeel;

	/** Plays on the owner when a charged strike breaks the guard. */
	UPROPERTY(EditAnywhere, Category = "Feel")
	TObjectPtr<UFeelRecipe> BreakFeel;

	/** Plays on the owner when it parries; the attacker is the instigator. */
	UPROPERTY(EditAnywhere, Category = "Feel")
	TObjectPtr<UFeelRecipe> ParryFeel;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	double Now() const;
	void Push(AActor* Actor, const FVector& Away, float Speed) const;
	void Play(UFeelRecipe* Recipe, bool bOnShield, AActor* Attacker, const FVector& ImpactPoint, float Power);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ShieldMaterial;

	bool bWantRaised = false;
	bool bSuppressed = false;
	float Shown = 0.0f;
	double RaisedAt = -1.0;
	double StaggeredUntil = 0.0;

	/** A sweep reports every component it touches, so one swing can arrive more than once; it gets one answer. */
	TWeakObjectPtr<AActor> LastAttacker;
	double LastAnswerTime = -1.0;
	ECombatGuardResult LastAnswer = ECombatGuardResult::None;
};
