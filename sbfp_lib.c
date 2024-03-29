//
// sbfp_lib.c
//
// This file contains function definitions for SBFP formatting and arithmetic. 
// An SBFP is a Standard Binary Floating Point, defined by the sbfp_t type (see sbfp_lib.h).
// The IEEE 754 standard uses the following format for floating points:
// 		- 32-bit precision
// 			Sign = bit 31
// 			Expo = bits 23-30
// 			Frac = bits 0-22
// 		- 64-bit precision
// 			Sign = bit 63
// 			Expo = bits 52-62
// 			Frac = bits 0-51
// In this implementation, an SBFP uses 16-bit precision:
// 		- 16-bit precision
// 			Sign = bit 15
// 			Expo = bits 10-14
// 			Frac = bits 0-9
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
#include "sbfp_const.h"
#include "sbfp_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//
// Extracts the fraction of a given double value and stores it as an integer.
//
// [in] value - the double value whose fraction to extract
//
// Returns the extracted fraction as an integer.
//
static int extract_frac(double value)
{
	double fracDbl = value - (long)value;
	int    fracInt = 0;

	for (int count = 0; count < SBFP_BIT_COUNT_FRAC; ++count)
	{
		fracInt <<= 1;

		double temp = fracDbl * 2;

		if (temp >= 1.0)
		{
			fracInt += 1;

			fracDbl = temp - (long)temp;
		}
		else
		{
			fracDbl = temp;
		}
	}

	return fracInt;
}

//
// Converts a given double value to the sbfp_t type.
//
// [in] value - the double value to be converted
// 
// Returns the converted value.
//
sbfp_t double_to_sbfp(double dblValue)
{
	int status = 0;

	sbfp_t sbfpValue = 0;
	int    sbfpSign  = 0;
	int    sbfpExpo  = 0;
	int    sbfpFrac  =  0;

	//
	// Extract sign (treating 0 as positive):
	//
	if (status == 0)
	{
		if (dblValue < 0.0)
		{
			sbfpSign = 1; // negative
			dblValue *= -1.0;
		}
		else
		{
			sbfpSign = 0; // positive
		}
	}

	//
	// Determine infinity:
	//
	unsigned long long dblValueWhole = (unsigned long long)dblValue;

	if (status == 0)
	{
		if (dblValueWhole >= (1 << (SBFP_BIAS + 1)))
		{
			if (sbfpSign == 1)
			{
				sbfpValue = SBFP_NEG_INF;
			}
			else
			{
				sbfpValue = SBFP_POS_INF;
			}

			status = 1;
		}
	}

	//
	// Determine if value needs to be denormalized:
	//
	bool denormalize  = false;

	if (status == 0)
	{
		if (dblValueWhole == 0 &&
			(dblValue - (long)dblValue) < (((double)((1 << 10) + 1) / (1 << 10)) / (1 << (SBFP_BIAS - 1))))
		{
			denormalize = true;
		}
	}

	//
	// Extract expo and frac:
	//
	if (status == 0)
	{
		if (denormalize)
		{
			sbfpExpo = 0;
			sbfpFrac = extract_frac(dblValue * (1 << (SBFP_BIAS - 1)));
		}
		else
		{
			int E = 0;

			while (!(dblValue < 2 && dblValue >= 1))
			{
				dblValue /= 2;
				++E;
			}

			sbfpExpo = E + SBFP_BIAS;

			sbfpFrac = extract_frac(dblValue);
		}
	}

	//
	// Concatenate sign, expo and frac to the sbfp value, and return:
	//
	if (status == 0)
	{
		sbfpValue += sbfpSign;

		sbfpValue <<= SBFP_BIT_COUNT_EXPO;
		sbfpValue += sbfpExpo;

		sbfpValue <<= SBFP_BIT_COUNT_FRAC;
		sbfpValue += sbfpFrac;
	}

	return sbfpValue;
}

