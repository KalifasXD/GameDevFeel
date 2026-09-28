// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FeelTypes.h"
#include "GameplayTagContainer.h"
#include "InputTriggers.h"
#include "FeelInputComponent.generated.h"

class UEnhancedInputComponent;
class UFeelRecipe;
class UInputAction;
struct FInputActionValue;

/** Plays a recipe or sends a Feel Event when an input action reaches a trigger event. */
USTRUCT(BlueprintType)
struct FEELENHANCEDINPUT_API FFeelInputBinding
{
	GENERATED_BODY()

	/** The input action to listen to. It must be in an input mapping context the player has added. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<UInputAction> Action = nullptr;

	/** When to play: Started (the input begins), Triggered (each time the action's triggers fire; every frame while held for a plain press), Completed or Canceled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	ETriggerEvent PlayOn = ETriggerEvent::Started;

	/** Recipe to play. When empty, Event is sent instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Feel Event to send when no recipe is set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FGameplayTag Event;

	/** Intensity of the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ClampMin = "0"))
	float Intensity = 1.0f;

	/** Multiplies the intensity by the action value's magnitude, clamped to 0 to 1 (for example how far a trigger is pressed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bScaleIntensityByValue = false;

	/** Recipe parameter that receives the action value's magnitude. Empty sends none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FName ValueParameter;

	/**
	 * Ends the play when the input ends (Completed or Canceled), for feedback that lasts while the input is held, such as a
	 * charge-up. A sustained recipe is released and plays its ending; other recipes stop with a blend out.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bEndWhenInputEnds = false;

	/** Plays at most once until the input ends, even when Play On fires repeatedly (such as Triggered while held). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bOncePerPress = false;
};

/**
 * Plays FeelKit recipes from Enhanced Input actions without Blueprint wiring (TRG-003). Add it to a pawn or a player
 * controller and list the actions to respond to. Plays target the owning actor. Bindings attach to the owner's input
 * component once it exists and are attached again when the pawn is possessed by a new controller.
 */
UCLASS(ClassGroup = "Feel", meta = (BlueprintSpawnableComponent, DisplayName = "Feel Input"))
class FEELENHANCEDINPUT_API UFeelInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFeelInputComponent();

	/** Actions this component responds to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (TitleProperty = "Action"))
	TArray<FFeelInputBinding> Bindings;

	/**
	 * Responds to one binding as if its action fired. Used by the input bindings; callable to simulate input.
	 * @param BindingIndex		Index of the binding in Bindings, starting at 0.
	 * @param TriggerEvent		The input event to respond to, such as Started or Completed. Only bindings set to play on it respond.
	 * @param ActionMagnitude	Strength of the input, 0 to 1 for most actions. Scales the intensity when the binding uses Scale Intensity By Value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Input")
	void HandleInput(int32 BindingIndex, ETriggerEvent TriggerEvent, float ActionMagnitude = 1.0f);

	/** Attaches the bindings to an input component, replacing earlier ones. Called automatically. */
	void BindTo(UEnhancedInputComponent* InputComponent);

	/** Handle of the play a binding started and has not ended yet. */
	FFeelHandle GetActiveHandle(int32 BindingIndex) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void RefreshBinding();
	UEnhancedInputComponent* FindOwnerInputComponent() const;
	void Unbind();

	TWeakObjectPtr<UEnhancedInputComponent> BoundComponent;
	TArray<uint32> BindingHandles;
	TArray<FFeelHandle> ActiveHandles;
	TArray<bool> PressConsumed;
	FTimerHandle RefreshTimer;
};
