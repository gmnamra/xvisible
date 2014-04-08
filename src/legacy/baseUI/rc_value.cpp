/******************************************************************************
*	rc_value.cpp
*
*	This file contains the definition for a generic value holder,
*	which supports int, float, Double, bool, rect and string value types.
*	
******************************************************************************/

#include <exception>
#include <string>
#include <strstream>

#include <rc_value.h>

#if WIN32
using namespace std;
#endif

/******************************************************************************
*	rcValue definition
******************************************************************************/

// null value (default) constructor
rcValue::rcValue()
{
	_type = eNullValue;
	_val.intValue = 0;
}

// int value constructor
rcValue::rcValue( int intValue )
{
	_type = eIntValue;
	_val.intValue = intValue;
}

// bool value constructor
rcValue::rcValue( bool boolValue )
{
	_type = eBoolValue;
	_val.boolValue = boolValue;
}

// float value constructor
rcValue::rcValue( float floatValue )
{
	_type = eFloatValue;
	_val.floatValue = floatValue;
}

// double value constructor
rcValue::rcValue( double doubleValue )
{
	_type = eDoubleValue;
	_val.doubleValue = doubleValue;
}

// string value constructor
rcValue::rcValue( const std::string& stringValue )
{
	_type = eStringValue;
	_stringValue = stringValue;
}

// string value constructor (alternate)
rcValue::rcValue( const char* stringValue )
{
	_type = eStringValue;
	_stringValue = stringValue;
}

// rect value constructor
rcValue::rcValue( const rcRect& rectValue )
{
	_type = eRectValue;
	_val.rectValue[0] = rectValue.x(), _val.rectValue[1] = rectValue.y(), _val.rectValue[2] = rectValue.width(), 
	  _val.rectValue[3] = rectValue.height();
}

// Affine rect value constructor
rcValue::rcValue( const rcAffineRectangle& affValue )
{
  _type = eAffineRectValue;
  _val.affineValue[0] = (float) affValue.origin().x(), _val.affineValue[1] = (float) affValue.origin().y(),
    _val.affineValue[2] = (float) affValue.cannonicalSize().x(),     _val.affineValue[3] = (float) affValue.cannonicalSize().y(), 
    _val.affineValue[4] = (float) affValue.angle().basic ();
}

// position value constructor
rcValue::rcValue( const rcFPair& posValue )
{
	_type = ePositionValue;
	_val.positionValue[0] = posValue.x();
	_val.positionValue[1] = posValue.y();
}

// position value constructor
rcValue::rcValue( const rcIPair& ipairValue )
{
	_type = eiPairValue;
	_val.ipairValue[0] = ipairValue.x();
	_val.ipairValue[1] = ipairValue.y();
}

// return value cast as an int
int rcValue::intValue( void ) const
{
	int i;

	switch (_type)
	{
	case eIntValue:
		return _val.intValue;

	case eFloatValue:
		return (int) _val.floatValue;

    case eDoubleValue:
		return (int) _val.doubleValue;
        
	case eBoolValue:
		return _val.boolValue ? 1 : 0;

	case eStringValue:
		if (sscanf( _stringValue.c_str() , "%d" , &i ) != 1)
			throw rcConvertValueException( "string to int conversion failed" );
		return i;

    case eNullValue:
		throw rcNullValueException();

    case eRectValue:
        throw rcConvertValueException( "rect to int conversion failed" );

    case eAffineRectValue:
        throw rcConvertValueException( "affine rect to int conversion failed" );

    case ePositionValue:
        throw rcConvertValueException( "position to int conversion failed" );

    case eiPairValue:
        throw rcConvertValueException( "ipair to int conversion failed" );
        
	}
	throw rcConvertValueException( "unknown to int conversion failed" );
}

// return value cast as a float
float rcValue::floatValue( void ) const
{
	float f;

	switch (_type)
	{
	case eIntValue:
		return (float) _val.intValue;

	case eFloatValue:
		return _val.floatValue;

    case eDoubleValue:
		return (float) _val.doubleValue;
        
	case eBoolValue:
		return _val.boolValue ? 1.0f : 0.0f;

	case eStringValue:
		if (sscanf( _stringValue.c_str() , "%f" , &f ) != 1)
			throw rcConvertValueException( "string to float conversion failed" );
		return f;

	case eNullValue:
		throw rcNullValueException();
        
    case eRectValue:
        throw rcConvertValueException( "rect to float conversion failed" );

    case eAffineRectValue:
        throw rcConvertValueException( "affine rect to int conversion failed" );

    case ePositionValue:
        throw rcConvertValueException( "position to float conversion failed" );

	case eiPairValue:
        throw rcConvertValueException( "ipair to float conversion failed" );

	}
	throw rcConvertValueException( "unknown to float conversion failed" );
}

