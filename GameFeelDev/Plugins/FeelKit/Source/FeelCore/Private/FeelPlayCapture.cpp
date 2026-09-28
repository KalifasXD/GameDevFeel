// Copyright 2026 Billo. All Rights Reserved.

#include "FeelPlayCapture.h"

FFeelPlayCaptureStore& FFeelPlayCaptureStore::Get()
{
	static FFeelPlayCaptureStore Store;
	return Store;
}

void FFeelPlayCaptureStore::Add(FFeelPlayCapture&& Capture)
{
#if !UE_BUILD_SHIPPING
	Captures.Add(MoveTemp(Capture));
	if (Captures.Num() > MaxCaptures)
	{
		Captures.RemoveAt(0, Captures.Num() - MaxCaptures);
	}
	OnChanged.Broadcast();
#endif
}

void FFeelPlayCaptureStore::Clear()
{
	Captures.Reset();
	OnChanged.Broadcast();
}
