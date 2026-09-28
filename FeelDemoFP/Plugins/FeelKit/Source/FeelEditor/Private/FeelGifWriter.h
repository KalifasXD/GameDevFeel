// Copyright 2026 Billo. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** A GIF color table built from the frames it encodes, with a lookup from any color to its nearest entry. */
struct FFeelGifPalette
{
	TArray<FColor> Colors;

	/** Nearest palette index for each 15-bit color (5 bits per channel). */
	TArray<uint8> Lookup;

	/** Palette index of a pixel; X and Y pick a small ordered dither offset so gradients do not band. */
	uint8 Map(const FColor& Color, int32 X, int32 Y) const;
};

/** Animated GIF encoder for preview captures: one adaptive 256-color palette for all frames, LZW compression, looping. */
class FFeelGifWriter
{
public:
	/** Median-cut palette of up to 256 colors covering every pixel of every frame. */
	static FFeelGifPalette BuildPalette(const TArray<TArray<FColor>>& Frames);

	/** GIF LZW data (without the minimum code size byte and sub-block framing) for 8-bit indices. Exposed for tests. */
	static TArray<uint8> CompressLzw(TConstArrayView<uint8> Indices);

	/**
	 * Encodes frames of Width x Height pixels (each frame Width * Height colors, row by row) into a looping GIF file.
	 * DelayCentiseconds is the time each frame is shown.
	 */
	static TArray<uint8> Encode(int32 Width, int32 Height, const TArray<TArray<FColor>>& Frames, int32 DelayCentiseconds);
};