// return value cast as a double
double rcValue::doubleValue( void ) const
{
	double f;

	switch (_type)
	{
	case eIntValue:
		return (double) _val.intValue;

	case eFloatValue:
		return (double)_val.floatValue;

    case eDoubleValue:
		return _val.doubleValue;
        
	case eBoolValue:
		return _val.boolValue ? 1.0f : 0.0f;

	case eStringValue:
		if (sscanf( _stringValue.c_str() , "%lf" , &f ) != 1)
			throw rcConvertValueException( "string to double conversion failed" );
		return f;

	case eNullValue:
		throw rcNullValueException();
        
    case eRectValue:
        throw rcConvertValueException( "rect to double conversion failed" );

    case eAffineRectValue:
        throw rcConvertValueException( "affine rect to int conversion failed" );

    case ePositionValue:
        throw rcConvertValueException( "position to double conversion failed" );

	case eiPairValue:
        throw rcConvertValueException( "ipair to double conversion failed" );

	}
	throw rcConvertValueException( "unknown to double conversion failed" );
}

// return value cast as a boolean
bool rcValue::boolValue( void ) const
{
	switch (_type)
	{
	case eIntValue:
		return _val.intValue != 0;

	case eFloatValue:
		return _val.floatValue != 0.0f;
        
    case eDoubleValue:
        return _val.doubleValue != 0.0l;
        
	case eBoolValue:
		return _val.boolValue;

	case eStringValue:
		return _stringValue == "true";

	case eNullValue:
		throw rcNullValueException();

    case eRectValue:
        throw rcConvertValueException( "rect to bool conversion failed" );

    case eAffineRectValue:
        throw rcConvertValueException( "affine rect to int conversion failed" );
        
    case ePositionValue:
        throw rcConvertValueException( "position to bool conversion failed" );

	case eiPairValue:
        throw rcConvertValueException( "ipair to bool conversion failed" );
        
	}
	throw rcConvertValueException( "unknown to bool conversion failed" );
}

// return value cast as string
std::string rcValue::stringValue( void ) const
{
	strstream buf;
	switch (_type)
	{
	case eIntValue:
		buf << _val.intValue;
		break;

	case eFloatValue:
        buf.precision(12);
		buf << _val.floatValue;
		break;

    case eDoubleValue:
        buf.precision(12);
		buf << _val.doubleValue;
		break;

    case eBoolValue:
		buf << (_val.boolValue ? "true" : "false");
		break;

	case eStringValue:
		buf << _stringValue;
		break;

    case eRectValue:
        buf << "[";
        buf << _val.rectValue[0] << " ";
        buf << _val.rectValue[1] << " ";
        buf << _val.rectValue[2] << " ";
        buf << _val.rectValue[3] << "]";
        break;

	case eAffineRectValue:
	  buf << "{";
        buf << _val.affineValue[0] << " ";
        buf << _val.affineValue[1] << " ";
        buf << _val.affineValue[2] << " ";
        buf << _val.affineValue[3] << "]";
        buf << _val.affineValue[4] << "]";
	break;

   case ePositionValue:
       buf.precision(12);
       buf << _val.positionValue[0] << "," << _val.positionValue[1];
       break;

   case eiPairValue:
       buf.precision(12);
       buf << _val.ipairValue[0] << "," << _val.ipairValue[1];
       break;
       
	case eNullValue:
		throw rcNullValueException();
	}
	buf << ends;
    std::string value( buf.str() );
    buf.freeze( false ); // Without freeze(), strstream::str() leaks the string it returns
    
	return value;
}

// return value cast as rect
rcRect rcValue::rectValue( void ) const
{
	if (_type == eRectValue)
	  return rcRect (_val.rectValue[0], _val.rectValue[1], _val.rectValue[2], _val.rectValue[3]);

    if (_type == eStringValue) {
        int x, y, w, h;
        // Try bracketed format
        if (sscanf( _stringValue.c_str() , "[%i %i %i %i]" , &x, &y, &w, &h ) != 4) {
            // Try comma-separated format
            if (sscanf( _stringValue.c_str() , "%i,%i,%i,%i" , &x, &y, &w, &h ) != 4) {
                std::string what( "string value \"" );
                what += _stringValue;
                what += "\" to rect conversion failed";
                throw rcConvertValueException( what );
            }
        }
        return rcRect( x, y, w, h );
    }
    // Cannot convert from other types
    throw rcConvertValueException( "other to rect conversion failed" );
}

