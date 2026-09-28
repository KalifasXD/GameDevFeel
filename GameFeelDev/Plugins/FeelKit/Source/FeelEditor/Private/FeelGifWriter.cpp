// Copyright 2026 Billo. All Rights Reserved.

#include "FeelGifWriter.h"

namespace FeelGifWriterPrivate
{
	/** 15-bit color key, 5 bits per channel. */
	int32 ToKey(int32 Red, int32 Green, int32 Blue)
	{
		return (FMath::Clamp(Red, 0, 255) >> 3) << 10 | (FMath::Clamp(Green, 0, 255) >> 3) << 5 | (FMath::Clamp(Blue, 0, 255) >> 3);
	}

	/** A box of the 15-bit color cube for median cut. */
	struct FColorBox
	{
		TArray<int32> Keys;
		int64 Count = 0;
		int32 LongestChannel = 0;
		int32 LongestRange = 0;
	};

	void MeasureBox(FColorBox& Box)
	{
		int32 Min[3] = { 31, 31, 31 };
		int32 Max[3] = { 0, 0, 0 };
		for (int32 Key : Box.Keys)
		{
			const int32 Channels[3] = { (Key >> 10) & 31, (Key >> 5) & 31, Key & 31 };
			for (int32 Channel = 0; Channel < 3; ++Channel)
			{
				Min[Channel] = FMath::Min(Min[Channel], Channels[Channel]);
				Max[Channel] = FMath::Max(Max[Channel], Channels[Channel]);
			}
		}
		Box.LongestChannel = 0;
		Box.LongestRange = -1;
		for (int32 Channel = 0; Channel < 3; ++Channel)
		{
			// Green differences are the most visible, so green ranges count a little more.
			const int32 Range = (Max[Channel] - Min[Channel]) * (Channel == 1 ? 5 : 4);
			if (Range > Box.LongestRange)
			{
				Box.LongestRange = Range;
				Box.LongestChannel = Channel;
			}
		}
	}

	void WriteUInt16(TArray<uint8>& Out, int32 Value)
	{
		Out.Add(static_cast<uint8>(Value & 0xFF));
		Out.Add(static_cast<uint8>((Value >> 8) & 0xFF));
	}

	/** Classic GIF LZW (the compress algorithm with a 5003-entry open hash), 8-bit initial code size. */
	class FLzwEncoder
	{
	public:
		explicit FLzwEncoder(TArray<uint8>& InOut)
			: Out(InOut)
		{
			HashKeys.Init(-1, HashSize);
			HashCodes.Init(0, HashSize);
		}

		void Compress(TConstArrayView<uint8> Indices)
		{
			NumBits = InitBits;
			MaxCode = MakeMaxCode(NumBits);
			FreeCode = ClearCode + 2;
			bClearFlag = false;

			Output(ClearCode);
			if (Indices.Num() == 0)
			{
				Output(EndCode);
				return;
			}

			int32 Prefix = Indices[0];
			for (int32 Position = 1; Position < Indices.Num(); ++Position)
			{
				const int32 Character = Indices[Position];
				const int32 FullCode = (Character << MaxBits) + Prefix;
				int32 Slot = (Character << HashShift) ^ Prefix;

				if (HashKeys[Slot] == FullCode)
				{
					Prefix = HashCodes[Slot];
					continue;
				}

				bool bFound = false;
				if (HashKeys[Slot] >= 0)
				{
					const int32 Displacement = Slot == 0 ? 1 : HashSize - Slot;
					do
					{
						Slot -= Displacement;
						if (Slot < 0)
						{
							Slot += HashSize;
						}
						if (HashKeys[Slot] == FullCode)
						{
							Prefix = HashCodes[Slot];
							bFound = true;
							break;
						}
					}
					while (HashKeys[Slot] >= 0);
				}
				if (bFound)
				{
					continue;
				}

				Output(Prefix);
				Prefix = Character;
				if (FreeCode < MaxMaxCode)
				{
					HashCodes[Slot] = FreeCode++;
					HashKeys[Slot] = FullCode;
				}
				else
				{
					ClearTable();
				}
			}

			Output(Prefix);
			Output(EndCode);
		}

	private:
		static constexpr int32 HashSize = 5003;
		static constexpr int32 HashShift = 4;
		static constexpr int32 MaxBits = 12;
		static constexpr int32 MaxMaxCode = 1 << MaxBits;
		static constexpr int32 InitBits = 9;
		static constexpr int32 ClearCode = 256;
		static constexpr int32 EndCode = 257;

		static int32 MakeMaxCode(int32 Bits) { return (1 << Bits) - 1; }

		void ClearTable()
		{
			for (int32& Key : HashKeys)
			{
				Key = -1;
			}
			FreeCode = ClearCode + 2;
			bClearFlag = true;
			Output(ClearCode);
		}

