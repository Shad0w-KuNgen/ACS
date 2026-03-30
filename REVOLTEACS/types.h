#pragma once

// 25xx Full Hook System - Temel Tip Tanımları

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned __int64    uint64;

typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed __int64      int64;

typedef float               float32;
typedef double              float64;

#ifndef INLINE
#define INLINE __forceinline
#endif

#ifndef DWORD
typedef unsigned long       DWORD;
#endif