// return value cast as rect
rcAffineRectangle rcValue::affineValue( void ) const
{
  static const rcDPair midOrigin (0.0, 0.5);
	if (_type == eAffineRectValue)
	  {
	    rc2Dvector org ((double) _val.affineValue[0], (double) _val.affineValue[1]);
	    rcRadian angle ( (double) _val.affineValue[4]);
	    rcDPair scale ((double)  _val.affineValue[2], (double)  _val.affineValue[3]);
	    rcIPair size ( (int32) _val.affineValue[2],  (int32) _val.affineValue[3]);
	    return rcAffineRectangle (org, angle, scale, size, midOrigin);
	  }

    if (_type == eStringValue) {
      float x, y, w, h, a;
        // Try bracketed format
        if (sscanf( _stringValue.c_str() , "[%f %f %f %f %f]" , &x, &y, &w, &h, &a ) != 5) {
            // Try comma-separated format
	  if (sscanf( _stringValue.c_str() , "%f,%f,%f,%f, %f" , &x, &y, &w, &h, &a ) != 5) {
	    std::string what( "string value \"" );
	    what += _stringValue;
	    what += "\" to rect conversion failed";
	    throw rcConvertValueException( what );
	  }
        }
	return rcAffineRectangle (rc2Dvector ((double) x, (double) y),
				  rcRadian ( (double) a), rcDPair ((double) w, (double) h), rcIPair ( (int32) w, (int32) h),  midOrigin);
    }
    // Cannot convert from other types
    throw rcConvertValueException( "other to affine rect conversion failed" );
}

// return value cast as rect
rcFPair rcValue::positionValue( void ) const
{
	if (_type == ePositionValue)
	  return rcFPair (_val.positionValue[0], _val.positionValue[1]);
    if (_type == eStringValue) {
        float x, y;
        // Try comma-separated format
        if (sscanf( _stringValue.c_str() , "%f,%f" , &x, &y ) != 2) {
                std::string what( "string value \"" );
                what += _stringValue;
                what += "\" to position conversion failed";
                throw rcConvertValueException( what );
        }
        return rcFPair( x, y );
    }
    // Cannot convert from other types
    throw rcConvertValueException( "other to position conversion failed" );
}

// return value cast as rect
rcIPair rcValue::ipairValue( void ) const
{
	if (_type == eiPairValue)
	  return rcIPair (_val.ipairValue[0], _val.ipairValue[1]);
    if (_type == eStringValue) {
        int32 x, y;
        // Try comma-separated format
        if (sscanf( _stringValue.c_str() , "%d,%d" , &x, &y ) != 2) {
                std::string what( "string value \"" );
                what += _stringValue;
                what += "\" to position conversion failed";
                throw rcConvertValueException( what );
        }
        return rcIPair( x, y );
    }
    // Cannot convert from other types
    throw rcConvertValueException( "other to position conversion failed" );
}

// equals support: values are only equal if same type and value
bool rcValue::equals( const rcValue& otherValue ) const
{
	if (otherValue._type != _type)
		return false;

	switch (_type)
	{
	case eIntValue:
		return _val.intValue == otherValue._val.intValue;

	case eFloatValue:
		return _val.floatValue == otherValue._val.floatValue;

    case eDoubleValue:
		return _val.doubleValue == otherValue._val.doubleValue;
        
	case eBoolValue:
		return _val.boolValue == otherValue._val.boolValue;

	case eStringValue:
		return _stringValue == otherValue._stringValue;

    case eRectValue:
        return _val.rectValue[0] == otherValue._val.rectValue[0] &&
	  _val.rectValue[1] == otherValue._val.rectValue[1] &&
	  _val.rectValue[2] == otherValue._val.rectValue[2] &&
	  _val.rectValue[3] == otherValue._val.rectValue[3];

    case eAffineRectValue:
        return _val.affineValue[0] == otherValue._val.affineValue[0] &&
	  _val.affineValue[1] == otherValue._val.affineValue[1] &&
	  _val.affineValue[2] == otherValue._val.affineValue[2] &&
	  _val.affineValue[3] == otherValue._val.affineValue[3] &&
	  _val.affineValue[4] == otherValue._val.affineValue[4];

    case ePositionValue:
      return _val.positionValue[0] == otherValue._val.positionValue[0] &&
	_val.positionValue[1] == otherValue._val.positionValue[1];

    case eiPairValue:
      return _val.ipairValue[0] == otherValue._val.ipairValue[0] &&
	_val.ipairValue[1] == otherValue._val.ipairValue[1];
        
        
	case eNullValue:
		return true;
	}
	throw rcConvertValueException( "rcValue::equals() unknown types");
}

// return true if null value
bool rcValue::isNull( void ) const
{
	return _type == eNullValue;
}

// get the value type
rcValue::Type rcValue::getType( void ) const
{
	return _type;
}

// insertion operator
ostream& operator << ( ostream& os, const rcValue& value )
{
   os << value.stringValue();
   return os;
}