		void Output(int32 Code)
		{
			Accumulator |= static_cast<uint64>(Code) << AccumulatedBits;
			AccumulatedBits += NumBits;
			while (AccumulatedBits >= 8)
			{
				Out.Add(static_cast<uint8>(Accumulator & 0xFF));
				Accumulator >>= 8;
				AccumulatedBits -= 8;
			}

			// Code width grows once the next free code no longer fits, and resets after a clear.
			if (FreeCode > MaxCode || bClearFlag)
			{
				if (bClearFlag)
				{
					NumBits = InitBits;
					MaxCode = MakeMaxCode(NumBits);
					bClearFlag = false;
				}
				else
				{
					++NumBits;
					MaxCode = NumBits == MaxBits ? MaxMaxCode : MakeMaxCode(NumBits);
				}
			}

			if (Code == EndCode)
			{
				while (AccumulatedBits > 0)
				{
					Out.Add(static_cast<uint8>(Accumulator & 0xFF));
					Accumulator >>= 8;
					AccumulatedBits = FMath::Max(AccumulatedBits - 8, 0);
				}
			}
		}

		TArray<uint8>& Out;
		TArray<int32> HashKeys;
		TArray<int32> HashCodes;
		uint64 Accumulator = 0;
		int32 AccumulatedBits = 0;
		int32 NumBits = InitBits;
		int32 MaxCode = 0;
		int32 FreeCode = 0;
		bool bClearFlag = false;
	};
}

uint8 FFeelGifPalette::Map(const FColor& Color, int32 X, int32 Y) const
{
	// 4x4 Bayer matrix, scaled to about half a 5-bit step.
	static constexpr int32 Bayer[16] = { 0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5 };
	const int32 Offset = (Bayer[(Y & 3) * 4 + (X & 3)] - 8) / 2;
	const int32 Key = FeelGifWriterPrivate::ToKey(Color.R + Offset, Color.G + Offset, Color.B + Offset);
	return Lookup.IsValidIndex(Key) ? Lookup[Key] : 0;
}

FFeelGifPalette FFeelGifWriter::BuildPalette(const TArray<TArray<FColor>>& Frames)
{
	using namespace FeelGifWriterPrivate;

	TArray<int64> Histogram;
	Histogram.SetNumZeroed(32768);
	for (const TArray<FColor>& Frame : Frames)
	{
		for (const FColor& Pixel : Frame)
		{
			++Histogram[ToKey(Pixel.R, Pixel.G, Pixel.B)];
		}
	}

	TArray<FColorBox> Boxes;
	FColorBox& First = Boxes.AddDefaulted_GetRef();
	for (int32 Key = 0; Key < Histogram.Num(); ++Key)
	{
		if (Histogram[Key] > 0)
		{
			First.Keys.Add(Key);
			First.Count += Histogram[Key];
		}
	}
	MeasureBox(First);

	// Split the box with the most pixels (and some spread) at the pixel median of its longest channel.
	while (Boxes.Num() < 256)
	{
		int32 BestBox = INDEX_NONE;
		double BestScore = 0.0;
		for (int32 BoxIndex = 0; BoxIndex < Boxes.Num(); ++BoxIndex)
		{
			const FColorBox& Box = Boxes[BoxIndex];
			if (Box.Keys.Num() < 2 || Box.LongestRange <= 0)
			{
				continue;
			}
			const double Score = static_cast<double>(Box.Count) * Box.LongestRange;
			if (Score > BestScore)
			{
				BestScore = Score;
				BestBox = BoxIndex;
			}
		}
		if (BestBox == INDEX_NONE)
		{
			break;
		}

		FColorBox Source = MoveTemp(Boxes[BestBox]);
		const int32 Shift = Source.LongestChannel == 0 ? 10 : (Source.LongestChannel == 1 ? 5 : 0);
		Source.Keys.Sort([Shift](int32 A, int32 B) { return ((A >> Shift) & 31) < ((B >> Shift) & 31); });

		FColorBox Low;
		FColorBox High;
		int64 Running = 0;
		for (int32 Index = 0; Index < Source.Keys.Num(); ++Index)
		{
			const int32 Key = Source.Keys[Index];
			const bool bLow = Index == 0 || (Running < Source.Count / 2 && Index < Source.Keys.Num() - 1);
			FColorBox& Into = bLow ? Low : High;
			Into.Keys.Add(Key);
			Into.Count += Histogram[Key];
			Running += Histogram[Key];
		}
		MeasureBox(Low);
		MeasureBox(High);
		Boxes[BestBox] = MoveTemp(Low);
		Boxes.Add(MoveTemp(High));
	}

	FFeelGifPalette Palette;
	for (const FColorBox& Box : Boxes)
	{
		double Red = 0.0;
		double Green = 0.0;
		double Blue = 0.0;
		double Total = 0.0;
		for (int32 Key : Box.Keys)
		{
			const double Weight = static_cast<double>(Histogram[Key]);
			Red += (((Key >> 10) & 31) * 255.0 / 31.0) * Weight;
			Green += (((Key >> 5) & 31) * 255.0 / 31.0) * Weight;
			Blue += ((Key & 31) * 255.0 / 31.0) * Weight;
			Total += Weight;
		}
		if (Total > 0.0)
		{
			Palette.Colors.Add(FColor(static_cast<uint8>(FMath::RoundToInt32(Red / Total)), static_cast<uint8>(FMath::RoundToInt32(Green / Total)), static_cast<uint8>(FMath::RoundToInt32(Blue / Total))));
		}
	}
	if (Palette.Colors.Num() == 0)
	{
		Palette.Colors.Add(FColor::Black);
	}

	// Nearest palette entry for every 15-bit color.
	Palette.Lookup.SetNumUninitialized(32768);
	for (int32 Key = 0; Key < 32768; ++Key)
	{
		const int32 Red = ((Key >> 10) & 31) * 255 / 31;
		const int32 Green = ((Key >> 5) & 31) * 255 / 31;
		const int32 Blue = (Key & 31) * 255 / 31;
		int32 Best = 0;
		int32 BestDistance = MAX_int32;
		for (int32 Index = 0; Index < Palette.Colors.Num(); ++Index)
		{
			const FColor& Entry = Palette.Colors[Index];
			const int32 DeltaRed = Red - Entry.R;
			const int32 DeltaGreen = Green - Entry.G;
			const int32 DeltaBlue = Blue - Entry.B;
			const int32 Distance = 2 * DeltaRed * DeltaRed + 4 * DeltaGreen * DeltaGreen + 3 * DeltaBlue * DeltaBlue;
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				Best = Index;
			}
		}
		Palette.Lookup[Key] = static_cast<uint8>(Best);
	}
	return Palette;
}