//
// Converts a given sbfp_t value to a double value.
//
// [in] sbfpValue - the sbfp_t value to be converted
//
// Returns the converted value.
//
double sbfp_to_double(sbfp_t sbfpValue)
{
	int    status   = 0;
	double dblValue = 0.0;

	//
	// Extract frac, expo and sign:
	//
	int sbfpFrac = 0;
	int sbfpExpo = 0;
	int sbfpSign = 0;

	if (status == 0)
	{
		sbfpFrac = sbfpValue & ((1 << SBFP_BIT_COUNT_FRAC) - 1);
		sbfpValue >>= SBFP_BIT_COUNT_FRAC;

		sbfpExpo = sbfpValue & ((1 << SBFP_BIT_COUNT_EXPO) - 1);
		sbfpValue >>= SBFP_BIT_COUNT_EXPO;

		sbfpSign = sbfpValue & ((1 << SBFP_BIT_COUNT_SIGN) - 1);
	}

	//
	// Determine if sbfp value is infinity or NaN:
	//
	if (status == 0)
	{
		if (sbfpExpo == ((1 << SBFP_BIT_COUNT_EXPO) - 1))
		{
			if (sbfpFrac == 0)
			{
				if (sbfpSign == 0)
				{
					dblValue = DOUBLE_POS_INF;
				}
				else
				{
					dblValue = DOUBLE_NEG_INF;
				}
			}
			else
			{
				dblValue = DOUBLE_NAN;
			}

			status = 1;
		}
	}

	//
	// Calculate double value and return:
	//
	if (status == 0)
	{
		int    E = 0;
		double M = 0.0;

		if (sbfpExpo == 0)
		{
			E = 1 - SBFP_BIAS;
			M = (double)sbfpFrac / (1 << SBFP_BIT_COUNT_FRAC);
		}
		else
		{
			E = sbfpExpo - SBFP_BIAS;
			M = 1 + ((double)sbfpFrac / (1 << SBFP_BIT_COUNT_FRAC));
		}

		if (E < 0)
		{
			dblValue = M / (1 << ((-1) * E));
		}
		else
		{
			dblValue = M * (1 << E);
		}

		if (sbfpSign == 1)
		{
			dblValue *= -1;
		}
	}

	return dblValue;
}

//
// Multiplies two special sbfp values.
//
// [in] sbfpValue1 - the multiplicand
// [in] sbfpValue2 - the multiplier
//
// Returns the product.
//
sbfp_t handle_special_mul(sbfp_t sbfpValue1, sbfp_t sbfpValue2)
{
	sbfp_t sbfpProduct = 0;

	switch (sbfpValue1)
	{
		case SBFP_POS_INF:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpProduct = SBFP_POS_INF;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpProduct = SBFP_NEG_INF;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpProduct = SBFP_NAN;
			}
			else
			{
				sbfpProduct = SBFP_POS_INF;
			}
		}

		case SBFP_NEG_INF:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpProduct = SBFP_NEG_INF;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpProduct = SBFP_POS_INF;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpProduct = SBFP_NAN;
			}
			else
			{
				sbfpProduct = SBFP_NEG_INF;
			}
		}

		case SBFP_NAN:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpProduct = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpProduct = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpProduct = SBFP_NAN;
			}
			else
			{
				sbfpProduct = SBFP_NAN;
			}
		}

		default:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpProduct = SBFP_POS_INF;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpProduct = SBFP_NEG_INF;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpProduct = SBFP_NAN;
			}
			else
			{
				// error: you shouldn't be here
			}
		}
	}

	return sbfpProduct;
}

