//
// sbfp_const.h
//
// This files contain constant definitions for SBFP formatting and arithmetic.
//
// The MIT License (MIT)
//
// Copyright (c) 2021 Luke Andrews.  All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sub-license, and/or sell copies of the Software, and to permit persons
// to whom the Software is furnished to do so, subject to the following conditions:
//
// * The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
#ifndef SBFP_CONST_H
#define SBFP_CONST_H

#include <math.h>

#define SBFP_BIT_COUNT_SIGN 1
#define SBFP_BIT_COUNT_EXPO 5
#define SBFP_BIT_COUNT_FRAC 10
#define SBFP_BIAS ((1 << (SBFP_BIT_COUNT_EXPO - 1)) - 1)

#define SBFP_NEG_INF 0x7C00
#define SBFP_POS_INF 0x3C00
#define SBFP_NAN     0x3C01

#define DOUBLE_POS_INF HUGE_VAL
#define DOUBLE_NEG_INF (HUGE_VAL * -1.0)
#define DOUBLE_NAN (INFINITY * 0.0F)

#endif
