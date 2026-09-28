// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"

class FAdvancedPreviewScene;
class FFeelPreviewViewportClient;
class FFeelRecipeEditorState;

/**
 * Preview viewport of the recipe editor. Applies the shared evaluator output to a preview camera,
 * a sample mesh and the view overlay, without starting PIE.
 */
class SFeelPreviewViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SFeelPreviewViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<FFeelRecipeEditorState>& InState);
	virtual ~SFeelPreviewViewport() override;

	/** Silences the preview: sounds already playing stop and new ones do not start. */
	void SetAudioMuted(bool bMuted);

	/** World of the preview scene. */
	UWorld* GetPreviewWorld() const;

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
	TSharedPtr<FFeelRecipeEditorState> State;
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;
	TSharedPtr<FFeelPreviewViewportClient> ViewportClient;
};
