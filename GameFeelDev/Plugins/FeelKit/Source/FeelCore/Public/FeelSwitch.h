// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "FeelSwitch.generated.h"

class APlayerController;
class SFeelSwitchOverlay;
class UFeelComfortMenu;
class ULocalPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFeelSwitchedSignature, bool, bFeelEnabled);

/** One row of the controls list under the Feel Switch badge: what the player does, and which keys or buttons do it. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelSwitchControl
{
	GENERATED_BODY()

	/** What the input does, such as "Attack". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch")
	FText Action;

	/** The keys and buttons, such as "Left mouse / RB". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch")
	FText Keys;
};

/**
 * Lets players turn FeelKit off and on while playing, to see and feel the difference. Place one in a level: it shows a
 * start card that explains the switch, keeps a small ON/OFF badge in a corner of the screen, and switches when a player
 * presses one of the Switch Keys (keyboard or controller). No input assets or Blueprint changes are needed: the keys are
 * read from each local player's own key presses, next to whatever input setup the game uses.
 * Off stops only FeelKit (see Set Feel Enabled); the game's own effects keep working.
 * The Comfort Menu Keys open the player's comfort menu, so players can also try the comfort settings while playing.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Feel Switch"))
class FEELCORE_API AFeelSwitch : public AActor
{
	GENERATED_BODY()

public:
	AFeelSwitch();

	/** Keys and controller buttons that turn the feel off and on. The first keyboard key and the first controller button are named on the start card and the badge. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch")
	TArray<FKey> SwitchKeys;

	/** Whether FeelKit is on when the level starts. The choice is not saved between plays. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch")
	bool bStartEnabled = true;

	/** Show the start card and the corner badge. Turn off to build your own on-screen display with On Feel Switched. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display")
	bool bShowBuiltInDisplay = true;

	/** Show a card when the level starts that explains how to switch. It fades after Start Card Seconds or on any key or button. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display", meta = (EditCondition = "bShowBuiltInDisplay"))
	bool bShowStartCard = true;

	/** Heading of the start card. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display", meta = (EditCondition = "bShowBuiltInDisplay && bShowStartCard"))
	FText StartCardTitle;

	/** One or two lines under the heading, for example what this level shows. Leave empty for none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display", meta = (EditCondition = "bShowBuiltInDisplay && bShowStartCard", MultiLine = true))
	FText StartCardText;

	/** Seconds (real time) the start card stays before it fades, unless a key or button is pressed first. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display", meta = (EditCondition = "bShowBuiltInDisplay && bShowStartCard", ClampMin = "1.0", Units = "Seconds"))
	float StartCardSeconds = 8.0f;

	/** The level's controls, shown in a small panel under the badge for as long as the level runs. Leave empty for no panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Display", meta = (EditCondition = "bShowBuiltInDisplay", TitleProperty = "Action"))
	TArray<FFeelSwitchControl> Controls;

	/** Keys and controller buttons that open the player's comfort menu (Show Feel Comfort Menu) and close it again. They are added to the controls panel. Leave empty for none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Comfort Menu")
	TArray<FKey> ComfortMenuKeys;

	/** Menu to open. Empty uses Comfort Menu Class from Project Settings > Plugins > FeelKit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Comfort Menu")
	TSubclassOf<UFeelComfortMenu> ComfortMenuClass;

	/** Pause the game while the comfort menu is open. The menu's previews still play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Comfort Menu")
	bool bPauseInComfortMenu = true;

	/** Name of the comfort menu row in the controls panel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel Switch|Comfort Menu")
	FText ComfortMenuControlName;

	/** Called whenever FeelKit is turned off or on, from a Switch Key, a Blueprint node or the console variable feel.Enabled. */
	UPROPERTY(BlueprintAssignable, Category = "Feel Switch")
	FFeelSwitchedSignature OnFeelSwitched;

	/** Turns FeelKit off when it is on, and on when it is off. Same as pressing a Switch Key. */
	UFUNCTION(BlueprintCallable, Category = "Feel Switch")
	void Switch();

	/** Opens the comfort menu for a player, or closes it when it is open. Same as pressing a Comfort Menu Key. */
	UFUNCTION(BlueprintCallable, Category = "Feel Switch")
	void ToggleComfortMenu(APlayerController* PlayerController);

	/** The comfort menu this Feel Switch opened for a player, while it is open. */
	UFUNCTION(BlueprintPure, Category = "Feel Switch")
	UFeelComfortMenu* GetOpenComfortMenu(const APlayerController* PlayerController) const;

	/** Short name of a key for on-screen text, such as "Tab" or "View / Share" for the controller button left of center. */
	static FText GetKeyLabel(const FKey& Key);

	/** The text the start card uses to explain the switch, such as "Press Tab or View / Share (controller) to turn the feel off and on." */
	FText GetSwitchHint() const;

	/** True while the built-in display is on screen for at least one local player. */
	bool IsShowingDisplay() const { return Overlays.Num() > 0; }

	/** True while the start card is visible (not yet faded). */
	bool IsStartCardVisible() const;

	/** Rows in the controls panel on screen (0 when there is no panel or no display). */
	int32 GetShownControlRows() const;

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	struct FPlayerOverlay
	{
		TWeakObjectPtr<ULocalPlayer> Player;
		TSharedPtr<SFeelSwitchOverlay> Widget;
	};

	void UpdateOverlays();
	void RemoveOverlays();
	void ReadPlayerKeys(APlayerController& Controller);

	TArray<FPlayerOverlay> Overlays;
	TArray<TWeakObjectPtr<UFeelComfortMenu>> ComfortMenus;
	bool bLastEnabled = true;
	bool bStartCardDismissed = false;
	bool bSwitchedThisFrame = false;
	/** True for the one Feel Switch of the level that reads keys and shows the display. */
	bool bOwnsSwitch = false;
	/** Whether FeelKit was on before this level started; put back when the level ends. */
	bool bRestoreEnabledOnEnd = true;
	double StartTime = 0.0;
	double DismissTime = 0.0;
	double LastSwitchTime = -100.0;
};
