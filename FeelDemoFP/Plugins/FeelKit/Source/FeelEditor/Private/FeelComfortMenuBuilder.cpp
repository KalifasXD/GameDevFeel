// Copyright 2026 Billo. All Rights Reserved.

#include "FeelEditorScripting.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Slider.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "FeelComfortMenu.h"
#include "FeelRecipe.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "WidgetBlueprint.h"

#define LOCTEXT_NAMESPACE "FeelComfortMenuBuilder"

namespace FeelComfortMenuBuilder
{
	FLinearColor Srgb(uint8 R, uint8 G, uint8 B, uint8 A = 255)
	{
		return FLinearColor::FromSRGBColor(FColor(R, G, B, A));
	}

	// Quiet and dark, like the Feel Switch badge: light text on a near-black panel, one blue accent.
	const FLinearColor BackdropColor = Srgb(0, 0, 0, 90);
	const FLinearColor PanelColor = Srgb(12, 12, 13, 240);
	const FLinearColor PanelOutline = Srgb(58, 59, 62);
	const FLinearColor TitleColor = Srgb(255, 255, 255);
	const FLinearColor TextColor = Srgb(232, 232, 232);
	const FLinearColor LabelColor = Srgb(200, 202, 206);
	const FLinearColor MutedColor = Srgb(138, 140, 144);
	const FLinearColor AccentColor = Srgb(91, 141, 239);
	const FLinearColor BarColor = Srgb(70, 72, 76);
	const FLinearColor ButtonFill = Srgb(38, 40, 43);
	const FLinearColor ButtonHoverFill = Srgb(50, 53, 58);
	const FLinearColor ButtonPressedFill = Srgb(34, 48, 71);
	const FLinearColor ButtonOutline = Srgb(69, 71, 75);
	const FLinearColor DividerColor = Srgb(51, 53, 56);

