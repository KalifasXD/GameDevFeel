// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelParameters.h"
#include "FeelTypes.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "FeelPlayAndWaitAction.generated.h"

class UFeelRecipe;
class UFeelSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFeelPlayAndWaitPin, FFeelHandle, Handle);

/** Blueprint node that plays a recipe and continues when it ends. */
UCLASS()
class FEELCORE_API UFeelPlayAndWaitAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/**
	 * Plays a recipe and waits for it to end.
	 * On Finished fires when the recipe plays to its end. On Cancelled fires when it is stopped early, its target is
	 * destroyed, or it could not start (cooldown, max concurrent, feel.Enabled 0).
	 * @param WorldContextObject	Any object in the world to play in.
	 * @param Recipe				The recipe to play.
	 * @param Target				What the recipe plays on.
	 * @param Context				Optional parameter values, second actor and details for this play.
	 * @param Intensity				Multiplier for every track of this play.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel", meta = (DisplayName = "Play Feel and Wait", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "Context"))
	static UFeelPlayAndWaitAction* PlayFeelAndWait(UObject* WorldContextObject, UFeelRecipe* Recipe, const FFeelTarget& Target, const FFeelPlayContext& Context, float Intensity = 1.0f);

	/** The recipe played to its end. */
	UPROPERTY(BlueprintAssignable)
	FFeelPlayAndWaitPin OnFinished;

	/** The recipe was stopped early, its target was destroyed, or it could not start. */
	UPROPERTY(BlueprintAssignable)
	FFeelPlayAndWaitPin OnCancelled;

	virtual void Activate() override;

private:
	void HandleFinished(FFeelHandle FinishedHandle, UFeelRecipe* FinishedRecipe, bool bInterrupted);
	void Complete(bool bCancelled);

	TWeakObjectPtr<UObject> WorldContext;
	TWeakObjectPtr<UFeelSubsystem> Subsystem;

	UPROPERTY()
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	FFeelTarget Target;
	FFeelPlayContext Context;
	float Intensity = 1.0f;
	FFeelHandle Handle;
	FDelegateHandle FinishedDelegateHandle;
};
