// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "FeelComfortTypes.h"
#include "Styling/SlateTypes.h"
#include "FeelComfortMenu.generated.h"

class APlayerController;
class UBorder;
class UButton;
class UCheckBox;
class UFeelComfortSubsystem;
class UFeelRecipe;
class USlider;
class UTextBlock;

/** The settings a comfort menu row changes. Also used as the key for row descriptions and preview recipes. */
UENUM(BlueprintType)
enum class EFeelComfortMenuRow : uint8
{
	None,
	Master,
	CameraShake UMETA(DisplayName = "Camera Shake"),
	CameraMotion UMETA(DisplayName = "Camera Motion"),
	Flashes,
	HitstopAndSlowMo UMETA(DisplayName = "Hitstop and Slow-mo"),
	ScreenDistortion UMETA(DisplayName = "Screen Distortion"),
	Haptics UMETA(DisplayName = "Controller Vibration"),
	CameraRoll UMETA(DisplayName = "Camera Roll"),
	FieldOfViewSpeed UMETA(DisplayName = "Zoom Speed Limit"),
	FlashLimiter UMETA(DisplayName = "Flash Limiter"),
	Presets,
	Try
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFeelComfortMenuClosedSignature);

/**
 * A comfort settings menu for players: Master and the six comfort groups, presets, camera roll, a zoom speed limit
 * and the flash limiter, with a live preview of each setting. The look lives in a Widget Blueprint made from this
 * class; FeelKit ships WBP_FeelComfortMenu. Every control is found by its name (MasterSlider, CameraShakeSlider,
 * TryButton and so on), and a control the Widget Blueprint leaves out is skipped. Changes apply at once and are saved
 * shortly after the player stops changing them, when Auto Save Comfort is on in the project settings.
 */
