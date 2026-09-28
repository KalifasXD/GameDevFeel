// Copyright 2026 Billo. All Rights Reserved.

#include "FeelRecipeDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "FeelCurveCustomization.h"
#include "IDetailsView.h"
#include "Curves/CurveFloat.h"
#include "FeelRecipe.h"
#include "FeelRecipeEditorState.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "FeelRecipeEditor"

FFeelRecipeDetails::FFeelRecipeDetails(const TWeakPtr<FFeelRecipeEditorState>& InState)
	: State(InState)
{
}

void FFeelRecipeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Tracks are edited on the timeline; only the selected one is shown here.
	const TSharedRef<IPropertyHandle> TracksHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UFeelRecipe, Tracks));
	DetailBuilder.HideProperty(TracksHandle);

	const TSharedPtr<FFeelRecipeEditorState> PinnedState = State.Pin();
	const int32 TrackIndex = PinnedState.IsValid() ? PinnedState->GetSelectedTrack() : INDEX_NONE;
	const TSharedPtr<IPropertyHandleArray> TracksArray = TracksHandle->AsArray();

	uint32 NumTracks = 0;
	if (TrackIndex == INDEX_NONE
		|| !TracksArray.IsValid()
		|| TracksArray->GetNumElements(NumTracks) != FPropertyAccess::Success
		|| static_cast<uint32>(TrackIndex) >= NumTracks)
	{
		return;
	}

	const FName TrackCategoryName(TEXT("Track"));
	DetailBuilder.EditCategory(TrackCategoryName, FText::Format(LOCTEXT("TrackCategory", "Track {0}"), FText::AsNumber(TrackIndex + 1)), ECategoryPriority::Important);
	DetailBuilder.EditCategory(TEXT("Recipe")).InitiallyCollapsed(true);

	const TSharedRef<IPropertyHandle> TrackHandle = TracksArray->GetElement(TrackIndex);
	uint32 NumChildren = 0;
	TrackHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		const TSharedPtr<IPropertyHandle> ChildHandle = TrackHandle->GetChildHandle(ChildIndex);
		if (!ChildHandle.IsValid() || !ChildHandle->GetProperty())
		{
			continue;
		}

		const FString& CategoryMeta = ChildHandle->GetProperty()->GetMetaData(TEXT("Category"));
		const FName CategoryName = CategoryMeta.IsEmpty() ? TrackCategoryName : FName(*CategoryMeta);
		DetailBuilder.EditCategory(CategoryName).AddProperty(ChildHandle);
	}
}

void FFeelRecipeDetails::RegisterLayouts(IDetailsView& DetailsView, const TWeakPtr<FFeelRecipeEditorState>& State)
{
	DetailsView.RegisterInstancedCustomPropertyLayout(UFeelRecipe::StaticClass(), FOnGetDetailCustomizationInstance::CreateLambda([State]()
	{
		return MakeShared<FFeelRecipeDetails>(State);
	}));

	// Unreal's inline curve editor rebuilds the whole panel after every value change, which drops keyboard focus.
	DetailsView.RegisterInstancedCustomPropertyTypeLayout(FRuntimeFloatCurve::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFeelCurveCustomization::MakeInstance));
}

#undef LOCTEXT_NAMESPACE
