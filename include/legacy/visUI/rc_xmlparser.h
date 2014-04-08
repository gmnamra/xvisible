/*
 *  
 *
 *	$Id: rc_xmlparser.h 6427 2009-01-29 05:39:15Z arman $
 *
 *  
 *  Copyright (c) 2002 Reify Corp. All rights reserved.
 *
 */

#ifndef _UI_rcXMLPARSER_H_
#define _UI_rcXMLPARSER_H_

#include <qxml.h>
#include <qstring.h>

#include <rc_xml.h>
#include <rc_model.h>

//
// Qt-based XML parser for native experiment data import
//

class rcXMLParser : public QXmlDefaultHandler
{
public:
    rcXMLParser( rcExperimentImportMode mode, uint32 textLines,
                 rcProgressIndicator* progress,
                 QXmlErrorHandler* errorHandler );
    
    // QXmlDefaultHandler API
    bool startDocument();
    bool endDocument();
    bool startElement( const QString&, const QString&, const QString& ,
                       const QXmlAttributes& );
    bool endElement( const QString&, const QString&, const QString& );
    bool characters( const QString& ch );
    QString errorString();
    
    // Reify API

    // Accessors
    const rcXMLElementTree& elementTree() const { return mElementTree; };
    rcXMLElementTree& elementTree() { return mElementTree; };

    // Control content ignore mode
    void setIgnoreMode( rcXMLElementType elementType, bool startElement );
    
private:
    rcXMLElementTree             mElementTree;   // Final element tree
    rcXMLElementTree::iterator   mElementLatest; // Iterator pointing to latest open element
    uint32                     mErrors;        // Total count of content errors
    rcExperimentImportMode       mMode;          // Import mode
    bool                         mIgnoreAll;     // When true, ignore all incoming data
    rcTimestamp                  mStartTime;     // Start time stamp
    bool                         mEmptyTree;        // Element tree is empty
    uint32                     mExpectedElements; // Expected number of elemnst in input file
    uint32                     mParsedElements;   // Total count of parsed bytes
    rcProgressIndicator*         mProgress;         // Progress indicator
    QXmlErrorHandler*            mErrorHandler;     // Registered error handler
    QString                      mErrorString;      // Latest internal error string
    const rcXMLNameMapper*       mNameMapper;       // Name mapper for detected file version
    bool                         mVersionDetected;  // Has file version been detected
};


class rcXMLErrorHandler : public QXmlErrorHandler
{
  public:
    // ctor
    rcXMLErrorHandler( QString fileName ) :
    mFileName( fileName ),
        mErrorString(),
        mWarnings( 0 ),
        mErrors( 0 ),
        mFatalErrors( 0 ) { };
    // Virtual dtor is required
    virtual ~rcXMLErrorHandler() { };
    
    // QXmlErrorHandler API
    virtual bool warning( const QXmlParseException & exception );
    virtual bool error( const QXmlParseException & exception );
    virtual bool fatalError( const QXmlParseException & exception );
    QString errorString() const;

    // Reify API
    
    // Accessors
    uint32 warnings() const { return mWarnings; };
    uint32 errors() const { return mErrors; };
    uint32 fatalErrors() const { return mFatalErrors; };
    
  private:
    // Utilities
    QString buildErrorString(  const QString& prefix,
                               const QXmlParseException & exception );

    QString  mFileName;
    QString  mErrorString;

    uint32 mWarnings;
    uint32 mErrors;
    uint32 mFatalErrors;
};

#endif // _UI_rcXMLPARSER_H_

