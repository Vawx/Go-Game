/* go_types.h : date = February 6th 2025 0:56 pm */

#if !defined(GO_TYPES_H)

#pragma warning(disable : 4018) // '<': signed/unsigned mismatch
#pragma warning(disable : 4244) //'argument': conversion from 'int' to 'float', possible loss of data
#pragma warning(disable : 4146) // unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable : 4838) // conversion from 'u32' to 'float' requires a narrowing conversion
#pragma warning(disable : 4305) // 'argument': truncation from '__int64' to 'unsigned int'
#pragma warning(disable : 4309) // 'argument': truncation of a constant value

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float r32;
typedef double r64;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"
#include "raymath.h"

#define GO_TYPES_H
#endif //GO_TYPES_H
