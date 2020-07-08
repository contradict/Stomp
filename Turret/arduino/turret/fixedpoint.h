//
//  Super simple set of defines to help with code clearity
//  Does NOT deal with overflow / underflow properly
//  

#pragma once

//  1 signed bit, 21 bit integer, 10 bit fractional

#define FP_32x10 int32_t
#define TO_FP_32x10(num) ((num) * 1024)
#define FROM_FP_32x10_TO_INT(num) ((int32_t)(num) / 1024)
#define FROM_FP_32x10_TO_FLOAT(num) ((float)(num) / 1024)

//  1 signed bit, 20 bit integer, 11 bit fractional

#define FP_32x11 int32_t
#define TO_FP_32x11(num) ((num) * 2048)
#define FROM_FP_32x11_TO_INT(num) ((int32_t)(num) / 2048)
#define FROM_FP_32x11_TO_FLOAT(num) ((float)(num) / 2048)

//  1 signed bit, 17 bit integer, 14 bit fractional

#define FP_32x14 int32_t
#define TO_FP_32x14(num) ((int32_t)((num) * 16384L))
#define TO_FP_32x14_FROM_FP_32x20 (int32_t)num / 1024)
#define FROM_FP_32x14_TO_INT(num) ((int32_t)num / 16384L)
#define FROM_FP_32x14_TO_FLOAT(num) ((float)(num) / 16384L)

//  1 signed bit, 11 bit integer, 20 bit fractional

#define FP_32x20 int32_t
#define TO_FP_32x20(num) ((int32_t)((num) * 1048576L))
#define TO_FP_32x20_FROM_FP_32x11(num) ((int32_t)num * 512)
#define FROM_FP_32x20_TO_INT(num) ((int32_t)num / 1048576L)
#define FROM_FP_32x20_TO_FLOAT(num) ((float)(num) / 1048576L)
#define FP_32x20_POST_MUL(num) ((int32_t)num / 1048576L)
#define FP_32x20_POST_DIV(num) ((int32_t)num * 1048576L)
