/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2004/10/06 21:21:01  arman
 *numeric traits
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcNUMERIC_H
#define __rcNUMERIC_H

#include <rc_types.h>

#undef min
#undef max

/** \class rcNumericTraits
 * \brief Define additional traits for native types such as int or float.
 *
 * rcNumericTraits is used to extend the traits associated with native types
 * such as float, char, int, and so on. These traits are extensions of the
 * standard <numeric_limits> defined by the C++ compilers. Some of the added
 * traits include minimum and maximum value; accumulation type; etc.
 *
 * \ingroup DataRepresentation
 */
template <class T>
class rcNumericTraits
{
public:
  /** Return the type of this native type. */
  typedef T ValueType; 

  /** Return the type that can be printed. */
  typedef T PrintType; 

  /** Return value of abs(). */
  typedef T AbsType; 

  /** Accumulation of addition and multiplication. */
  typedef double AccumulateType; 

  /** Additive identity. */
  static const T Zero;

  /** Multiplicative identity. */
  static const T One;

  /** Is a given value positive? **/
  static bool IsPositive(T val) { return val > Zero; }

  /** Is a given value nonpositive? **/
  static bool IsNonpositive(T val) { return val <= Zero; }

  /** Is a given value negative? **/
  static bool IsNegative(T val) { return val < Zero; }

  /** Is a given value nonnegative? **/
  static bool IsNonnegative(T val) { return val >= Zero; }
};

/** \class rcNumericTraits<bool>
 * \brief Define traits for type bool.
 * 
 * \ingroup DataRepresentation
 */

template <>
class rcNumericTraits<bool>
{
public:
  typedef bool ValueType;
  typedef bool PrintType;
  typedef unsigned char AbsType;
  typedef unsigned char AccumulateType;
  static const bool  Zero;
  static const bool  One;

  static bool IsPositive(bool val) { return val; }
  static bool IsNonpositive(bool val) { return !val; }
  static bool IsNegative(bool /* val */) { return false; }
  static bool IsNonnegative(bool /*val*/) {return true; }
};

/** \class rcNumericTraits<char>
 * \brief Define traits for type char.
 * NOTE: char is not guarenteed to be signed. On SGI's, thge default is unsigned
 */
template <>
class rcNumericTraits<char> 
{
public:
  typedef char ValueType;
  typedef int PrintType;
  typedef unsigned char AbsType;
  typedef short AccumulateType;
  typedef double RealType;
  static const char  Zero;
  static const char  One;

  static bool IsPositive(char val) { return val > Zero; }
  static bool IsNonpositive(char val) { return val <= Zero; }
  static bool IsNegative(char val) { return val < Zero; }
  static bool IsNonnegative(char val) {return val >= Zero; }
};

/** \class rcNumericTraits<char>
 * \brief Define traits for type char.
 * NOTE: char is not guarenteed to be signed. On SGI's, thge default is unsigned
 */
template <>
class rcNumericTraits<signed char>
{
public:
  typedef signed char ValueType;
  typedef int PrintType;
  typedef unsigned char AbsType;
  typedef short AccumulateType;
  typedef double RealType;
  static const signed char  Zero;
  static const signed char  One;

  static bool IsPositive(signed char val) { return val > Zero; }
  static bool IsNonpositive(signed char val) { return val <= Zero; }
  static bool IsNegative(signed char val) { return val < Zero; }
  static bool IsNonnegative(signed char val) {return val >= Zero; }
};

/** \class rcNumericTraits<unsigned char>
 * \brief Define traits for type unsigned char.
 * \ingroup DataRepresentation
 */
template <>
class rcNumericTraits<unsigned char>
{
public:
  typedef unsigned char ValueType;
  typedef int PrintType;
  typedef unsigned char AbsType;
  typedef unsigned short AccumulateType;
  typedef double RealType;
  static const unsigned char  Zero;
  static const unsigned char  One;

  static bool IsPositive(unsigned char val) { return val != Zero; }
  static bool IsNonpositive(unsigned char val) { return val == Zero; }
  static bool IsNegative(unsigned char /* val */) { return false; }
  static bool IsNonnegative(unsigned char /*val */) {return true; }
};

/** \class rcNumericTraits<short>
 * \brief Define traits for type short.
 */
template <>
class rcNumericTraits<short>
{
public:
  typedef short ValueType;
  typedef short PrintType;
  typedef unsigned short AbsType;
  typedef int AccumulateType;
  typedef double RealType;
  static const short  Zero;
  static const short  One;

  static bool IsPositive(short val) { return val > Zero; }
  static bool IsNonpositive(short val) { return val <= Zero; }
  static bool IsNegative(short val) { return val < Zero; }
  static bool IsNonnegative(short val) {return val >= Zero; }
};

/** \class rcNumericTraits<unsigned short>
 * \brief Define traits for type unsigned short.
 * \ingroup DataRepresentation
 */
template <>
class rcNumericTraits<unsigned short>
{
public:
  typedef unsigned short ValueType;
  typedef unsigned short PrintType;
  typedef unsigned short AbsType;
  typedef unsigned int AccumulateType;
  typedef double RealType;
  static const unsigned short  Zero;
  static const unsigned short  One;

