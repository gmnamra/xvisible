/******************************************************************************
*   Copyright (c) 2002 Reify Corp. All Rights reserved.
*
*	$Id: rc_xmldefs.h 6335 2009-01-29 05:39:01Z arman $
*
*	This file contains XML persistence element/attribute definitions.
*
******************************************************************************/

// Warning: this file will be included multiple times for different uses.
// The lack of an inclusion guard is intentional.

//
// Element definitions
//

// First argument is enumerated type, second argument is XML element name.
// For example, rcXML_ELEMENTDEF( Track, track ) will be used produce
// an enumerated value eElementTrack with an associated name "track".

// Root element
rcXML_ELEMENTDEF( Experiment, experiment )

// 1st level elements
rcXML_ELEMENTDEF( ExperimentSettingCategory, setting-category )
rcXML_ELEMENTDEF( ExperimentData, experiment-data )
rcXML_ELEMENTDEF( Version, version )

// 2nd level elements
rcXML_ELEMENTDEF( Track, track )
rcXML_ELEMENTDEF( Group, group )
rcXML_ELEMENTDEF( TimeStamp, tm )

// Track elements
rcXML_ELEMENTDEF( Setting, setting )
rcXML_ELEMENTDEF( AnalysisRect, arect )
rcXML_ELEMENTDEF( Type, type )
rcXML_ELEMENTDEF( ValueType, vtype )
rcXML_ELEMENTDEF( DisplayFormatString, disp-frm )
rcXML_ELEMENTDEF( ExpectedMin, exp-min )
rcXML_ELEMENTDEF( ExpectedMax, exp-max )
rcXML_ELEMENTDEF( SizeLimit, sz-lim )
rcXML_ELEMENTDEF( TrackStart, start )
rcXML_ELEMENTDEF( TrackValue, v )
rcXML_ELEMENTDEF( TrackVersion, tver )
rcXML_ELEMENTDEF( Frame, f )

// Common elements (used by all)
rcXML_ELEMENTDEF( Name, name )
rcXML_ELEMENTDEF( Description, descr )
rcXML_ELEMENTDEF( Value, value )

// Comment (will be ignored at import time)
rcXML_ELEMENTDEF( Comment, comment )

//
// Attribute definitions
//

// First argument is enumerated type, second argument is XML attribute name.
// For example, rcXML_ATTRIBUTEDEF( Time, tm ) will be used produce
// an enumerated value eAttributeTime with an associated name "tm".

// Group and track attributes
rcXML_ATTRIBUTEDEF( GroupId, g )
rcXML_ATTRIBUTEDEF( TrackId, i )
rcXML_ATTRIBUTEDEF( TimeId, t )
rcXML_ATTRIBUTEDEF( Time, time ) // For backwards compatibility