//
// Multiplies two sbfp values.
//
// [in] sbfpValue1 - the multiplicand
// [in] sbfpValue2 - the multiplier
//
// Returns the product.
//
sbfp_t sbfp_mul(sbfp_t sbfpValue1, sbfp_t sbfpValue2)
{
	int status = 0;

	sbfp_t sbfpProduct = 0;

	//
	// Handle if the sbfp values are infinity or NaN:
	//
	if (status == 0)
	{
		if (sbfpValue1 == SBFP_POS_INF || sbfpValue1 == SBFP_NEG_INF || sbfpValue1 == SBFP_NAN ||
			sbfpValue2 == SBFP_POS_INF || sbfpValue2 == SBFP_NEG_INF || sbfpValue2 == SBFP_NAN)
		{
			sbfpProduct = handle_special_mul(sbfpValue1, sbfpValue2);

			status = 1;
		}
	}

	//
	// Extract the frac, expo and sign of both sbfp values:
	//
	int sbfpFrac1 = 0;
	int sbfpExpo1 = 0;
	int sbfpSign1 = 0;

	int sbfpFrac2 = 0;
	int sbfpExpo2 = 0;
	int sbfpSign2 = 0;

	if (status == 0)
	{
		//
		// First sbfp value:
		//
		sbfpFrac1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_FRAC) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_FRAC;

		sbfpExpo1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_EXPO) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_EXPO;

		sbfpSign1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_SIGN) - 1);

		//
		// Second sbfp value:
		//
		sbfpFrac2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_FRAC) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_FRAC;

		sbfpExpo2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_EXPO) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_EXPO;

		sbfpSign2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_SIGN) - 1);
	}

	//
	// Calculate individual variables for multiplying:
	//
	int    E1 = 0;
	int    E2 = 0;
	double M1 = 0.0;
	double M2 = 0.0;

	if (status == 0)
	{
		//
		// First sbfp value:
		//
		if (sbfpExpo1 == 0)
		{
			E1 = 1 - SBFP_BIAS;
			M1 = (double)sbfpFrac1 / (1 << SBFP_BIT_COUNT_FRAC);
		}
		else
		{
			E1 = sbfpExpo1 - SBFP_BIAS;
			M1 = 1 + ((double)sbfpFrac1 / (1 << SBFP_BIT_COUNT_FRAC));
		}

		//
		// Second sbfp value:
		//
		if (sbfpExpo2 == 0)
		{
			E2 = 1 - SBFP_BIAS;
			M2 = (double)sbfpFrac2 / (1 << SBFP_BIT_COUNT_FRAC);
		}
		else
		{
			E2 = sbfpExpo2 - SBFP_BIAS;
			M2 = 1 + ((double)sbfpFrac2 / (1 << SBFP_BIT_COUNT_FRAC));
		}
	}

	//
	// Multiply the values and return:
	//
	if (status == 0)
	{
		int    S = sbfpSign1 ^ sbfpSign2;
		int    E = E1 + E2;
		double M = M1 * M2;
		double sbfpDblProduct = 0.0;

		if (E < 0)
		{
			sbfpDblProduct = M / (1 << ((-1) * E));
		}
		else
		{
			sbfpDblProduct = M * (1 << E);
		}

		if (S == 1)
		{
			sbfpDblProduct *= -1;
		}

		sbfpProduct = double_to_sbfp(sbfpDblProduct);
	}

	return sbfpProduct;
}

//
// Adds two special sbfp values.
//
// [in] sbfpValue1 - the augend
// [in] sbfpValue2 - the addend
//
// Returns the sum.
//
sbfp_t handle_special_add(sbfp_t sbfpValue1, sbfp_t sbfpValue2)
{
	sbfp_t sbfpSum = 0;

	switch (sbfpValue1)
	{
		case SBFP_POS_INF:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpSum = SBFP_POS_INF;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpSum = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpSum = SBFP_NAN;
			}
			else
			{
				sbfpSum = SBFP_POS_INF;
			}
		}

		case SBFP_NEG_INF:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpSum = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpSum = SBFP_NEG_INF;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpSum = SBFP_NAN;
			}
			else
			{
				sbfpSum = SBFP_NEG_INF;
			}
		}

		case SBFP_NAN:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpSum = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpSum = SBFP_NAN;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpSum = SBFP_NAN;
			}
			else
			{
				sbfpSum = SBFP_NAN;
			}
		}

		default:
		{
			if (sbfpValue2 == SBFP_POS_INF)
			{
				sbfpSum = SBFP_POS_INF;
			}
			else if (sbfpValue2 == SBFP_NEG_INF)
			{
				sbfpSum = SBFP_NEG_INF;
			}
			else if (sbfpValue2 == SBFP_NAN)
			{
				sbfpSum = SBFP_NAN;
			}
			else
			{
				// error: you shouldn't be here
			}
		}
	}

	return sbfpSum;
}