  static unsigned short IsPositive(unsigned short val) { return val != Zero; }
  static bool IsNonpositive(unsigned short val) { return val == Zero; }
  static bool IsNegative(unsigned short/* val*/) { return false; }
  static bool IsNonnegative(unsigned short /*val*/) {return true; }
};

/** \class rcNumericTraits<int>
 * \brief Define traits for type int.
 */
template <>
class rcNumericTraits<int>
{
public:
  typedef int ValueType;
  typedef int PrintType;
  typedef unsigned int AbsType;
  typedef long AccumulateType;
  typedef double RealType;
  static const int  Zero;
  static const int  One;

  static bool IsPositive(int val) { return val > Zero; }
  static bool IsNonpositive(int val) { return val <= Zero; }
  static bool IsNegative(int val) { return val < Zero; }
  static bool IsNonnegative(int val) {return val >= Zero; }
};

/** \class rcNumericTraits<unsigned int>
 * \brief Define traits for type unsigned int.
 * \ingroup DataRepresentation
 */
template <>
class rcNumericTraits<unsigned int>
{
public:
  typedef unsigned int ValueType;
  typedef unsigned int PrintType;
  typedef unsigned int AbsType;
  typedef unsigned int AccumulateType;
  typedef double RealType;
  static const unsigned int  Zero;
  static const unsigned int  One;

  static bool IsPositive(unsigned int val) { return val != Zero; }
  static bool IsNonpositive(unsigned int val) { return  val == Zero; }
  static bool IsNegative(unsigned int /*val*/) { return false; }
  static bool IsNonnegative(unsigned int /*val*/) {return true; }
};

/** \class rcNumericTraits<long>
 * \brief Define traits for type long.
 * \ingroup DataRepresentation
 */
template <>
class rcNumericTraits<long> 
{
public:
  typedef long ValueType;
  typedef long PrintType;
  typedef unsigned long AbsType;
  typedef long AccumulateType;
  typedef double RealType;
  static const long  Zero;
  static const long  One;

  static bool IsPositive(long val) { return val > Zero; }
  static bool IsNonpositive(long val) { return val <= Zero; }
  static bool IsNegative(long val) { return val < Zero; }
  static bool IsNonnegative(long val) {return val >= Zero; }
};

/** \class rcNumericTraits<unsigned long>
 * \brief Define traits for type unsigned long.
 * \ingroup DataRepresentation 
 */
template <>
class rcNumericTraits<unsigned long>
{
public:
  typedef unsigned long ValueType;
  typedef unsigned long PrintType;
  typedef unsigned long AbsType;
  typedef unsigned long AccumulateType;
  typedef double RealType;
  static const unsigned long  Zero;
  static const unsigned long  One;

  static bool IsPositive(unsigned long val) { return val != Zero; }
  static bool IsNonpositive(unsigned long val) { return val == Zero; }
  static bool IsNegative(unsigned long) { return false; }
  static bool IsNonnegative(unsigned long) {return true; }
};

/** \class rcNumericTraits<float>
 * \brief Define traits for type float.
 * \ingroup DataRepresentation
 */
template <>
class rcNumericTraits<float>
{
public:
  typedef float ValueType;
  typedef float PrintType;
  typedef float AbsType;
  typedef double AccumulateType;
  typedef double RealType;
  static const float  Zero;
  static const float  One;
  static const float  epsilon () { return rcFLT_EPSILON; }
  static bool IsPositive(float val) { return val > Zero; }
  static bool IsNonpositive(float val) { return val <= Zero; }
  static bool IsNegative(float val) { return val < Zero; }
  static bool IsNonnegative(float val) {return val >= Zero; }
};

/** \class rcNumericTraits<double>
 * \brief Define traits for type double.
 * \ingroup DataRepresentation 
 */
template <>
class rcNumericTraits<double>
{
public:
  typedef double ValueType;
  typedef double PrintType;
  typedef double AbsType;
  typedef double AccumulateType;
  typedef double RealType;
  static const double  Zero;
  static const double  One;

  static bool IsPositive(double val) { return val > Zero; }
  static bool IsNonpositive(double val) { return val <= Zero; }
  static bool IsNegative(double val) { return val < Zero; }
  static bool IsNonnegative(double val) {return val >= Zero; }
};

/** \class rcNumericTraits<long double>
 * \brief Define traits for type long double.
 * \ingroup DataRepresentation 
 */
template <>
class rcNumericTraits<long double>
{
public:
  typedef long double ValueType;
  typedef long double PrintType;
  typedef long double AbsType;
  typedef long double AccumulateType;
  typedef long double RealType;
  static const long double  Zero;
  static const long double  One;

  static bool IsPositive(long double val) { return val > Zero; }
  static bool IsNonpositive(long double val) { return val <= Zero; }
  static bool IsNegative(long double val) { return val < Zero; }
  static bool IsNonnegative(long double val) {return val >= Zero; }
};

#endif /* __rcNUMERIC_H */
