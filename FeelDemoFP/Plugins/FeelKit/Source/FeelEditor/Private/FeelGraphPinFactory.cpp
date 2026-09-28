// Copyright 2026 Billo. All Rights Reserved.

#include "FeelGraphPinFactory.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "FeelRecipe.h"
#include "K2Node_CallFunction.h"
#include "SGraphPinNameList.h"

namespace FeelGraphPinFactoryPrivate
{
	const FName ParameterFromHandleMeta(TEXT("FeelParameterFromHandle"));
	TSharedPtr<FFeelGraphPinFactory> Instance;

	/** The recipe set as a literal on the node that produced the handle, such as Play Feel with its Recipe pin filled in. */
	const UFeelRecipe* FindHandleRecipe(const UEdGraphPin& Pin, const FString& HandlePinName)
	{
		const UEdGraphNode* Node = Pin.GetOwningNode();
		const UEdGraphPin* HandlePin = Node ? Node->FindPin(HandlePinName, EGPD_Input) : nullptr;
		if (!HandlePin || HandlePin->LinkedTo.Num() != 1 || !HandlePin->LinkedTo[0])
		{
			return nullptr;
		}
		const UEdGraphNode* Source = HandlePin->LinkedTo[0]->GetOwningNode();
		const UEdGraphPin* RecipePin = Source ? Source->FindPin(TEXT("Recipe"), EGPD_Input) : nullptr;
		return RecipePin && RecipePin->LinkedTo.Num() == 0 ? Cast<UFeelRecipe>(RecipePin->DefaultObject) : nullptr;
	}
}

TArray<FName> FFeelGraphPinFactory::GetParameterNames(const UEdGraphPin& Pin, const FString& HandlePinName)
{
	TArray<FName> Names;
	if (const UFeelRecipe* Recipe = FeelGraphPinFactoryPrivate::FindHandleRecipe(Pin, HandlePinName))
	{
		for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
		{
			if (!Parameter.Name.IsNone())
			{
				Names.AddUnique(Parameter.Name);
			}
		}
		return Names;
	}

	TArray<FAssetData> Assets;
	IAssetRegistry::GetChecked().GetAssetsByClass(UFeelRecipe::StaticClass()->GetClassPathName(), Assets, true);
	for (const FAssetData& Asset : Assets)
	{
		FString Tag;
		if (Asset.GetTagValue(UFeelRecipe::ParametersTagName, Tag))
		{
			TArray<FString> Parts;
			Tag.ParseIntoArray(Parts, TEXT(","), true);
			for (const FString& Part : Parts)
			{
				Names.AddUnique(FName(*Part));
			}
		}
		else if (const UFeelRecipe* Recipe = Cast<UFeelRecipe>(Asset.GetAsset()))
		{
			// Recipes saved before the tag existed.
			for (const FFeelRecipeParameter& Parameter : Recipe->Parameters)
			{
				if (!Parameter.Name.IsNone())
				{
					Names.AddUnique(Parameter.Name);
				}
			}
		}
	}
	Names.Sort([](const FName A, const FName B) { return A.LexicalLess(B); });
	return Names;
}

TSharedPtr<SGraphPin> FFeelGraphPinFactory::CreatePin(UEdGraphPin* Pin) const
{
	if (!Pin || Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Name || Pin->Direction != EGPD_Input)
	{
		return nullptr;
	}
	UK2Node_CallFunction* Node = Cast<UK2Node_CallFunction>(Pin->GetOwningNode());
	const FString HandlePinName = Node ? Node->GetPinMetaData(Pin->PinName, FeelGraphPinFactoryPrivate::ParameterFromHandleMeta) : FString();
	if (HandlePinName.IsEmpty())
	{
		return nullptr;
	}

	TArray<FName> Names = GetParameterNames(*Pin, HandlePinName);
	const FName Current(*Pin->GetDefaultAsString());
	if (!Current.IsNone())
	{
		Names.AddUnique(Current);
	}
	if (Names.Num() == 0)
	{
		return nullptr;
	}

	TArray<TSharedPtr<FName>> Options;
	for (const FName Name : Names)
	{
		Options.Add(MakeShared<FName>(Name));
	}
	return SNew(SGraphPinNameList, Pin, Options);
}

void FFeelGraphPinFactory::Register()
{
	FeelGraphPinFactoryPrivate::Instance = MakeShared<FFeelGraphPinFactory>();
	FEdGraphUtilities::RegisterVisualPinFactory(FeelGraphPinFactoryPrivate::Instance);
}

void FFeelGraphPinFactory::Unregister()
{
	if (FeelGraphPinFactoryPrivate::Instance.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(FeelGraphPinFactoryPrivate::Instance);
		FeelGraphPinFactoryPrivate::Instance.Reset();
	}
}
