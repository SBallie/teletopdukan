
#pragma once

#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <float.h>
#include <limits.h>
#include <iso646.h>
//#include <stdtypes.h>
#include <stdarg.h>
#include <stdint.h>

//////////////////////////////////////////////////////////////////
// Types

// Data Types
// TODO: make sure to #ifdef or include in platform-specific header
// TODO: 's' instead of 'i' prefix?
typedef uint8_t    u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef char       i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef uint8_t    b8;
typedef uint32_t  b32;
typedef int64_t   i64;
typedef uint64_t  u64;

typedef float              real32;
typedef double             real64;
typedef float                 f32;
typedef double                f64;
typedef const char*         c_str;
typedef char*           c_str_mut;

//////////////////////////////////////////////////////////////////
// Variable arg functions (t, ....)

// NOTE: uses <stdarg.h> and <stddef.h> by default
#ifndef va_start
typedef __builtin_va_list va_list;
#define va_start(ap,last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)