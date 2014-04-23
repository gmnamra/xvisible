/*
 *
 *$Id $
 *$Log$
 *Revision 1.1  2004/10/07 19:37:30  arman
 *numeric constants
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include "rc_math.h"
#include "rc_numeric.h"


const bool rcNumericTraits<bool>::Zero = false;
const bool rcNumericTraits<bool>::One = true;

const unsigned char rcNumericTraits<unsigned char>::Zero = 0;
const unsigned char rcNumericTraits<unsigned char>::One = 1;

const signed char rcNumericTraits<signed char>::Zero = 0;
const signed char rcNumericTraits<signed char>::One = 1;

const char rcNumericTraits<char>::Zero = 0;
const char rcNumericTraits<char>::One = 1;

const unsigned short rcNumericTraits<unsigned short>::Zero = 0;
const unsigned short rcNumericTraits<unsigned short>::One = 1;

const short rcNumericTraits<short>::Zero = 0;
const short rcNumericTraits<short>::One = 1;

const unsigned int rcNumericTraits<unsigned int>::Zero = 0;
const unsigned int rcNumericTraits<unsigned int>::One = 1;

const int rcNumericTraits<int>::Zero = 0;
const int rcNumericTraits<int>::One = 1;

const unsigned long rcNumericTraits<unsigned long>::Zero = 0;
const unsigned long rcNumericTraits<unsigned long>::One = 1;

const long rcNumericTraits<long>::Zero = 0UL;
const long rcNumericTraits<long>::One = 1UL;

const float rcNumericTraits<float>::Zero = 0.0F;
const float rcNumericTraits<float>::One = 1.0F;

const double rcNumericTraits<double>::Zero = 0.0;
const double rcNumericTraits<double>::One = 1.0;

const long double rcNumericTraits<long double>::Zero = 0.0;
const long double rcNumericTraits<long double>::One = 1.0;
