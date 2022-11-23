// Copyright Snaps 2022, All Rights Reserved.

#pragma once

/*
	CoreMinimal is our main PCH for typical includes or typedefs.
*/

#include <stdint.h>
#include <Memory>

using uint8 	= uint8_t;
using uint16 	= uint16_t;
using uint32 	= uint32_t;
using uint64 	= uint64_t;
using int8 		= int8_t;
using int16 	= int16_t;
using int32 	= int32_t;
using int64 	= int64_t;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
#endif

#include <string>
#include <wrl.h>
#include <shellapi.h>