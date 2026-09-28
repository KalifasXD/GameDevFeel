// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelStep.h"
#include "FeelStep_BlueprintEvent.generated.h"

/** Which object receives a Blueprint Event step's calls. */
UENUM(BlueprintType)
enum class EFeelEventReceiver : uint8
{
	/** The recipe's target actor. */
	TargetActor UMETA(DisplayName = "Target Actor"),
	/** The pawn of the target's local player. */
	PlayerPawn UMETA(DisplayName = "Player Pawn"),
	/** The target's local player controller. */
	PlayerController UMETA(DisplayName = "Player Controller"),
	/** The Level Blueprint of the world. */
	LevelBlueprint UMETA(DisplayName = "Level Blueprint"),
};

/**
 * Calls a custom event or function on a receiver when the track starts and when it stops.
 * The event may take no inputs, or a single float that receives the track's intensity.
 */
UCLASS(meta = (DisplayName = "Blueprint Event"))
class FEELCORE_API UFeelStep_BlueprintEvent : public UFeelStep
{
	GENERATED_BODY()

public:
	/** Object that receives the calls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EFeelEventReceiver Receiver = EFeelEventReceiver::TargetActor;

	/** Custom event or function called when the track starts. Leave empty to skip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName StartEventName;

	/** Custom event or function called when the track ends. Leave empty to skip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName StopEventName;

	/** Also call the stop event when the recipe is stopped early. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	bool bCallStopEventWhenInterrupted = true;

	virtual void OnStart_Implementation(const FFeelContext& Context) override;
	virtual void OnStop_Implementation(const FFeelContext& Context, bool bInterrupted) override;
	virtual FGameplayTag GetDefaultChannel_Implementation() const override;

	/** Blueprint calls need a game world, so the editor preview cannot simulate them. */
	virtual bool SupportsPreview_Implementation() const override;

	virtual bool RequiresDuration() const override { return false; }

#if WITH_EDITOR
	virtual void ValidateStep(TArray<FText>& OutErrors, TArray<FText>& OutWarnings) const override;
#endif

	/**
	 * Calls EventName on Target, passing Intensity when the event takes a single float or double.
	 * Returns false and logs a warning when the event is missing or has another signature.
	 */
	static bool CallEvent(UObject* Target, FName EventName, float Intensity);

private:
	UObject* ResolveReceiver(const FFeelContext& Context) const;
};
