// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FeelComfortStorage.h"
#include "FeelComfortTypes.h"
#include "UObject/WeakObjectPtr.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "FeelComfortSubsystem.generated.h"

class UFeelComfortPreset;

/**
 * One local player's comfort settings.
 * Loaded when the local player is created, so they apply before any recipe plays.
 */
UCLASS()
class FEELCORE_API UFeelComfortSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Current comfort scales of this player.
	 * @return	All scales, each from 0 to 1.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel|Comfort")
	FFeelComfortScales GetComfortScales() const { return Scales; }

	/** C++ access without a copy. */
	const FFeelComfortScales& GetComfortScalesRef() const { return Scales; }

	/**
	 * Applies new scales at once without saving them, for settings that change continuously, such as a slider being
	 * dragged. Call Save Comfort Settings when the change is done.
	 */
	void SetComfortScalesWithoutSaving(const FFeelComfortScales& NewScales);

	/**
	 * Replaces all comfort scales. Values are clamped to 0 to 1 and saved when auto save is on.
	 * @param NewScales	The new scales.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void SetComfortScales(const FFeelComfortScales& NewScales);

	/**
	 * Sets the master scale that applies to every effect.
	 * @param Scale	0 turns off all non-essential effects, 1 is full strength.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void SetMasterComfortScale(float Scale);

	/**
	 * Sets the scale of one comfort group.
	 * @param Group	The group to change.
	 * @param Scale	0 turns off the group's non-essential effects, 1 is full strength.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void SetComfortGroupScale(EFeelComfortGroup Group, float Scale);

	/**
	 * Replaces the scales with a built-in preset whose values come from the project settings.
	 * @param Preset	The preset to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void ApplyComfortPreset(EFeelBuiltInComfortPreset Preset);

	/**
	 * Replaces the scales with a custom preset asset.
	 * @param Preset	The preset asset to apply.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void ApplyCustomComfortPreset(const UFeelComfortPreset* Preset);

	/**
	 * Saves the scales through the comfort storage.
	 * @return	True if the storage reported success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	bool SaveComfortSettings();

	/**
	 * Loads the scales from the comfort storage. The current scales stay when nothing is stored.
	 * @return	True if stored settings were found.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	bool LoadComfortSettings();

	/**
	 * Routes loading and saving to your own save system, then loads from it.
	 * @param NewStorage	An object implementing Feel Comfort Storage. Leave empty to use the project's storage.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void SetComfortStorage(TScriptInterface<IFeelComfortStorage> NewStorage);

	/**
	 * How much of the game's controller vibration reaches the player right now: the player's Haptics and Master comfort
	 * combined, or 1 when Apply Comfort to Engine Force Feedback is off in the project settings. 0 means the controller
	 * stays still, whatever plays.
	 * @return	Scale from 0 to 1 applied to the game's controller vibration.
	 */
	UFUNCTION(BlueprintPure, Category = "Feel|Comfort")
	float GetEffectiveForceFeedbackScale() const;

private:
	UObject* GetStorage();
	void HandleScalesChanged();
	void HandlePlayerControllerChanged(APlayerController* NewPlayerController);

	/**
	 * Engine force feedback follows Haptics x Master when enabled in the project settings, as a multiplier on the value the
	 * game itself set. The game's value is remembered so FeelKit can put it back.
	 */
	void ApplyEngineForceFeedbackScale();

	/** Puts the game's own force feedback scale back on the controller, if FeelKit changed it. */
	void RestoreEngineForceFeedbackScale();

	FDelegateHandle PlayerControllerChangedHandle;

	/** The controller FeelKit changed, the scale the game had set, and the scale FeelKit last wrote. */
	TWeakObjectPtr<APlayerController> ScaledController;
	float GameForceFeedbackScale = 1.0f;
	float AppliedForceFeedbackScale = 1.0f;

	/** Whether the log already said that FeelKit silenced controller vibration for this player. */
	bool bWarnedAboutSilencedHaptics = false;

	UPROPERTY(Transient)
	FFeelComfortScales Scales;

	UPROPERTY(Transient)
	TObjectPtr<UObject> Storage;
};
