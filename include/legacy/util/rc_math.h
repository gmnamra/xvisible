/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.26  2005/11/07 17:32:09  arman
 *cell lineage iv and bug fix
 *
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcMATH_H
#define __rcMATH_H

#include "rc_types.h"
#include "rc_limits.h"
#include <cmath>

using namespace std;

//! Returns an index to the left of two interpolation values.
 /*! \a begin points to an array containing a sequence of values
     sorted in ascending order. The length of the array is \a len.
     If \a val is lower than the first element, 0 is returned.
     If \a val is higher than the last element, \a len-2
     is returned. Else, the index of the largest element smaller
     than \a val is returned. */
template<typename T> inline int interpol_left
(const T *begin, int len, const T &val)
{
  const T *end = begin+len;
  const T *iter = std::lower_bound (begin, end, val);
  if (iter==begin) return 0;
  if (iter==end) return len-2;
  return (iter-begin)-1;
}

 //! Returns an index to the nearest interpolation value.
 /*! \a begin points to an array containing a sequence of values
     sorted in ascending order. The length of the array is \a len.
     If \a val is lower than the first element, 0 is returned.
     If \a val is higher than the last element, \a len-1 is returned.
     Else, the index of the nearest element within the sequence of
     values is returned. */
template<typename T> inline int interpol_nearest
(const T *begin, int len, const T &val)
{
  int left = interpol_left(begin, len, val);
  T delleft = val-(*(begin+left));
  T delright = (*(begin+left+1))-val;
  if (delright<0) return left+1;
  return (delright<delleft) ? (left+1) : left;
}

// Some other constants
#define rkPI (3.1415926535897932384626433832795)
#define rk2PI (2*rkPI)
#define rkRadian (rkPI / 180.0)

// The usual Min and Max
#define rmMin(a,b) ((a) < (b) ? (a) : (b))
#define rmMax(a,b) ((a) > (b) ? (a) : (b))
#define rmABS(a) ((a) < 0 ? (-(a)) : (a))
#define rmSquare(a) ((a) * (a))


#define rmMedOf3(a, b, c)                                       \
((a) > (b) ? ((a) < (c) ? (a) : ((b) < (c) ? (c) : (b))) :    \
 ((a) > (c) ? (a) : ((b) < (c) ? (b) : (c))))

#define rmMaxOf3(a, b, c) (rmMax(rmMax(a, b), c))

#define rmMinOf3(a, b, c) (rmMin(rmMin(a, b), c))

#define rmSGN(a) (((a) < 0) ? -1 : 1)

#define rmLimit(a,b,c) (rmMin((rmMax((a),(c))),(b)))

bool rfIsPowerOf2 (uint32);

template <class T>
inline T rfSqr(T x) {return x*x;}

/** \brief Check if val1 and val2 are equals to an epsilon extent
 * \param[in] val1 first number to check
 * \param[in] val2 second number to check
 * \param[in] eps epsilon
 * \return true if val1 is equal to val2, false otherwise.
 */
template<typename T> bool
equal (T val1, T val2, T eps = std::numeric_limits<T>::epsilon() )
{
    return (fabs (val1 - val2) < eps);
}

inline bool real_equal(double x,double y,double epsilon = 1.e-15  )
{return std::abs (x - y) <= epsilon;}

inline bool real_equal(float x,float y,float epsilon = 1.e-10  )
{return std::abs (x - y) <= epsilon;}


template<class T>
T rfSigmoid(T x)
{
  return ((T) 1.0)/((T) 1.0 + exp(-x));
}

template<class T>
T rfNormPositiveSigmoid(T x)
{
  // Transform x [0 1] to [-1 1]
  T xp = x + x + (T) (-1);
  xp = rfSigmoid (xp);

  // Transform [-1 1] back to [0 1]
  xp = (xp + 1) / (T) 2;

  return xp;
}


template<class T>
bool rfQuadradic (T a, T b, T c, T& pos, T& neg)
{
  T det = rmSquare (b) - (4 * a * c);

  if (a == T (0))
    {
      pos = -c / b;
      neg = -c / b;
      return true;
    }

  if (det >= 0)
    {
      pos = (-b + sqrt (det)) / (2*a);
      neg = (-b - sqrt (det)) / (2*a);
      return true;
    }

  return false;
}


// Fix point Fast vector support using cordic
const int32 rcFxPrecision = 16;
int32 rfFxAtan2(int32 x, int32 y);
void rfFxUnitVec(int32 *cos, int32 *sin, int32 theta);
void rfFxRotate(int32 *argx, int32 *argy, int32 theta);
void rfFxPolarize(int32 *argx, int32 *argy);

// Rounding Templates and Specialization

template <class S, class D>
inline D rfRoundPlus(S val, D) { return D(val); }

template <class S, class D>
inline D rfRound(S val, D) { return D(val); }

