/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_xml.h 6565 2009-01-30 03:24:44Z arman $
*
*	This file contains XML utility classes for file import/export.
*
******************************************************************************/

#ifndef _rcXML_H_
#define _rcXML_H_

#include <fstream>
#include <map>
#include <vector>

#include <rc_types.h>
#include <rc_tree.hh>  // STL-style tree class


//
// Forward declarations
//

class rcXMLElement;

//
// Type definitions
//


//
// Versioning note: never remove attribute or element types from
// enumerated types rcXMLElementType and rcXMLAttributeType even if they become obsolete.
// The enumerated types must contain a superset of all versions.
// Obsoleteness will be handled by lists of required and optional elements and attributes.
//

// XML element count comment, this is the first line in the file
const std::string rcXMLElementCountFormat = "<!-- Element count %i -->";

// Oldest supported XML file format version
const int32 rcXMLOldestSupportedFileVersion = 0;
// Current XML file format version
const int32 rcXMLFileVersion = 3;

// XML element/attribute names

// Define XML element enumerated type values
#define rcXML_ELEMENTDEF( TYPE, NAME ) eXMLElement##TYPE,
#define rcXML_ATTRIBUTEDEF( TYPE, NAME )

enum rcXMLElementType {
#include "rc_xmldefs.h"
    eXMLElementMax // Synthetic sentinel value
};

// Define XML element enumerated type name strings
#undef rcXML_ELEMENTDEF
#define rcXML_ELEMENTDEF( TYPE, NAME ) (#NAME),

// Current file version
const std::string rcXMLElementNames[] = {
#include "rc_xmldefs.h"
    "unknown" // Synthetic sentinel value
};

// File version 0
const std::string rcXMLElementNamesV0[] = {
#include "rc_xmldefs_v0.h"
    "unknown" // Synthetic sentinel value
};

// Define XML attribute enumerated type values
#undef rcXML_ELEMENTDEF
#undef rcXML_ATTRIBUTEDEF
#define rcXML_ELEMENTDEF( TYPE, NAME )
#define rcXML_ATTRIBUTEDEF( TYPE, NAME ) eXMLAttribute##TYPE,

enum rcXMLAttributeType {
#include "rc_xmldefs.h"
    eXMLAttributeMax // Synthetic sentinel value
};

// Define XML attribute enumerated type name strings
#undef rcXML_ATTRIBUTEDEF
#define rcXML_ATTRIBUTEDEF( TYPE, NAME ) (#NAME),

// Current file version
const std::string rcXMLAttributeNames[] = {
#include "rc_xmldefs.h"
    "unknown" // Synthetic sentinel value
};

// File version 0
const std::string rcXMLAttributeNamesV0[] = {
#include "rc_xmldefs_v0.h"
    "unknown" // Synthetic sentinel value
};

// XML element, attribute, tree, mapper classes
#include "rc_xmlelem.h"

// Global XML name mappers
// Get appropriate name mapper for version
extern const rcXMLNameMapper* rfGetXMLNameMapper( int32 version );

#endif // _rcXML_H_
