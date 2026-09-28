// Copyright 2026 Billo. All Rights Reserved.

#include "FeelEditorSettings.h"

#include "FeelLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelEditorSettings)

#if WITH_EDITOR
void UFeelEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UFeelEditorSettings, bAllowLibraryEditing))
	{
		FeelLibrary::ApplyWritePermission();
	}
}
#endif
