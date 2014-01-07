/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_xml.cpp 6565 2009-01-30 03:24:44Z arman $
*
*	This file contains the implementation od XML utility classes
*
******************************************************************************/

#include <iostream>
#include <stack>
#include "rc_xml.h"

#if WIN32
using namespace std;
#endif

// Characater definitions
static const char cOpenBracket = '<';
static const char cCloseBracket = '>';
static const char cDoubleQuote = '\"';
static const char cEquals = '=';
static const char cSlash = '/';
static const char cTab = '\t';
static const char cSpace = ' ';
static const char cAmpersand = '&';

static const char* cOpenBracketSlash = "</";
static const char* sStartComment = "<!--";
static const char* sEndComment = "-->";

// Global XML name mapper for current file version
static const rcXMLNameMapper gXMLNameMapper( rcXMLElementNames, eXMLElementMax,
                                             rcXMLAttributeNames, eXMLAttributeMax );
// Global XML name mapper for file verion 0
static const rcXMLNameMapper gXMLNameMapperV0( rcXMLElementNamesV0, eXMLElementMax,
                                               rcXMLAttributeNamesV0, eXMLAttributeMax );

// Get appropriate name mapper for version
const rcXMLNameMapper* rfGetXMLNameMapper( int32 version )
{
    switch ( version ) {
        case 0:
        case 1:
            return &gXMLNameMapperV0;
        case 2:
        case rcXMLFileVersion:
            return &gXMLNameMapper;
        default:
            // Unknown version, return latest mapper
            // Loading will be aborted anyway so it doesn't matter
            return &gXMLNameMapper;
    }

    return NULL;
}

//
// Local utilities
//


//
// Stream operator implementations
//
    
// Stream insertion operator for XML open tag class.
// Inserts the tag to stream
ostream& operator << ( ostream& os, const rcXMLOpenTag& e ) 
{
    if ( e.element() == eXMLElementComment ) {
        // Special case for comment element
        os << sStartComment;
    } else {
        // Open bracket
        os << cOpenBracket;
        // Tag name
        os << gXMLNameMapper.elementName( e.element() );
       
        // Attributes
        const rcAttributeList& attributes = e.attributes();
        if ( !attributes.empty() ) {
            rcAttributeList::const_iterator a;
            for ( a = attributes.begin(); a < attributes.end(); a++ ) {
                // Name
                os << cSpace << gXMLNameMapper.attributeName( a->first );
                // Value
                os << cEquals << cDoubleQuote << a->second << cDoubleQuote;
            }
        }
        // End bracket
        os << cCloseBracket;
    }
    
    return os;
}

// Stream insertion operator for XML close tag class.
// Inserts the tag to stream
ostream& operator << ( ostream& os, const rcXMLCloseTag& e ) 
{
    const rcXMLElementType type = e.element();
    
    if ( type == eXMLElementComment ) {
        // Special case for comment element
        os << sEndComment << "\n";
    } else {
        //    </                   name                                  >
        os << cOpenBracketSlash << gXMLNameMapper.elementName( type ) << cCloseBracket;
        if ( type != eXMLElementTrackValue )
            os << endl;
    }
    
    return os;
}

// Stream insertion operator for XML empty tag class.
// Inserts the tag to stream
ostream& operator << ( ostream& os, const rcXMLEmptyTag& e ) 
{
    const rcXMLElementType type = e.element();
    
    if ( type == eXMLElementComment ) {
        // Special case for comment element
        os << sEndComment << "\n";
    } else {
        //    <               name                                  /         > 
        os << cOpenBracket << gXMLNameMapper.elementName( type ) << cSlash << cCloseBracket << "\n";
    }
    
    return os;
}

// Stream insertion operator for XML element class
// Inserts the tag to stream
ostream& operator << ( ostream& os, const rcXMLElement& e ) {
    // Open tag
    os << rcXMLOpenTag( e.type(), e.attributes() );

    // Content
    const std::string& content = e.content();

    // Character entity reference creation may be required
    for ( uint32 i = 0; i < content.size(); ++i ) {
        const char c = content[i];

        // We MUST use entity references for '<', '>' and '&'
        if ( c == cOpenBracket ) {
            os << "&lt;";
        } else if ( c == cCloseBracket ) {
            os << "&gt;";
        } else if ( c == cAmpersand ) {
            os << "&amp;";
        } else
            os << c;
    }
    // Add newline after an empty element
    if ( content.empty() )
        os << "\n";
            
    return os;
}

// Store close tag type with its indent level
typedef pair<rcXMLElementType,int32> rcIndentedCloseTag;

// Stream insertion operator for indented close tag class.
// Inserts the tag to stream
ostream& operator << ( ostream& os, const rcIndentedCloseTag& t ) 
{
    return os << rcXMLCloseTag( t.first );
}
    
// Stream insertion operator for XML element tree class
// Inserts the whole tree to stream
ostream& operator << ( ostream& os, const rcXMLElementTree& tree )
{
    return output( os, tree, NULL, 0.0 );
}

// Output tree to stream
ostream& output( ostream& os, const rcXMLElementTree& tree,
                 rcProgressIndicator* progress, double completedAlready )
{
    // Pre-order iterators
    rcXMLElementTree::iterator sibBegin = tree.begin();
    rcXMLElementTree::iterator sibEnd = tree.end();
    const uint32 totalElements = tree.size();
    uint32 processedElements = 0;

    // Stack for close tags
    stack<rcIndentedCloseTag> closeStack;
    
    while ( sibBegin != sibEnd ) {
        const rcXMLElement& e = *sibBegin;
        const rcXMLElementType type = e.type();
        const int32 depth = tree.depth( sibBegin );

        // Report progress
        if ( progress ) {
            if ( !(processedElements % 256) ) {
                progress->progress( completedAlready + (100.0-completedAlready) * processedElements / totalElements );
            }
            ++processedElements;
        }
        // Pop close tags if necessary
        while ( !closeStack.empty() ) {
            const rcIndentedCloseTag& t = closeStack.top();
            if ( depth <= t.second ) {
                os << t;
                closeStack.pop();
            } else
                break;
        }
        // Open tag with content
        os << e;
        
        // Optimization, produce close tag immediately if there are no children
        if ( ! tree.number_of_children( sibBegin ) ) {
            // Close tag
            os << rcXMLCloseTag( type );
        }
        else {
            closeStack.push( rcIndentedCloseTag( type, depth ) );
        }
        ++sibBegin;
    }

    while ( !closeStack.empty() ) {
        os << closeStack.top();
        closeStack.pop();
    }

    os << "\n";
    
    return os;
}

