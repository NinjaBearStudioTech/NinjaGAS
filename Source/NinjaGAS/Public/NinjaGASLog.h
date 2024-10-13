// Ninja Bear Studio Inc. 2024, all rights reserved.
#pragma once

#include "CoreMinimal.h"

NINJAGAS_API DECLARE_LOG_CATEGORY_EXTERN(LogNinjaGAS, Log, All);

/**
 * Creates appropriate messages for the GAS category.
 * 
 * @param Verbosity		Verbosity level of this message. See ELogVerbosity.
 * @param Message		Message string literal.
 */
#define GAS_LOG(Verbosity, Message) \
	UE_LOG(LogNinjaGAS, Verbosity, TEXT("%s: %s"), *(GetClass()->GetName() + "::" + __FUNCTION__), TEXT(Message));

/**
 * Creates appropriate messages for the GAS category, with format arguments.
 * 
 * @param Verbosity		Verbosity level of this message. See ELogVerbosity.
 * @param Format		Format string literal in the style of printf.
 * @param Args			Comma-separated arguments used to format the message.
 */
#define GAS_LOG_ARGS(Verbosity, Format, ...) \
	UE_LOG(LogNinjaGAS, Verbosity, TEXT("%s: %s"), *(GetClass()->GetName() + "::" + __FUNCTION__), *FString::Printf(TEXT(Format), ##__VA_ARGS__));