	FSlateBrush RoundedBrush(const FLinearColor& Fill, float Radius, const FLinearColor& Outline = FLinearColor::Transparent, float OutlineWidth = 0.0f)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(Fill);
		Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(Outline), OutlineWidth);
		Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
		return Brush;
	}

	struct FBuilder
	{
		UWidgetBlueprint* Blueprint = nullptr;
		UWidgetTree* Tree = nullptr;

		template <typename T>
		T* Make(const TCHAR* Name = nullptr)
		{
			T* Widget = Tree->ConstructWidget<T>(T::StaticClass(), Name ? FName(Name) : NAME_None);
			// Named widgets are the ones the comfort menu logic binds to, so they must be variables. Every widget needs an
			// entry in the blueprint's widget GUID map, as the designer gives it.
			Widget->bIsVariable = Name != nullptr;
			Blueprint->OnVariableAdded(Widget->GetFName());
			return Widget;
		}

		UTextBlock* Text(const TCHAR* Name, const FText& Value, int32 Size, const FLinearColor& Color, const FName Typeface = TEXT("Regular"))
		{
			UTextBlock* Block = Make<UTextBlock>(Name);
			Block->SetText(Value);
			FSlateFontInfo Font = Block->GetFont();
			Font.Size = Size;
			Font.TypefaceFontName = Typeface;
			Block->SetFont(Font);
			Block->SetColorAndOpacity(FSlateColor(Color));
			return Block;
		}

		UVerticalBoxSlot* Add(UVerticalBox* Box, UWidget* Child, const FMargin& Padding = FMargin(0.0f))
		{
			UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
			Slot->SetPadding(Padding);
			return Slot;
		}

		UHorizontalBoxSlot* Add(UHorizontalBox* Box, UWidget* Child, bool bFill = false, const FMargin& Padding = FMargin(0.0f))
		{
			UHorizontalBoxSlot* Slot = Box->AddChildToHorizontalBox(Child);
			Slot->SetSize(FSlateChildSize(bFill ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic));
			Slot->SetVerticalAlignment(VAlign_Center);
			Slot->SetPadding(Padding);
			return Slot;
		}

		USizeBox* Sized(UWidget* Child, float Width)
		{
			USizeBox* Box = Make<USizeBox>();
			Box->SetWidthOverride(Width);
			Box->SetContent(Child);
			return Box;
		}

		UBorder* Row(const TCHAR* Name)
		{
			UBorder* Border = Make<UBorder>(Name);
			Border->SetBrush(RoundedBrush(FLinearColor::White, 5.0f));
			Border->SetBrushColor(FLinearColor::Transparent);
			Border->SetPadding(FMargin(12.0f, 7.0f));
			return Border;
		}

		void SliderRow(UVerticalBox* Parent, const TCHAR* RowName, const TCHAR* SliderName, const TCHAR* ValueName, const FText& Label, const FText& Value)
		{
			UBorder* Border = Row(RowName);
			UHorizontalBox* Line = Make<UHorizontalBox>();
			Border->SetContent(Line);
			Add(Line, Sized(Text(nullptr, Label, 16, LabelColor), 230.0f));
			USlider* Slider = Make<USlider>(SliderName);
			// A thick bar and a large round handle, readable from a distance.
			FSliderStyle SliderStyle = Slider->GetWidgetStyle();
			FSlateBrush Bar = RoundedBrush(FLinearColor::White, 3.0f);
			FSlateBrush Thumb = RoundedBrush(FLinearColor::White, 11.0f);
			Thumb.ImageSize = FVector2D(22.0f, 22.0f);
			FSlateBrush HoveredThumb = RoundedBrush(FLinearColor::White, 11.0f, TitleColor, 2.0f);
			HoveredThumb.ImageSize = FVector2D(22.0f, 22.0f);
			SliderStyle.SetNormalBarImage(Bar).SetHoveredBarImage(Bar).SetDisabledBarImage(Bar);
			SliderStyle.SetNormalThumbImage(Thumb).SetHoveredThumbImage(HoveredThumb).SetDisabledThumbImage(Thumb);
			SliderStyle.SetBarThickness(6.0f);
			Slider->SetWidgetStyle(SliderStyle);
			Slider->SetSliderBarColor(BarColor);
			Slider->SetSliderHandleColor(AccentColor);
			Slider->SetStepSize(0.05f);
			// Left and right adjust a selected slider straight away, without pressing a button to lock it first.
			Slider->RequiresControllerLock = false;
			Add(Line, Slider, true, FMargin(8.0f, 0.0f));
			UTextBlock* ValueText = Text(ValueName, Value, 16, TextColor);
			ValueText->SetJustification(ETextJustify::Right);
			Add(Line, Sized(ValueText, 90.0f));
			Add(Parent, Border, FMargin(0.0f, 1.0f));
		}

		void CheckRow(UVerticalBox* Parent, const TCHAR* RowName, const TCHAR* CheckName, const FText& Label)
		{
			UBorder* Border = Row(RowName);
			UHorizontalBox* Line = Make<UHorizontalBox>();
			Border->SetContent(Line);
			Add(Line, Text(nullptr, Label, 16, LabelColor), true);
			UCheckBox* Check = Make<UCheckBox>(CheckName);
			Check->SetIsChecked(true);
			// The engine's check box, drawn larger so it reads from a distance.
			FCheckBoxStyle CheckStyle = Check->GetWidgetStyle();
			for (FSlateBrush* Brush : { &CheckStyle.BackgroundImage, &CheckStyle.BackgroundHoveredImage, &CheckStyle.BackgroundPressedImage,
				&CheckStyle.UncheckedImage, &CheckStyle.UncheckedHoveredImage, &CheckStyle.UncheckedPressedImage,
				&CheckStyle.CheckedImage, &CheckStyle.CheckedHoveredImage, &CheckStyle.CheckedPressedImage })
			{
				Brush->ImageSize = FVector2D(26.0f, 26.0f);
			}
			Check->SetWidgetStyle(CheckStyle);
			Add(Line, Check);
			Add(Parent, Border, FMargin(0.0f, 1.0f));
		}

		UButton* Button(const TCHAR* Name, const FText& Label)
		{
			UButton* Result = Make<UButton>(Name);
			FButtonStyle Style = Result->GetStyle();
			Style.SetNormal(RoundedBrush(ButtonFill, 5.0f, ButtonOutline, 1.0f));
			Style.SetHovered(RoundedBrush(ButtonHoverFill, 5.0f, AccentColor, 1.0f));
			Style.SetPressed(RoundedBrush(ButtonPressedFill, 5.0f, AccentColor, 1.0f));
			Style.SetDisabled(RoundedBrush(ButtonFill, 5.0f, ButtonOutline, 1.0f));
			Style.SetNormalPadding(FMargin(14.0f, 8.0f));
			Style.SetPressedPadding(FMargin(14.0f, 9.0f, 14.0f, 7.0f));
			Result->SetStyle(Style);
			UButtonSlot* Slot = Cast<UButtonSlot>(Result->AddChild(Text(nullptr, Label, 15, TextColor)));
			if (Slot)
			{
				Slot->SetHorizontalAlignment(HAlign_Center);
				Slot->SetVerticalAlignment(VAlign_Center);
			}
			return Result;
		}

		UTextBlock* Section(UVerticalBox* Parent, const FText& Title)
		{
			UTextBlock* Block = Text(nullptr, Title, 13, MutedColor, TEXT("Bold"));
			Add(Parent, Block, FMargin(12.0f, 16.0f, 0.0f, 4.0f));
			return Block;
		}

		void Divider(UVerticalBox* Parent)
		{
			UBorder* Line = Make<UBorder>();
			Line->SetBrush(RoundedBrush(DividerColor, 0.0f));
			USizeBox* Box = Make<USizeBox>();
			Box->SetHeightOverride(1.0f);
			Box->SetContent(Line);
			Add(Parent, Box, FMargin(0.0f, 14.0f, 0.0f, 10.0f));
		}
	};

	void Build(FBuilder& B)
	{
		UOverlay* Root = B.Make<UOverlay>(TEXT("Root"));
		B.Tree->RootWidget = Root;

		UBorder* Backdrop = B.Make<UBorder>(TEXT("Backdrop"));
		Backdrop->SetBrush(RoundedBrush(FLinearColor::White, 0.0f));
		Backdrop->SetBrushColor(BackdropColor);
		UOverlaySlot* BackdropSlot = Root->AddChildToOverlay(Backdrop);
		BackdropSlot->SetHorizontalAlignment(HAlign_Fill);
		BackdropSlot->SetVerticalAlignment(VAlign_Fill);

		USizeBox* PanelSize = B.Make<USizeBox>(TEXT("PanelSize"));
		PanelSize->SetWidthOverride(720.0f);
		UOverlaySlot* PanelSlot = Root->AddChildToOverlay(PanelSize);
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
		PanelSlot->SetVerticalAlignment(VAlign_Center);

		UBorder* Panel = B.Make<UBorder>(TEXT("Panel"));
		Panel->SetBrush(RoundedBrush(PanelColor, 10.0f, PanelOutline, 1.0f));
		Panel->SetPadding(FMargin(24.0f, 22.0f));
		PanelSize->SetContent(Panel);

		UVerticalBox* Content = B.Make<UVerticalBox>(TEXT("Content"));
		Panel->SetContent(Content);

		UHorizontalBox* TitleLine = B.Make<UHorizontalBox>();
		B.Add(TitleLine, B.Text(TEXT("TitleText"), LOCTEXT("Title", "Comfort"), 26, TitleColor, TEXT("Bold")), true);
		B.Add(TitleLine, B.Text(TEXT("SaveStatusText"), FText::GetEmpty(), 13, MutedColor));
		B.Add(Content, TitleLine, FMargin(12.0f, 0.0f, 12.0f, 10.0f));

		B.SliderRow(Content, TEXT("MasterRow"), TEXT("MasterSlider"), TEXT("MasterValue"), LOCTEXT("Master", "Master"), LOCTEXT("Full", "100%"));

		B.Section(Content, LOCTEXT("Effects", "EFFECTS"));
		B.SliderRow(Content, TEXT("CameraShakeRow"), TEXT("CameraShakeSlider"), TEXT("CameraShakeValue"), LOCTEXT("CameraShake", "Camera shake"), LOCTEXT("Full", "100%"));
		B.SliderRow(Content, TEXT("CameraMotionRow"), TEXT("CameraMotionSlider"), TEXT("CameraMotionValue"), LOCTEXT("CameraMotion", "Camera motion"), LOCTEXT("Full", "100%"));
		B.SliderRow(Content, TEXT("FlashesRow"), TEXT("FlashesSlider"), TEXT("FlashesValue"), LOCTEXT("Flashes", "Flashes"), LOCTEXT("Full", "100%"));
		B.SliderRow(Content, TEXT("HitstopRow"), TEXT("HitstopSlider"), TEXT("HitstopValue"), LOCTEXT("Hitstop", "Hitstop and slow motion"), LOCTEXT("Full", "100%"));
		B.SliderRow(Content, TEXT("ScreenDistortionRow"), TEXT("ScreenDistortionSlider"), TEXT("ScreenDistortionValue"), LOCTEXT("ScreenDistortion", "Screen distortion"), LOCTEXT("Full", "100%"));
		B.SliderRow(Content, TEXT("HapticsRow"), TEXT("HapticsSlider"), TEXT("HapticsValue"), LOCTEXT("Haptics", "Controller vibration"), LOCTEXT("Full", "100%"));

		B.Section(Content, LOCTEXT("Presets", "PRESETS"));
		UWrapBox* Presets = B.Make<UWrapBox>(TEXT("PresetButtons"));
		auto AddPreset = [&B, Presets](const TCHAR* Name, const FText& Label)
		{
			UWrapBoxSlot* Slot = Cast<UWrapBoxSlot>(Presets->AddChild(B.Button(Name, Label)));
			if (Slot)
			{
				Slot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 8.0f));
			}
		};
		AddPreset(TEXT("DefaultPresetButton"), LOCTEXT("DefaultPreset", "Default"));
		AddPreset(TEXT("ReducedMotionPresetButton"), LOCTEXT("ReducedMotionPreset", "Reduced motion"));
		AddPreset(TEXT("ReducedFlashingPresetButton"), LOCTEXT("ReducedFlashingPreset", "Reduced flashing"));
		AddPreset(TEXT("NoHapticsPresetButton"), LOCTEXT("NoHapticsPreset", "No vibration"));
		B.Add(Content, Presets, FMargin(12.0f, 2.0f, 12.0f, 0.0f));

		B.Section(Content, LOCTEXT("Advanced", "ADVANCED"));
		B.CheckRow(Content, TEXT("CameraRollRow"), TEXT("CameraRollCheckBox"), LOCTEXT("CameraRoll", "Camera roll"));
		B.SliderRow(Content, TEXT("ZoomSpeedRow"), TEXT("ZoomSpeedSlider"), TEXT("ZoomSpeedValue"), LOCTEXT("ZoomSpeed", "Zoom speed"), LOCTEXT("NoLimit", "No limit"));
		B.CheckRow(Content, TEXT("FlashLimiterRow"), TEXT("FlashLimiterCheckBox"), LOCTEXT("FlashLimiter", "Flash limiter"));

		B.Divider(Content);

		USizeBox* DescriptionBox = B.Make<USizeBox>(TEXT("DescriptionBox"));
		DescriptionBox->SetMinDesiredHeight(46.0f);
		UTextBlock* Description = B.Text(TEXT("DescriptionText"), LOCTEXT("DescriptionStart", "Scales every effect below at once."), 16, LabelColor);
		Description->SetAutoWrapText(true);
		DescriptionBox->SetContent(Description);
		B.Add(Content, DescriptionBox, FMargin(12.0f, 0.0f, 12.0f, 10.0f));

		UHorizontalBox* Bottom = B.Make<UHorizontalBox>();
		B.Add(Bottom, B.Button(TEXT("TryButton"), LOCTEXT("Try", "Try it")));
		B.Add(Bottom, B.Make<USpacer>(), true);
		B.Add(Bottom, B.Text(TEXT("CloseHintText"), LOCTEXT("Hint", "Esc to close"), 13, MutedColor), false, FMargin(0.0f, 0.0f, 12.0f, 0.0f));
		B.Add(Bottom, B.Button(TEXT("ResetButton"), LOCTEXT("Reset", "Reset")), false, FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		B.Add(Bottom, B.Button(TEXT("CloseButton"), LOCTEXT("Close", "Close")));
		B.Add(Content, Bottom, FMargin(12.0f, 0.0f));
	}
}