inline int32  rfRoundPlus(double val, int32)  { return int32(val + 0.5); }
inline int32  rfRoundPlus(float val,  int32)  { return int32(val + 0.5); }

inline int32  rfRoundNeg(double val, int32)  { return int32(val -  0.5); }
inline int32  rfRoundNeg(float val,  int32)  { return int32(val - 0.5); }

inline int32 rfRound(double val, int32 Dummy)
{
  return val < 0 ? -rfRoundPlus(-val, Dummy) : rfRoundPlus(val, Dummy);
}

inline int32 rfRound(float val, int32 Dummy)
{
  return val < 0 ? -rfRoundPlus(-val, Dummy) : rfRoundPlus(val, Dummy);
}

// Integer versions of ceil and div
inline int32 rfiDiv (int32 x, int32 y)
{
  return (x >= 0 ? x : x - abs(y) + 1) / y;
}

inline int32 rfiFloor (int x, int y)
{
  return rfiDiv (x, y) * y;
}

inline int32 rfiCeil (int x, int y)
{
  return (x < 0 ? x : x + abs(y) - 1) / y * y;
}


/*
 * Angle Support
 *
 * Usage: transformation <-> binary angles, radians, and degrees
 */


class rcAngle8;
class rcAngle16;
class rcRadian;
class rcDegree;
class rcFixed16;


// Allow only explicit constructions.
// Implement 1D fixed units

#define rmMakeUnit(T, U)					\
public:								\
  U () : val (0) {}							\
  explicit U (T a) : val(a) {}					\
								\
  U operator + (U a) const { return U (val + a.val);}		\
  U operator - (U a) const { return U (val - a.val);}		\
								\
  U operator * (T a) const { return U (val * a);}			\
  U operator / (T a) const { return U (val / a);}			\
								\
  U operator - () const { return U (-val);}			\
								\
  T operator * (U a) const { return val * a.val;}			\
  T operator / (U a) const { return val / a.val;}			\
								\
  friend U operator * (T a, U b) { return U (a * b.val);}		\
								\
  double Double () const { return (double) val;}		\
  T      basic    () const { return          val;}		\
  U& operator  = (T a) { val  = a; return *this;}			\
  U& operator *= (T a) { val *= a; return *this;}			\
  U& operator /= (T a) { val /= a; return *this;}			\
								\
  U& operator *= (U a) { val *= a.val; return *this;}		\
  U& operator /= (U a) { val /= a.val; return *this;}		\
								\
  U& operator += (U a) { val += a.val; return *this;}		\
  U& operator -= (U a) { val -= a.val; return *this;}		\
								\
  bool operator == (U a) const { return val == a.val;}		\
  bool operator != (U a) const { return val != a.val;}		\
  bool operator <  (U a) const { return val <  a.val;}		\
  bool operator <= (U a) const { return val <= a.val;}		\
  bool operator >  (U a) const { return val >  a.val;}		\
  bool operator >= (U a) const { return val >= a.val;}		\
								\
								\
private:							\
  T val


/*
 * Classes reprenting Angular units
 *
*/

class rcRadian
{
  rmMakeUnit (double, rcRadian);

public:
  inline rcRadian (rcDegree);
  inline rcRadian (rcAngle8);
  inline rcRadian (rcAngle16);

  rcRadian norm () const;	// result range is [0-2PI)
  rcRadian normSigned () const;	// result range is [-PI, PI)
};

bool real_equal (rcRadian x, rcRadian y, rcRadian epsilon = rcRadian(1.e-15));

class rcDegree
{
  rmMakeUnit (double, rcDegree);

public:
  inline rcDegree (rcRadian);
  inline rcDegree (rcAngle8);
  inline rcDegree (rcAngle16);

  rcDegree norm () const;	// result range is [0-360)
  rcDegree normSigned () const;	// result range is [-180, 180)
};

class rcAngle8
{
  rmMakeUnit (uint8, rcAngle8);

public:
  inline rcAngle8 (rcRadian);
  inline rcAngle8 (rcDegree);
  inline rcAngle8 (rcAngle16);
  inline static const rcAngle8 piOverTwo () { return rcAngle8 (rcUINT8_MAX / 4); }
};

class rcAngle16
{
  rmMakeUnit (uint16, rcAngle16);

public:
  inline rcAngle16 (rcRadian);
  inline rcAngle16 (rcDegree);
  inline rcAngle16 (rcAngle8);
  inline static const rcAngle8 piOverTwo () { return rcAngle16 (rcUINT16_MAX / 4); }
};


// Classes for Fixed Pixel Positions

const int32 rcFixed16Precision = 8;

class rcFixed16
{
  rmMakeUnit (int16, rcFixed16);

public:
  inline rcFixed16 (double d) : val (int16 (d * (1<<rcFixed16Precision))) {}

