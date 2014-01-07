/*
 *  ut_simd.h
 *  vf
 *
 *  Created by arman on 1/27/09.
 *  Copyright 2009 Reify Corporation. All rights reserved.
 *
 */

#ifndef __UT_SIMD_H
#define __UT_SIMD_H

#include <iostream>
#include <functional>
#include <numeric>

using namespace std;

int ut_simd (std::string& );

// calculate a distance and direction
template<class X>
struct distance1 : public binary_function<X,X,pair<X,int> >
{
	pair<X,int> operator () (X x, X y)
	{
		X d = x*x + y*y;
		
		// assign a quadrant to point
		int quad;
		if (x<0)
		{
			(y<0) ? quad=2 : quad=1;
		}
		else
		{
			(y<0) ? quad=3 : quad=0;
		}
		
		return make_pair(d,quad);
	}
};


#endif /* __UT_SIMD_H */

