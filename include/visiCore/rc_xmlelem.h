/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_xmlelem.h 6561 2009-01-29 19:10:59Z arman $
*
*	This file contains XML utility classes for file import/export.
*
******************************************************************************/

#ifndef _rcXML_ELEM_H_
#define _rcXML_ELEM_H_

#include <fstream>
#include <map>
#include <vector>

#include <rc_types.h>
#include <rc_tree.hh>  // STL-style tree class
#include <rc_timestamp.h>

//
// This is an internal header file to be included by rc_xml.h
// Do NOT include this header file directly
//

// Name-to-type mappers
typedef map<std::string,rcXMLElementType>   rcXMLElementNameMap;
typedef map<std::string,rcXMLAttributeType> rcXMLAttributeNameMap;
typedef map<rcTimestamp,int32>         rcXMLTimeMap;

// Element attribute list: first is attribute name, second is value
typedef pair<rcXMLAttributeType,std::string> rcAttribute;
typedef vector<rcAttribute> rcAttributeList;

// XML element tree
typedef tree<rcXMLElement> rcXMLElementTree;

//
// XML name mapper class
//

class rcXMLNameMapper {
  public:
	rcXMLNameMapper( const std::string* elemArray, uint32 elemSize,
									const std::string* attrArray, uint32 attrSize ) :
        mElemNames( elemArray ), mAttrNames( attrArray ) {
        rmAssert( elemArray );
        rmAssert( attrArray );
        rmAssert( elemSize > 0 );
        rmAssert( attrSize > 0 );
        
        // Fill element map
        for ( uint32 i = 0; i < elemSize; ++i ) {
            mElementMap.insert( make_pair( elemArray[i], static_cast<rcXMLElementType>(i) ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( elementType( elemArray[i] ) == static_cast<rcXMLElementType>(i) );
        }

        // Fill attribute map
        for ( uint32 i = 0; i < attrSize; ++i ) {
            mAttributeMap.insert( make_pair( attrArray[i], static_cast<rcXMLAttributeType>(i) ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( attributeType( attrArray[i] ) == static_cast<rcXMLAttributeType>(i) );
        }
        
        // Each name should be unique, check final size
        rmAssert( mElementMap.size() == elemSize );
        rmAssert( mAttributeMap.size() == attrSize );
    };

    // Map element type to name
	const std::string& elementName( rcXMLElementType id ) const {
        return mElemNames[id];
    };
     
    // Map element name to type
	rcXMLElementType elementType( const std::string& name ) const {
        rcXMLElementNameMap::const_iterator pos = mElementMap.find( name );
        if ( pos != mElementMap.end() )
            return pos->second;
        else
            return eXMLElementMax;
    };

    // Map attribute type to name
	const std::string& attributeName( rcXMLAttributeType id ) const {
        return mAttrNames[id];
    };

    // Map attribute name to type
	rcXMLAttributeType attributeType( const std::string& name ) const {
        rcXMLAttributeNameMap::const_iterator pos = mAttributeMap.find( name );
        if ( pos != mAttributeMap.end() )
            return pos->second;
        else
            return eXMLAttributeMax;
    };
    
  private:
    rcXMLElementNameMap   mElementMap;
    rcXMLAttributeNameMap mAttributeMap;
	const std::string*       mElemNames;    // Element name array
	const std::string*       mAttrNames;    // Attribute name array
};

class rcXMLTimeMapper {
  public:
    rcXMLTimeMapper() {}
    rcXMLTimeMapper( const vector<rcTimestamp>& timeArray )
    {
        mTimes = timeArray;
        // Fill time map
        for ( uint32 i = 0; i < timeArray.size(); ++i ) {
            mTimeMap.insert( make_pair( timeArray[i], i ) );
            // Immediate self-test, it can catch duplicates
            rmAssert( timeIndex( mTimes[i] ) == int32(i) );
        }
        
        // Each time should be unique, check final size
        rmAssert( mTimeMap.size() == timeArray.size() );
    };

    // Map index to time value
    const rcTimestamp timeValue( uint32 i ) const {
        if ( i < mTimes.size() )
            return mTimes[i];
        else
            return rcTimestamp(-1.0);
    };
     
    // Map time value to index
    int32 timeIndex( const rcTimestamp& time ) const {
        rcXMLTimeMap::const_iterator pos = mTimeMap.find( time );
        if ( pos != mTimeMap.end() )
            return pos->second;
        else
            return -1;
    };

  private:
    rcXMLTimeMap          mTimeMap;
    vector<rcTimestamp>   mTimes;
};

//
// XML tag classes
//

// Open tag: <example>
class rcXMLOpenTag {
  public:
    // ctors
    rcXMLOpenTag( rcXMLElementType element )
        : mElement( element ), mAttributes() { };
    
    rcXMLOpenTag( rcXMLElementType element, const rcAttributeList& attrs )
        : mElement( element ), mAttributes( attrs ) { };

    // Accessors
    rcXMLElementType element() const { return mElement; };
    const rcAttributeList& attributes() const { return mAttributes; };
    
  private:
    rcXMLElementType  mElement;
    rcAttributeList   mAttributes;
};

// Close tag: </example>
class rcXMLCloseTag {
  public:
    // ctors
    rcXMLCloseTag( rcXMLElementType element )
        : mElement( element ) { };
    
    // Accessors
    rcXMLElementType element() const { return mElement; };
    
  private:
    rcXMLElementType  mElement;
};

// Empty tag: <example/>
class rcXMLEmptyTag {
  public:
     // ctors
    rcXMLEmptyTag( rcXMLElementType element )
        : mElement( element ) { };
    
    // Accessors
    rcXMLElementType element() const { return mElement; };
    
  private:
    rcXMLElementType  mElement;
};

//
// XML element class
//

class rcXMLElement {
  public:
    // ctors
    rcXMLElement() : mType( eXMLElementMax ) { };
    
    rcXMLElement( rcXMLElementType type )
	: mType( type ), mAttributes( rcAttributeList() ), mContent( std::string() ) {
    };
	rcXMLElement( rcXMLElementType type, const std::string& content )
        : mType( type ), mAttributes( rcAttributeList() ), mContent( content ) {
    };
    rcXMLElement( rcXMLElementType type, const rcAttributeList& attrs )
	: mType( type ), mAttributes( attrs ), mContent( std::string() ) {
    };
	rcXMLElement( rcXMLElementType type, const rcAttributeList& attrs, const std::string& content )
        : mType( type ), mAttributes( attrs ), mContent( content ) {
    };
    
    // Accessors
    rcXMLElementType type() const { return mType; };
    const rcAttributeList& attributes() const { return mAttributes; };
	const std::string& content() const { return mContent; };
	const std::string attributeValue( rcXMLAttributeType attributeType ) const {
        // TODO: replace linear search with something faster
        rcAttributeList::const_iterator i;

        for ( i = mAttributes.begin(); i < mAttributes.end(); i++ ) {
            if ( i->first == attributeType ) {
                // Found attribute with matching name
                return i->second;
            }
        }
		return std::string();
    };
    
    // Mutators
	std::string& content()  { return mContent; };
    
  private:
    rcXMLElementType        mType;
    rcAttributeList         mAttributes;
	std::string                mContent;
};

// Stream operator declarations

ostream& operator << ( ostream& os, const rcXMLOpenTag& e );
ostream& operator << ( ostream& os, const rcXMLCloseTag& e );
ostream& operator << ( ostream& os, const rcXMLEmptyTag& e );
ostream& operator << ( ostream& os, const rcXMLElement& e );
ostream& operator << ( ostream& os, const rcXMLElementTree& tree );

// Output tree to stream
ostream& output( ostream& os, const rcXMLElementTree& tree,
                 rcProgressIndicator* progress, double completedAlready );

#endif // _rcXML_ELEM_H_
