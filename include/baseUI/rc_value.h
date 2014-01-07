/******************************************************************************
*	rc_value.h
*
*	This file contains the declaration for a generic value holder,
*	which supports int, float, double, bool, rect and string value types.
*
******************************************************************************/

#ifndef _BASE_RCVALUE_H_
#define _BASE_RCVALUE_H_

#include <string>
#include <iostream>
#include <rc_exception.h>

#include <rc_rect.h>
#include <rc_affine.h>

#include "rc_uitypes.h"

#if WIN32
using namespace std;
#endif


/******************************************************************************
 *	Exceptions
 ******************************************************************************/

// rcNullValueException is thrown when trying to access a null value
class rcNullValueException : public general_exception {
};

// rcConvertValueException is thrown if a value can't be converted
// to a particular type (e.g., string -> int)
class rcConvertValueException : public general_exception {
public:
  rcConvertValueException( const std::string& msg ) throw() : general_exception::general_exception( msg ) { }
};


/******************************************************************************
 *	rcValue class definition
 *
 *	The rcValue class is a generic value holder which supports
 *	int, float, bool, and string value types.
 ******************************************************************************/

class rcValue
{
public:
  // the different types that can be stored
  enum Type { eIntValue = 0,
	      eBoolValue,
	      eFloatValue,
	      eDoubleValue,
	      eStringValue,
	      eRectValue,
	      eAffineRectValue,
	      ePositionValue,
	      eiPairValue,
	      eNullValue };

  // null value (default) constructor
  rcValue();

  // int value constructor
  rcValue( int intValue );

  // bool value constructor
  rcValue( bool boolValue );

  // float value constructor
  rcValue( float floatValue );

  // double value constructor
  rcValue( double doubleValue );

  // string value constructor
  rcValue( const std::string& stringValue );

  // string value constructor (alternate)
  rcValue( const char* stringValue );

  // rect value constructor
  rcValue( const rcRect& rectValue );

  // rect value constructor
  rcValue( const rcAffineRectangle& rectValue );

  // position value constructor
  rcValue( const rcFPair& posValue );

// ipair value constructor
  rcValue( const rcIPair& posValue );

  // return value cast/converted to an int
  int intValue( void ) const;

  // return value cast/converted to a float
  float floatValue( void ) const;

  // return value cast/converted to a double
  double doubleValue( void ) const;

  // return value cast/converted to a bool
  bool boolValue( void ) const;

  // return value cast/converted to a string
  std::string stringValue( void ) const;

  // return value cast/converted to a rect
  rcRect rectValue( void ) const;

 // return value cast/converted to an affineRectangle
  rcAffineRectangle affineValue( void ) const;

  // return value cast/converted to a coordinate pair
  rcFPair positionValue ( void ) const;

  // return value cast/converted to a coordinate pair
  rcIPair ipairValue ( void ) const;

  // returns true if a null value
  bool isNull( void ) const;

  // get the value type.
  Type getType( void ) const;

  // equals support: values are only equal if same type and value
  bool equals( const rcValue& otherValue ) const;

  // cast to integer operator
  operator int() const { return intValue(); }

  // cast to float operator
  operator float() const { return floatValue(); }

  // cast to double operator
  operator double() const { return doubleValue(); }

  // cast to bool operator
  operator bool() const { return boolValue(); }

  // cast to string operator
  operator std::string() const { return stringValue(); }

  // cast to rect operator
  operator rcRect() const { return rectValue(); }

  // cast to affine rect operator
  operator rcAffineRectangle() const { return affineValue(); }

  // cast to position operator
  operator rcFPair() const { return positionValue(); }

  // cast to position operator
  operator rcIPair() const { return ipairValue(); }

  // equals operator
  bool operator == ( const rcValue& value ) const { return equals( value ); }

  // not equals operator
  bool operator != ( const rcValue& value ) const { return !equals( value ); }

private:

  // TODO: improve memory usage
  union
  {
    int			intValue;
    bool		boolValue;
    float		floatValue;
    double      doubleValue;
    float     positionValue [2];
    int        ipairValue [2];
    int        rectValue[4];
    float     affineValue[5];
  } _val;

  std::string	_stringValue;
  Type		_type;

};

// insertion operator
ostream& operator << ( ostream& os, const rcValue& value );

#endif // _BASE_RCVALUE_H_