  inline rcFixed16 (float d) : val (int16 (d * (1<<rcFixed16Precision))) {}

  inline rcFixed16 (int32 d) : val (int16 (d * (1<<rcFixed16Precision))) {}

  float real () const;
};


// Actual class methods: created from defines

#define rmAngularUnitIntRange(T) (2. * ((unsigned)1 << (8 * sizeof(T) - 1)))

#define rmAngularUnitFloatFromAny(dst, src, dstRange, srcRange)			\
inline dst::dst (src x)								\
{										\
  val = x.Double() * ((dstRange) / (srcRange));					\
}

#define rmAngularUnitIntFromFloat(dst, src, dstT, srcRange)				\
inline dst::dst (src x)								\
{										\
  val = (dstT) (int) floor(x.Double() *	(rmAngularUnitIntRange(dstT)/srcRange) + 0.5);\
}

#define rmAngularUnitIntFromSmaller(dst, src, dstT, srcT)				\
inline dst::dst (src x)								\
{										\
  val = (dstT) (x.basic() << (8 * (sizeof(dstT) - sizeof(srcT))));		\
}

#define rmAngularUnitIntFromBigger(dst, src, dstT, srcT)				\
inline dst::dst (src x)								\
{										\
  val = (dstT) ((x.basic() + (1 << (8 * (sizeof(srcT) - sizeof(dstT)) - 1)))	\
	      >> (8 * (sizeof(srcT) - sizeof(dstT))));				\
}

#define rmAngularUnitFloatFloat(aType, bType, aRange, bRange)			\
  rmAngularUnitFloatFromAny (aType, bType, aRange, bRange)				\
  rmAngularUnitFloatFromAny (bType, aType, bRange, aRange)

#define rmAngularUnitFloatInt(fUnit, bUnit, fRange, bType)				\
  rmAngularUnitFloatFromAny (fUnit, bUnit, fRange, rmAngularUnitIntRange(bType))		\
  rmAngularUnitIntFromFloat (bUnit, fUnit, bType, fRange)

#define rmAngularUnitIntInt(smallUnit, bigUnit, smallType, bigType)			\
  rmAngularUnitIntFromSmaller (bigUnit, smallUnit, bigType, smallType)		\
  rmAngularUnitIntFromBigger  (smallUnit, bigUnit, smallType, bigType)

rmAngularUnitFloatFloat (rcRadian, rcDegree, rk2PI, 360.)

rmAngularUnitFloatInt (rcRadian, rcAngle8 , rk2PI, uint8 )
rmAngularUnitFloatInt (rcRadian, rcAngle16, rk2PI, uint16)
rmAngularUnitFloatInt (rcDegree, rcAngle8 ,  360., uint8 )
rmAngularUnitFloatInt (rcDegree, rcAngle16,  360., uint16)

rmAngularUnitIntInt (rcAngle8, rcAngle16, uint8, uint16)

// Define trig functions for the new classes

inline rcRadian arccos (double x) { return rcRadian (acos (x));}
inline rcRadian arcsin (double x) { return rcRadian (asin (x));}
inline rcRadian arctan (double x) { return rcRadian (atan (x));}
inline rcRadian arctan (double y, double x) { return rcRadian(atan2(y, x));}

inline double sin (rcRadian x) { return sin (x.Double());}
inline double cos (rcRadian x) { return cos (x.Double());}
inline double tan (rcRadian x) { return tan (x.Double());}

inline rcRadian abs (rcRadian x) { return rcRadian (rmABS (x.Double()));}
inline rcDegree abs (rcDegree x) { return rcDegree (rmABS (x.Double()));}

inline float rcFixed16::real () const { return val/((float) (1<<rcFixed16Precision)); }

////////////////////////////////////////////////////////////////////////////////
//
//	log2max
//
//	This function computes the ceiling of log2(n).  For example:
//
//	log2max(7) = 3
//	log2max(8) = 3
//	log2max(9) = 4
//
////////////////////////////////////////////////////////////////////////////////
int32 rfLog2max(int32 n);

int32 rfGCDeuclid(int32 a, int32 b);


// reimplemented because ISO C99 function "nearbyint",

template < typename T >
int32 rcNearbyint(const T& d)
{
  int32 z = int32 (d);
  T frac = d - z;

  rmAssert (abs(frac) < T(1.0));

  if (frac > 0.5)
    ++z;
  else if (frac < -0.5)
    --z;
  else if (frac == 0.5 && (z&1) != 0) // NB: We also need the round-to-even rule.
    ++z;
  else if (frac == -0.5 && (z&1) != 0)
    --z;

  rmAssert (abs(T(z) - d) < T(0.5) ||
                 (abs(T(z) - d) == T(0.5) && ((z&1) == 0)));
  return z;
}



#endif /* __rcMATH_H */
