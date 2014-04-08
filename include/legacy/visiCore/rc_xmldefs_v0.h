/******************************************************************************
*   Copyright (c) 2003 Reify Corp. All Rights reserved.
*
*	$Id: rc_xmldefs_v0.h 6337 2009-01-29 05:39:02Z arman $
*
*	This file contains XML persistence element/attribute definitions for
*   file version 0.
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
rcXML_ELEMENTDEF( AnalysisRect, analysis-rect )
rcXML_ELEMENTDEF( Type, type )
rcXML_ELEMENTDEF( ValueType, value-type )
rcXML_ELEMENTDEF( DisplayFormatString, display-format-string )
rcXML_ELEMENTDEF( ExpectedMin, expected-min )
rcXML_ELEMENTDEF( ExpectedMax, expected-max )
rcXML_ELEMENTDEF( SizeLimit, size-limit )
rcXML_ELEMENTDEF( TrackStart, track-start )
rcXML_ELEMENTDEF( TrackValue, track-value )
rcXML_ELEMENTDEF( TrackVersion, tver )
rcXML_ELEMENTDEF( Frame, f )

// Common elements (used by all)
rcXML_ELEMENTDEF( Name, name )
rcXML_ELEMENTDEF( Description, description )
rcXML_ELEMENTDEF( Value, value )

// Comment (will be ignored at import time)
rcXML_ELEMENTDEF( Comment, comment )

//
// Attribute definitions
//

// First argument is enumerated type, second argument is XML attribute name.
// For example, rcXML_ATTRIBUTEDEF( Time, time ) will be used produce
// an enumerated value eAttributeTime with an associated name "time".

// Group and track attributes
rcXML_ATTRIBUTEDEF( GroupId, group-id )
rcXML_ATTRIBUTEDEF( TrackId, track-id )
rcXML_ATTRIBUTEDEF( TimeId, t )
rcXML_ATTRIBUTEDEF( Time, time )