bool UFeelEditorScripting::BuildComfortMenuWidget(const FString& PackagePath, const FString& AssetName, const FString& PreviewFolder, FString& OutMessage)
{
	using namespace FeelComfortMenuBuilder;

	const FString PackageName = PackagePath / AssetName;
	UWidgetBlueprint* Blueprint = LoadObject<UWidgetBlueprint>(nullptr, *(PackageName + TEXT(".") + AssetName), nullptr, LOAD_NoWarn);
	const bool bExisted = Blueprint != nullptr;
	if (!Blueprint)
	{
		UPackage* Package = CreatePackage(*PackageName);
		Blueprint = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(UFeelComfortMenu::StaticClass(), Package, FName(*AssetName), BPTYPE_Normal,
			UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
		if (!Blueprint)
		{
			OutMessage = FString::Printf(TEXT("Could not create %s."), *PackageName);
			return false;
		}
		FAssetRegistryModule::AssetCreated(Blueprint);
	}
	else if (Blueprint->ParentClass != UFeelComfortMenu::StaticClass())
	{
		OutMessage = FString::Printf(TEXT("%s exists but is not a comfort menu."), *PackageName);
		return false;
	}

	Blueprint->Modify();
	UWidgetTree* Tree = Blueprint->WidgetTree;
	if (bExisted)
	{
		// Rebuild from scratch: the old widgets move out of the way so the new ones can take their names.
		TArray<UWidget*> OldWidgets;
		Tree->GetAllWidgets(OldWidgets);
		for (UWidget* Widget : OldWidgets)
		{
			Widget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
		}
		Tree->RootWidget = nullptr;
		Blueprint->WidgetVariableNameToGuidMap.Reset();
	}

	FBuilder Builder;
	Builder.Blueprint = Blueprint;
	Builder.Tree = Tree;
	Build(Builder);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	// The preview recipes are stored on the menu's defaults, so copies of the menu keep them.
	int32 PreviewCount = 0;
	if (UFeelComfortMenu* Defaults = Blueprint->GeneratedClass ? Cast<UFeelComfortMenu>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr)
	{
		auto LoadRecipe = [&PreviewFolder](const TCHAR* Name)
		{
			return LoadObject<UFeelRecipe>(nullptr, *FString::Printf(TEXT("%s/%s.%s"), *PreviewFolder, Name, Name), nullptr, LOAD_NoWarn);
		};
		Defaults->Modify();
		Defaults->TryRecipe = LoadRecipe(TEXT("FR_Comfort_Try"));
		Defaults->PreviewRecipes.Reset();
		const TPair<EFeelComfortMenuRow, const TCHAR*> Previews[] = {
			{ EFeelComfortMenuRow::CameraShake, TEXT("FR_Comfort_CameraShake") },
			{ EFeelComfortMenuRow::CameraMotion, TEXT("FR_Comfort_CameraMotion") },
			{ EFeelComfortMenuRow::Flashes, TEXT("FR_Comfort_Flashes") },
			{ EFeelComfortMenuRow::HitstopAndSlowMo, TEXT("FR_Comfort_Hitstop") },
			{ EFeelComfortMenuRow::ScreenDistortion, TEXT("FR_Comfort_ScreenDistortion") },
			{ EFeelComfortMenuRow::Haptics, TEXT("FR_Comfort_Haptics") },
			{ EFeelComfortMenuRow::FieldOfViewSpeed, TEXT("FR_Comfort_Zoom") },
		};
		for (const TPair<EFeelComfortMenuRow, const TCHAR*>& Preview : Previews)
		{
			if (UFeelRecipe* Recipe = LoadRecipe(Preview.Value))
			{
				Defaults->PreviewRecipes.Add(Preview.Key, Recipe);
				++PreviewCount;
			}
		}
		PreviewCount += Defaults->TryRecipe ? 1 : 0;
	}

	Blueprint->MarkPackageDirty();
	const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ Blueprint->GetPackage() }, false);
	TArray<UWidget*> Widgets;
	Tree->GetAllWidgets(Widgets);
	OutMessage = FString::Printf(TEXT("%s %s with %d widgets and %d preview recipes; compiled %s; saved %s."), bExisted ? TEXT("Rebuilt") : TEXT("Created"),
		*PackageName, Widgets.Num(), PreviewCount, Blueprint->Status == BS_Error ? TEXT("with errors") : TEXT("cleanly"), bSaved ? TEXT("yes") : TEXT("no"));
	return bSaved && Blueprint->Status != BS_Error;
}

#undef LOCTEXT_NAMESPACE
