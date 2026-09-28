// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "FeelParameters.h"
#include "FeelTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/WeakObjectPtr.h"
#include "FeelAnimNotifies.generated.h"

class UFeelRecipe;
class USkeletalMeshComponent;

/** Where a notify plays: the animated mesh or its owning actor. */
UENUM(BlueprintType)
enum class EFeelNotifyTarget : uint8
{
	/** The skeletal mesh component playing the animation. Actor effects such as scale apply to the mesh. */
	Mesh,
	/** The actor that owns the mesh. */
	OwningActor UMETA(DisplayName = "Owning Actor"),
};

/** Settings shared by FeelKit anim notifies. */
USTRUCT(BlueprintType)
struct FEELCORE_API FFeelNotifyPlaySettings
{
	GENERATED_BODY()

	/** What the recipe plays on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	EFeelNotifyTarget Target = EFeelNotifyTarget::Mesh;

	/** Intensity of the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ClampMin = "0"))
	float Intensity = 1.0f;

	/** Parameter values passed with the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TMap<FName, float> Parameters;

	/** Context tags passed with the play. Feel Maps use them to pick recipe variants. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FGameplayTagContainer ContextTags;

	/** Target and context for a notify fired on this mesh. */
	void MakePlay(USkeletalMeshComponent* MeshComp, FFeelTarget& OutTarget, FFeelPlayContext& OutContext) const;
};

/**
 * Plays a recipe at this point of an animation. Also plays in the animation editor preview, so the recipe's
 * actor effects can be tuned while scrubbing the animation.
 */
UCLASS(meta = (DisplayName = "Play Feel"))
class FEELCORE_API UAnimNotify_PlayFeel : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** Recipe to play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Target, intensity, parameters and tags for the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ShowOnlyInnerProperties))
	FFeelNotifyPlaySettings Settings;

	using Super::Notify;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

/** Sends a Feel Event at this point of an animation; Feel Maps decide which recipe plays. */
UCLASS(meta = (DisplayName = "Send Feel Event"))
class FEELCORE_API UAnimNotify_SendFeelEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** Event to send. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	FGameplayTag Event;

	/** Target, intensity, parameters and tags for the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ShowOnlyInnerProperties))
	FFeelNotifyPlaySettings Settings;

	using Super::Notify;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

/** Whose value a Set Feel Value notify sets. */
UENUM(BlueprintType)
enum class EFeelValueScope : uint8
{
	/** The actor that owns the animated mesh. Recipes played on it, or with it as instigator, read this value. */
	OwningActor UMETA(DisplayName = "Owning Actor"),
	/** One value for the whole world. */
	Global,
};

/** What a Set Feel Value notify does with each value. */
UENUM(BlueprintType)
enum class EFeelValueMode : uint8
{
	/** Replaces the accumulator's value. */
	Set,
	/** Adds to the accumulator's value (negative values subtract), for example to build up a charge each loop. */
	Add,
};

/**
 * Sets or adds to accumulator values at this point of an animation (Project Settings > Plugins > FeelKit > Accumulators). Lets an animation tell
 * recipes what kind of move is happening, for example how heavy a swing is, without any Blueprint: a recipe parameter that
 * reads the accumulator picks the value up when the move's recipe plays.
 */
UCLASS(meta = (DisplayName = "Set Feel Value"))
class FEELCORE_API UAnimNotify_SetFeelValue : public UAnimNotify
{
	GENERATED_BODY()

public:
	/** Accumulators and the values they are set to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (GetKeyOptions = "FeelBlueprintLibrary.GetFeelAccumulatorOptions"))
	TMap<FName, float> Values;

	/** Whether the values replace or add to the accumulators. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	EFeelValueMode Mode = EFeelValueMode::Set;

	/** Whose values are set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	EFeelValueScope Scope = EFeelValueScope::OwningActor;

	using Super::Notify;
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};

/**
 * Plays a recipe for the length of a notify window. At the end of the window a sustained recipe is released (it plays its
 * ending) and any other recipe is stopped, unless it is set to finish on its own.
 */
UCLASS(meta = (DisplayName = "Play Feel (Window)"))
class FEELCORE_API UAnimNotifyState_PlayFeel : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	/** Recipe to play. Recipes with a sustain region keep looping until the window ends. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	TObjectPtr<UFeelRecipe> Recipe = nullptr;

	/** Target, intensity, parameters and tags for the play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel", meta = (ShowOnlyInnerProperties))
	FFeelNotifyPlaySettings Settings;

	/** When the window ends on a recipe without sustain, let it finish on its own instead of stopping it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Feel")
	bool bLetNonSustainedRecipeFinish = false;

	using Super::NotifyBegin;
	using Super::NotifyEnd;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	/** Plays started by this notify, per mesh. Notify objects are shared by every mesh playing the animation. */
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FFeelHandle> ActivePlays;
};
