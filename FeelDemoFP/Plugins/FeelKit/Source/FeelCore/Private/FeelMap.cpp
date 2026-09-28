// Copyright 2026 Billo. All Rights Reserved.

#include "FeelMap.h"

#include "FeelRecipe.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FeelMap)

#define LOCTEXT_NAMESPACE "FeelMap"

namespace FeelMapPrivate
{
	/** Number of segments in a tag name: A.B.C is 3. */
	int32 GetTagDepth(const FGameplayTag& Tag)
	{
		const FString Name = Tag.GetTagName().ToString();
		int32 Depth = Name.IsEmpty() ? 0 : 1;
		for (const TCHAR Character : Name)
		{
			Depth += Character == TEXT('.') ? 1 : 0;
		}
		return Depth;
	}
}

const FFeelMapEntry* UFeelMap::FindBestEntry(TConstArrayView<const UFeelMap*> Maps, const FGameplayTag& Event, const FGameplayTagContainer& ContextTags)
{
	if (!Event.IsValid())
	{
		return nullptr;
	}

	const FFeelMapEntry* Best = nullptr;
	int32 BestDepth = -1;
	int32 BestTagCount = -1;
	int32 BestPriority = 0;

	for (const UFeelMap* Map : Maps)
	{
		if (!Map)
		{
			continue;
		}

		for (const FFeelMapEntry& Entry : Map->Entries)
		{
			if (!Entry.Recipe || !Entry.Event.IsValid() || !Event.MatchesTag(Entry.Event) || !ContextTags.HasAll(Entry.RequiredTags))
			{
				continue;
			}

			const int32 Depth = FeelMapPrivate::GetTagDepth(Entry.Event);
			const int32 TagCount = Entry.RequiredTags.Num();
			const bool bBetter = !Best
				|| Depth > BestDepth
				|| (Depth == BestDepth && TagCount > BestTagCount)
				|| (Depth == BestDepth && TagCount == BestTagCount && Entry.Priority > BestPriority);
			if (bBetter)
			{
				Best = &Entry;
				BestDepth = Depth;
				BestTagCount = TagCount;
				BestPriority = Entry.Priority;
			}
		}
	}
	return Best;
}

#if WITH_EDITOR
EDataValidationResult UFeelMap::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	bool bHasErrors = Result == EDataValidationResult::Invalid;

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FFeelMapEntry& Entry = Entries[EntryIndex];
		const FText RowLabel = FText::AsNumber(EntryIndex + 1);

		if (!Entry.Event.IsValid())
		{
			Context.AddError(FText::Format(LOCTEXT("EntryNoEvent", "Row {0} has no event, so it never plays."), RowLabel));
			bHasErrors = true;
		}
		if (!Entry.Recipe)
		{
			Context.AddError(FText::Format(LOCTEXT("EntryNoRecipe", "Row {0} has no recipe, so it never plays."), RowLabel));
			bHasErrors = true;
		}

		for (int32 OtherIndex = 0; OtherIndex < EntryIndex; ++OtherIndex)
		{
			const FFeelMapEntry& Other = Entries[OtherIndex];
			if (Other.Event == Entry.Event && Other.RequiredTags == Entry.RequiredTags && Other.Priority == Entry.Priority)
			{
				Context.AddWarning(FText::Format(LOCTEXT("EntryDuplicate", "Row {0} has the same event, required tags and priority as row {1}, so it never wins."), RowLabel, FText::AsNumber(OtherIndex + 1)));
				break;
			}
		}
	}

	return bHasErrors ? EDataValidationResult::Invalid : EDataValidationResult::Valid;
}
#endif

#undef LOCTEXT_NAMESPACE