UCLASS(Abstract)
class FEELCORE_API UFeelComfortMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFeelComfortMenu(const FObjectInitializer& ObjectInitializer);

	/**
	 * Opens a comfort menu for a player. Adds it to the screen, gives it keyboard and gamepad focus, and shows the mouse
	 * cursor until it closes.
	 * @param PlayerController	The player whose comfort settings the menu changes.
	 * @param MenuClass			Menu to open. Empty uses Comfort Menu Class from Project Settings > Plugins > FeelKit.
	 * @param bPauseGame		Pauses the game while the menu is open. Previews still play.
	 * @param ZOrder			Draw order on the screen; higher is on top.
	 * @return					The opened menu, or none when no menu class could be loaded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort", meta = (DisplayName = "Show Feel Comfort Menu", AdvancedDisplay = "ZOrder"))
	static UFeelComfortMenu* ShowFeelComfortMenu(APlayerController* PlayerController, TSubclassOf<UFeelComfortMenu> MenuClass, bool bPauseGame = false, int32 ZOrder = 100);

	/** Closes the menu, saves unsaved changes and gives input back to the game. */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void CloseMenu();

	/** Reads the player's current comfort settings into every control. */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void RefreshFromSettings();

	/** Writes the keyboard keys and the first controller button of Close Keys into CloseHintText, such as "Esc or O to close, B / Circle on a controller". */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void RefreshCloseHint();

	/** Plays the Try recipe with the player's current settings. */
	UFUNCTION(BlueprintCallable, Category = "Feel|Comfort")
	void PlayTry();

	/** Called when the menu closes. */
	UPROPERTY(BlueprintAssignable, Category = "Feel|Comfort")
	FFeelComfortMenuClosedSignature OnClosed;

	/** One-line explanation shown for the selected row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu")
	TMap<EFeelComfortMenuRow, FText> Descriptions;

	/** Recipe played by the Try button and when Master changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu")
	TObjectPtr<UFeelRecipe> TryRecipe;

	/** Recipe played when a row's value changes, so the player feels the new setting. Rows without one play nothing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu")
	TMap<EFeelComfortMenuRow, TObjectPtr<UFeelRecipe>> PreviewRecipes;

	/** Plays the row's preview recipe while its value changes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu")
	bool bLivePreview = true;

	/** Shortest time between two previews of the same row, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu", meta = (ClampMin = "0", Units = "Seconds"))
	float PreviewInterval = 0.45f;

	/** Time after the last change before the settings are saved, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu", meta = (ClampMin = "0", Units = "Seconds"))
	float SaveDelay = 0.75f;

	/** Step of the comfort sliders for keyboard and gamepad, as a share of the full range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu", meta = (ClampMin = "0.01", ClampMax = "0.5"))
	float SliderStep = 0.05f;

	/** Zoom speed limit just left of the slider's far right, in degrees per second. The far right means no limit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu", meta = (ClampMin = "10", Units = "Degrees"))
	float MaxZoomSpeedLimit = 180.0f;

	/** Keys and controller buttons that close the menu. CloseHintText names them; call Refresh Close Hint after changing the list while the menu is open. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu")
	TArray<FKey> CloseKeys;

	/** Background of a row that is neither selected nor under the mouse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Style")
	FLinearColor RowColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);

	/** Background of the selected row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Style")
	FLinearColor SelectedRowColor = FLinearColor(0.023f, 0.026f, 0.031f, 1.0f);

	/** Background of a row under the mouse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Style")
	FLinearColor HoveredRowColor = FLinearColor(0.013f, 0.014f, 0.017f, 1.0f);

	/** Text shown for a zoom speed limit of zero. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Text")
	FText NoLimitText;

	/** Text shown in SaveStatusText after the settings are saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Text")
	FText SavedText;

	/** Text shown in SaveStatusText while changes wait to be saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort Menu|Text")
	FText UnsavedText;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MasterSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> CameraShakeSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> CameraMotionSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> FlashesSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> HitstopSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> ScreenDistortionSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> HapticsSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> ZoomSpeedSlider;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> MasterValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CameraShakeValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CameraMotionValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> FlashesValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> HitstopValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ScreenDistortionValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> HapticsValue;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ZoomSpeedValue;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> CameraRollCheckBox;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UCheckBox> FlashLimiterCheckBox;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> MasterRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> CameraShakeRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> CameraMotionRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> FlashesRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> HitstopRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> ScreenDistortionRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> HapticsRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> CameraRollRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> ZoomSpeedRow;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UBorder> FlashLimiterRow;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> DefaultPresetButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ReducedMotionPresetButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ReducedFlashingPresetButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> NoHapticsPresetButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> TryButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> ResetButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> CloseButton;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> DescriptionText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SaveStatusText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> CloseHintText;

private:
	UFUNCTION() void HandleMasterChanged(float Value);
	UFUNCTION() void HandleCameraShakeChanged(float Value);
	UFUNCTION() void HandleCameraMotionChanged(float Value);
	UFUNCTION() void HandleFlashesChanged(float Value);
	UFUNCTION() void HandleHitstopChanged(float Value);
	UFUNCTION() void HandleScreenDistortionChanged(float Value);
	UFUNCTION() void HandleHapticsChanged(float Value);
	UFUNCTION() void HandleZoomSpeedChanged(float Value);
	UFUNCTION() void HandleCameraRollChanged(bool bIsChecked);
	UFUNCTION() void HandleFlashLimiterChanged(bool bIsChecked);
	UFUNCTION() void HandleDefaultPreset();
	UFUNCTION() void HandleReducedMotionPreset();
	UFUNCTION() void HandleReducedFlashingPreset();
	UFUNCTION() void HandleNoHapticsPreset();
	UFUNCTION() void HandleTry();
	UFUNCTION() void HandleReset();
	UFUNCTION() void HandleClose();

	UFeelComfortSubsystem* GetComfort() const;
	void ChangeScales(EFeelComfortMenuRow Row, TFunctionRef<void(FFeelComfortScales&)> Change);
	void ApplyPreset(EFeelBuiltInComfortPreset Preset);
	void SaveNow();
	void UpdateValueTexts(const FFeelComfortScales& Scales);
	void UpdateSelection();
	void PlayPreview(EFeelComfortMenuRow Row);
	void PlayRecipe(UFeelRecipe* Recipe);

	/**
	 * Buttons show their own looks for states they have no look for: a button selected with the keyboard or a gamepad
	 * uses its Hovered look, and the preset that matches the current settings uses its Pressed look.
	 */
	struct FButtonState
	{
		TWeakObjectPtr<UButton> Button;
		FButtonStyle Original;
		bool bIsPreset = false;
		EFeelBuiltInComfortPreset Preset = EFeelBuiltInComfortPreset::Default;
		uint8 Shown = 0;
	};
	TArray<FButtonState> ButtonStates;
	void UpdateButtons();

	/** Row of the control that has focus, or of the row under the mouse; None when neither. */
	EFeelComfortMenuRow FindSelectedRow(bool& bOutFocused) const;

	TMap<EFeelComfortMenuRow, double> LastPreviewTimes;
	EFeelComfortMenuRow ShownDescriptionRow = EFeelComfortMenuRow::None;
	double LastChangeTime = 0.0;
	bool bUnsaved = false;
	bool bRequestedPausedPlayback = false;

	/** Set when the menu was opened by Show Feel Comfort Menu, which then restores input and pause on close. */
	bool bOwnsInputMode = false;
	bool bPausedGame = false;
	bool bPreviousShowMouseCursor = false;
};