//
// Adds two sbfp values.
//
// [in] sbfpValue1 - the augend
// [in] sbfpValue2 - the addend
//
// Returns the sum.
//
sbfp_t sbfp_add(sbfp_t sbfpValue1, sbfp_t sbfpValue2)
{
	int status = 0;

	sbfp_t sbfpSum = 0;

	//
	// Handle if the sbfp values are infinity or NaN:
	//
	if (status == 0)
	{
		if (sbfpValue1 == SBFP_POS_INF || sbfpValue1 == SBFP_NEG_INF || sbfpValue1 == SBFP_NAN ||
			sbfpValue2 == SBFP_POS_INF || sbfpValue2 == SBFP_NEG_INF || sbfpValue2 == SBFP_NAN)
		{
			sbfpSum = handle_special_add(sbfpValue1, sbfpValue2);

			status = 1;
		}
	}

	//
	// Extract the frac, expo and sign of both sbfp values:
	//
	int sbfpFrac1 = 0;
	int sbfpExpo1 = 0;
	int sbfpSign1 = 0;

	int sbfpFrac2 = 0;
	int sbfpExpo2 = 0;
	int sbfpSign2 = 0;

	if (status == 0)
	{
		//
		// First sbfp value:
		//
		sbfpFrac1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_FRAC) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_FRAC;

		sbfpExpo1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_EXPO) - 1);
		sbfpValue1 >>= SBFP_BIT_COUNT_EXPO;

		sbfpSign1 = sbfpValue1 & ((1 << SBFP_BIT_COUNT_SIGN) - 1);

		//
		// Second sbfp value:
		//
		sbfpFrac2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_FRAC) - 1);
		sbfpValue2 >>= SBFP_BIT_COUNT_FRAC;

		sbfpExpo2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_EXPO) - 1);
		sbfpValue2 >>= SBFP_BIT_COUNT_EXPO;

		sbfpSign2 = sbfpValue2 & ((1 << SBFP_BIT_COUNT_SIGN) - 1);
	}

	//
	// Compute individual variables for adding:
	//
	int    S1 = 1;
	int    S2 = 1;
	int    E1 = 0;
	int    E2 = 0;
	double M1 = 0.0;
	double M2 = 0.0;

	if (status == 0)
	{
		//
		// First sbfp value:
		//
		if (sbfpSign1 == 1)
		{
			S1 = -1;
		}

		if (sbfpExpo1 == 0)
		{
			E1 = 1 - SBFP_BIAS;
			M1 = (double)sbfpFrac1 / (1 << SBFP_BIT_COUNT_FRAC);
		}
		else
		{
			E1 = sbfpExpo1 - SBFP_BIAS;
			M1 = 1 + ((double)sbfpFrac1 / (1 << SBFP_BIT_COUNT_FRAC));
		}

		//
		// Second sbfp value:
		//
		if (sbfpSign2 == 1)
		{
			S2 = -1;
		}

		if (sbfpExpo2 == 0)
		{
			E2 = 1 - SBFP_BIAS;
			M2 = (double)sbfpFrac2 / (1 << SBFP_BIT_COUNT_FRAC);
		}
		else
		{
			E2 = sbfpExpo2 - SBFP_BIAS;
			M2 = 1 + ((double)sbfpFrac2 / (1 << SBFP_BIT_COUNT_FRAC));
		}
	}

	//
	// Compute sum:
	//
	int    E          = 0;
	double M          = 0.0;
	double sbfpDblSum = 0.0;

	if (E1 > E2)
	{
		E = E2;
		M1 *= (1 << (E1 - E2));
	}
	else
	{
		E = E1;
		M2 *= (1 << (E2 - E1));
	}

	M = (S1 * M1) + (S2 * M2);

	if (E < 0)
	{
		sbfpDblSum = M / (1 << ((-1) * E));
	}
	else
	{
		sbfpDblSum = M * (1 << E);
	}

	sbfpSum = double_to_sbfp(sbfpDblSum);

	return sbfpSum;
}
