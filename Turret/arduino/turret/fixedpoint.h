//  super simple set of defines to help with code clearity
//  
//  To convert into the fixed point representation multiply by the #define
//  To convert from the fixed point representation divide by the #define

#pragma once

#define FROM_FP_32x14(num) ((num) / 16384L);
#define TO_FP_32x14(num) ((num) * 16384L);

