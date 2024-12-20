// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define Print_Text(Message) \
if (GEngine) \
{ \
    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, FString::Printf(TEXT("%s"), Message)); \
}

#define Print_float(Value) \
if (GEngine) \
{ \
    GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("%f"), Value)); \
}

#define Print_String(String1, String2) \
if (GEngine) \
{ \
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Black, FString::Printf(TEXT("%s, %s"), String1, *String2)); \
}