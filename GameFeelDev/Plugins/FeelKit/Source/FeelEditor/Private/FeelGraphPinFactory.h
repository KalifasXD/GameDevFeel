// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"

/**
 * Blueprint pin widgets for FeelKit nodes: recipe parameter name pins (meta FeelParameterFromHandle) get a dropdown of
 * parameter names instead of a text box.
 */
class FFeelGraphPinFactory : public FGraphPanelPinFactory
{
public:
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* Pin) const override;

	/**
	 * Parameter names for a pin: the parameters of the recipe played by the node its handle pin is connected to, or every
	 * recipe parameter in the project when that recipe cannot be known while editing.
	 */
	static TArray<FName> GetParameterNames(const class UEdGraphPin& Pin, const FString& HandlePinName);

	static void Register();
	static void Unregister();
};