TArray<uint8> FFeelGifWriter::CompressLzw(TConstArrayView<uint8> Indices)
{
	TArray<uint8> Result;
	FeelGifWriterPrivate::FLzwEncoder Encoder(Result);
	Encoder.Compress(Indices);
	return Result;
}

TArray<uint8> FFeelGifWriter::Encode(int32 Width, int32 Height, const TArray<TArray<FColor>>& Frames, int32 DelayCentiseconds)
{
	using namespace FeelGifWriterPrivate;

	TArray<uint8> Out;
	if (Width <= 0 || Height <= 0 || Width > 0xFFFF || Height > 0xFFFF)
	{
		return Out;
	}

	// Header and logical screen with a 256-entry global color table.
	Out.Append(reinterpret_cast<const uint8*>("GIF89a"), 6);
	WriteUInt16(Out, Width);
	WriteUInt16(Out, Height);
	Out.Add(0xF7);
	Out.Add(0);
	Out.Add(0);
	const FFeelGifPalette Palette = BuildPalette(Frames);
	for (int32 Index = 0; Index < 256; ++Index)
	{
		const FColor Color = Palette.Colors.IsValidIndex(Index) ? Palette.Colors[Index] : FColor::Black;
		Out.Add(Color.R);
		Out.Add(Color.G);
		Out.Add(Color.B);
	}

	// Loop forever.
	const uint8 LoopExtension[] = { 0x21, 0xFF, 0x0B, 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', 0x03, 0x01, 0x00, 0x00, 0x00 };
	Out.Append(LoopExtension, UE_ARRAY_COUNT(LoopExtension));

	TArray<uint8> Indices;
	Indices.SetNumUninitialized(Width * Height);
	for (const TArray<FColor>& Frame : Frames)
	{
		if (Frame.Num() != Width * Height)
		{
			continue;
		}

		// Graphic control extension: frame delay.
		Out.Add(0x21);
		Out.Add(0xF9);
		Out.Add(0x04);
		Out.Add(0x00);
		WriteUInt16(Out, DelayCentiseconds);
		Out.Add(0x00);
		Out.Add(0x00);

		// Image descriptor covering the whole screen, no local color table.
		Out.Add(0x2C);
		WriteUInt16(Out, 0);
		WriteUInt16(Out, 0);
		WriteUInt16(Out, Width);
		WriteUInt16(Out, Height);
		Out.Add(0x00);

		for (int32 Pixel = 0; Pixel < Frame.Num(); ++Pixel)
		{
			Indices[Pixel] = Palette.Map(Frame[Pixel], Pixel % Width, Pixel / Width);
		}

		Out.Add(8);
		const TArray<uint8> Compressed = CompressLzw(Indices);
		for (int32 Offset = 0; Offset < Compressed.Num(); Offset += 255)
		{
			const int32 BlockSize = FMath::Min(255, Compressed.Num() - Offset);
			Out.Add(static_cast<uint8>(BlockSize));
			Out.Append(Compressed.GetData() + Offset, BlockSize);
		}
		Out.Add(0x00);
	}

	Out.Add(0x3B);
	return Out;
}
