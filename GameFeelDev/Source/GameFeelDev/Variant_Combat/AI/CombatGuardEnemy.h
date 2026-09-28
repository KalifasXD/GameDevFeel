// FeelKit demo kit (Action/RPG): an enemy that mostly guards. It stands its ground, turns to face the player and keeps its
// guard up, so the difference between light, heavy and charged attacks shows on the guard. Every few seconds, when the
// player is close, it lowers the guard to attack (a combo, or now and then a charged strike that breaks the player's
// guard), which is the moment to block or parry. Hits from behind, and hits while it attacks or is staggered, land as
// usual. It runs without the template's AI; a fresh one takes its place a few seconds after it dies.

#pragma once

#include "CoreMinimal.h"
#include "CombatEnemy.h"
#include "CombatGuardEnemy.generated.h"

class UCombatGuardComponent;

UCLASS(abstract)
class ACombatGuardEnemy : public ACombatEnemy
{
	GENERATED_BODY()

	/** The guard and its shield. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatGuardComponent* Guard;

public:
	ACombatGuardEnemy();

	UCombatGuardComponent* GetGuard() const { return Guard; }

protected:
	/** Within this distance of the player it turns to face them. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "cm"))
	float WatchDistance = 1000.0f;

	/** Within this distance of the player it attacks now and then. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "cm"))
	float AttackDistance = 230.0f;

	/** Shortest wait between its attacks. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "s"))
	float MinAttackInterval = 2.5f;

	/** Longest wait between its attacks. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "s"))
	float MaxAttackInterval = 4.5f;

	/** Share of its attacks that are charged strikes, which break a guard. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", ClampMax = "1"))
	float ChargedShare = 0.3f;

	/** How fast it turns to face the player. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "deg/s"))
	float TurnRate = 300.0f;

	/** Time after its death before a fresh one takes its place. */
	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0", Units = "s"))
	float ReplaceTime = 6.0f;

	/** When off it only guards and never attacks by itself. */
	UPROPERTY(EditAnywhere, Category = "Guard")
	bool bAttacksOnItsOwn = true;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleDied();

private:
	FTransform StartTransform;
	double NextAttackTime = 0.0;
};
