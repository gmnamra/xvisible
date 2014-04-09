/*
 *  
 *
 *	$Id: rc_xmlparser.cpp 7191 2011-02-07 19:38:55Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#include "rc_xmlparser.h"

#include <iostream>
#include <strstream>

#include <QtGui/QtGui>
#include <QtCore/QtCore>



//
// rcXMLParser implementation
//

rcXMLParser::rcXMLParser( rcExperimentImportMode mode, uint32 textLines,
                          rcProgressIndicator* progress,
                          QXmlErrorHandler* errorHandler ) :
        QXmlDefaultHandler(),
        mErrors( 0 ),
        mMode( mode ),
        mIgnoreAll( false ),
        mStartTime( 0.0 ),
        mEmptyTree( true ),
        mExpectedElements( textLines ), // Just a guess, one element per line
        mParsedElements( 0 ),
        mProgress( progress ),
        mErrorHandler( errorHandler ),
        mNameMapper( rfGetXMLNameMapper(0) ),
        mVersionDetected( false )
{
}

// Called once at start of document
bool rcXMLParser::startDocument()
{
    mErrors = 0;
    mElementLatest = mElementTree.begin();
    mIgnoreAll = false;
    mStartTime = rcTimestamp::now();
    mEmptyTree = true;
    if ( mProgress)
        mProgress->progress( 0.0 );
    return true;
}

// Called once at end of document
bool rcXMLParser::endDocument()
{
    rcTimestamp elapsedTime = rcTimestamp::now() - mStartTime;
    cout << mElementTree.size() << " XML elements imported from file in " << elapsedTime.secs() << " seconds" << endl;
    if ( mProgress)
        mProgress->progress( 100.0 );
    return true;
}

// Start tag of element encountered, ie <experiment>
bool rcXMLParser::startElement( const QString&,
                                const QString&,
                                const QString& name,
                                const QXmlAttributes& attributes )
{
    if ( mProgress ) {
        ++mParsedElements;
        if ( !(mParsedElements % 32) )
            mProgress->progress( 100.0 * mParsedElements/mExpectedElements );
    }
    rcXMLElementType type = mNameMapper->elementType( name.latin1() );
    
    if ( type != eXMLElementMax ) {
        // Control mode
        setIgnoreMode( type, true );

        if ( !mIgnoreAll ) {
            rcAttributeList attrs;
            const int attrCount = attributes.count();

            if ( attrCount > 0 ) {
                attrs.reserve( attrCount );
                // Produce attributes
                for ( int i = 0; i < attrCount; ++i ) {
									const std::string attributeName = attributes.qName( i ).latin1();
                    // Check validity
                    rcXMLAttributeType type = mNameMapper->attributeType( attributeName );
                    if ( type != eXMLAttributeMax ) {
											const std::string attributeValue( attributes.value( i ).latin1() );
                        attrs.push_back( rcAttribute( type, attributeValue ) );
                    } else {
                        // Unknown attribute
                        cerr << "rcXMLParser error: unknown attribute \"" << attributeName << "\"" << endl;
                    }
                }
            }
            // Create element
            rcXMLElement e( type, attrs );
            // Populate tree
            if ( mEmptyTree ) {
                // Add first element as a special case
                mElementLatest = mElementTree.insert( mElementLatest, e );
                if ( mElementTree.size() > 0 )
                    mEmptyTree = false;
            } else {
                mElementLatest = mElementTree.append_child( mElementLatest, e );
            }
        }
    }
    else {
        // Error, unknown element
//        cerr << "rcXMLParser error: unknown element <" << name << ">" << endl;
    }
    
    return !mErrors;
}

// End tag of element encountered, ie </experiment>
bool rcXMLParser::endElement( const QString&,
                              const QString&,
                              const QString& name )
{
    if ( !mEmptyTree ) {
        rcXMLElementType elementType = mNameMapper->elementType( name.latin1() );

       if ( elementType == eXMLElementVersion && !mVersionDetected ) {
             rcXMLElement& e = *mElementLatest;
				 std::string& content = e.content();
             rcValue fileVer( content.c_str() );
             int v = fileVer;
             if ( v < rcXMLOldestSupportedFileVersion ) {
                 ostrstream estr;
                 // File version too obsolete to parse, bail out now
                 ++mErrors;
                 estr <<  "\nobsolete experiment file version " << v
                      << " not supported.";
                 estr << ends;
                 estr.freeze();
                 mErrorString = estr.str();
             } else if ( v > rcXMLFileVersion ) {
                 ostrstream estr;
                 // File version too new to parse, bail out now
                 ++mErrors;
                 estr <<  "\nexperiment file version " << v
                      << " not supported.";
                 estr << ends;
                 estr.freeze();
                 mErrorString = estr.str();
             }
             mNameMapper = rfGetXMLNameMapper( v );
             mVersionDetected = true;
        }
        
        // Control mode
        setIgnoreMode( elementType, false );
         
        if ( ! mIgnoreAll ) {
            if ( mElementLatest->type() == elementType ) {
                // Pop latest
                mElementLatest = mElementTree.parent( mElementLatest );
            } else {
                // Mismatched start and end tags, serious error!
                // This will be reported by rcXMLErrorHandler so don't count it here
            }
        }
    } else {
        // Empty tree, serious error!
        // This will be reported by rcXMLErrorHandler so don't count it here
    }

    return !mErrors;
}

// Gather element content
bool rcXMLParser::characters( const QString& ch )
{
    if ( !mIgnoreAll ) {
        // Strip extra whitespace
        QString stripped = ch.stripWhiteSpace();
        
        if ( !stripped.isEmpty() ) {
            if ( !mEmptyTree ) {
                rcXMLElement& e = *mElementLatest;
                // Add content to latest element
							std::string& content = e.content();
                content += ch.latin1();
            }
        }
    }

    return !mErrors;
}

// Control content ignore mode
void rcXMLParser::setIgnoreMode( rcXMLElementType elementType,
                                 bool startElement )
{
     switch ( mMode ) {
         case eExperimentImportAllSettings:
         case eExperimentImportExperimentSettings: 
             if ( elementType == eXMLElementExperimentData ) {
                 // We want to load settings only, ignore all measurement data
                 mIgnoreAll = startElement;
             }
             break;

         case eExperimentImportAllData:
             if ( elementType == eXMLElementExperimentSettingCategory ) {
                 // We want to load measurements only, ignore all settings data
                 mIgnoreAll = startElement;
             }
             break;
             
         case eExperimentImportAll:
             // Load everything
             break;
     }
}
// Return latest error 
QString rcXMLParser::errorString()
{
    return mErrorString;
}

//
// rcXMLErrorHandler implementation
//

bool rcXMLErrorHandler::warning( const QXmlParseException & exception )
{
    ++mWarnings;
    mErrorString = buildErrorString( "rcXMLErrorHandler warning: ", exception );
    return true;
}

// Note, this is counted as a fatal error
bool rcXMLErrorHandler::error( const QXmlParseException & exception )
{
    ++mFatalErrors;
    mErrorString = buildErrorString( "rcXMLErrorHandler error: ", exception );
    return false;
}
    
bool rcXMLErrorHandler::fatalError( const QXmlParseException & exception )
{
    ++mFatalErrors;
    mErrorString = buildErrorString( "rcXMLErrorHandler fatal error: ", exception );
    return false;
}
    
QString rcXMLErrorHandler::errorString() const
{
    return mErrorString;
}

// private

QString rcXMLErrorHandler::buildErrorString( const QString& prefix,
                                             const QXmlParseException & exception )
{
    QString error;

    error = prefix + QString( "%1 %2 %3 line %4, column %5: %6\n" )
        .arg( mFileName )
        .arg( exception.systemId() )
        .arg( exception.publicId() )
        .arg( exception.lineNumber() )
        .arg( exception.columnNumber() )
        .arg( exception.message() );
    
    return error;
}
