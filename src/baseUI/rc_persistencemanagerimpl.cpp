/*
 *
 * $Id: rc_persistencemanagerimpl.cpp 6565 2009-01-30 03:24:44Z arman $
 *
 *
 * Persistence management implementation
 *
 * Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 */

#include <rc_thread.h> // For rcLock

#include <rc_persistencemanager.h>

/******************************************************************************
*	rcPersistenceManager class implementation
*
******************************************************************************/

class rcPersistenceManagerImpl : public rcPersistenceManager
{
public:
    rcPersistenceManagerImpl() : mGeneratorComment( "Unknown Application" ) { };

    // File format export extension
    virtual const std::string fileFormatExportExtension( rcExperimentFileFormat format ) const;
    // File format export filter for dialog boxes
    virtual const std::string fileFormatExportFilter( rcExperimentFileFormat format ) const;
    // File format export caption for dialog boxes
    virtual const std::string fileFormatExportCaption( rcExperimentFileFormat format ) const;
    
    // File format import filter for dialog boxes
    virtual const std::string fileFormatImportFilter( rcExperimentFileFormat format ) const;
    // File format import caption for dialog boxes
    virtual const std::string fileFormatImportCaption( rcExperimentFileFormat format ) const;

    // Set data generator comment (done once during startup)
    virtual void setGeneratorComment( const std::string& comment );
    // Get data generator comment
    virtual const std::string generatorComment();

    // Set current data import directory
    virtual void setImportDirectory( const std::string& comment );
    // Get current data import directory
    virtual const std::string getImportDirectory();
    
private:
    rcMutex  mMutex;            // Mutex to protect globally mutable data
    std::string mGeneratorComment; // Generator information comment
    std::string mImportDirectory;  // Current data import directory
};

// File format export extension
const std::string rcPersistenceManagerImpl::fileFormatExportExtension( rcExperimentFileFormat format ) const
{
    std::string empty;
    
    switch( format ) {
        case eExperimentCSVFormat:
            return std::string( ".csv" );
        case eExperimentNativeFormat:
            return std::string( ".rfyeml" );
        case eExperimentNativeMovieFormat:
            return std::string( ".rfymov" );
        case eExperimentMolDevSTKFormat:
            return std::string( ".stk" );
        case eExperimentQuickTimeMovieFormat:
             return std::string( ".mov" );
        case eExperimentQuickTimeImageFormat:
        case eExperimentAllFormat:
            // Not applicable, there is no single extension
            break;
    }

    return empty;
}

// File format export caption for dialog boxes
const std::string rcPersistenceManagerImpl::fileFormatExportCaption( rcExperimentFileFormat format ) const
{
    std::string empty;
    
    switch( format ) {
        case eExperimentCSVFormat:
            return std::string( "Export Experiment Data as CSV" );
        case eExperimentNativeFormat:
            return std::string( "Save Experiment Data" );
        case eExperimentNativeMovieFormat:
            return std::string( "Save Movie File" );
        case eExperimentMolDevSTKFormat:
            return std::string( "Save Molecular Deviceds stk File" );
        case eExperimentQuickTimeMovieFormat:
            return std::string( "Export Movie as QuickTime" );
        case eExperimentQuickTimeImageFormat:
            return std::string( "Export Image");
        case eExperimentAllFormat:
            return std::string( "Save Experiment Data In All Formats" );
    }

    return empty;
}

// File format export filter for dialog boxes
const std::string rcPersistenceManagerImpl::fileFormatExportFilter( rcExperimentFileFormat format ) const
{
    std::string empty;
    
    switch( format ) {
        case eExperimentCSVFormat:
            return std::string( "Excel CSV File (*.csv *.CSV)" );
        case eExperimentNativeFormat:
            return std::string( "Reify Experiment File (*.rfyeml *.RFYEML)" );
        case eExperimentNativeMovieFormat:
            return std::string( "Reify Movie File (*.rfymov *.RFYMOV)" );
        case eExperimentMolDevSTKFormat:
            return std::string( "Molecular Devices stk File (*.stk *.STK)" );
        case eExperimentQuickTimeMovieFormat:
            return std::string( "Movie Files (*.mov *.MOV *.avi *.AVI *.mpg *.MPG *.rfymov *.RFYMOV)" );
        case eExperimentQuickTimeImageFormat:
            return std::string( "Image Files (*.tif *.TIF *.tiff *.TIFF *.jpg *.JPG *.jpeg *.JPEG *.gif *.GIF *.qtif *.QTIF *.bmp *.BMP)" );
        case eExperimentAllFormat:
            return std::string( "Reify Experiment Data (*.csv *.CSV *.rfyeml *.RFYEML)" );
    }

    return empty;
}

// File format import filter for dialog boxes
const std::string rcPersistenceManagerImpl::fileFormatImportFilter( rcExperimentFileFormat format ) const
{
    return fileFormatExportFilter( format );
}

// File format import caption for dialog boxes
const std::string rcPersistenceManagerImpl::fileFormatImportCaption( rcExperimentFileFormat format ) const
{
    std::string empty;
    
    switch( format ) {
        case eExperimentCSVFormat:
            return std::string( "Import CVS File" );
        case eExperimentNativeFormat:
            return std::string( "Open Experiment" );
        case eExperimentNativeMovieFormat:
        case eExperimentQuickTimeMovieFormat:
            return std::string( "Import Movie" );
        case eExperimentMolDevSTKFormat:
            return std::string( "Import stk" );
        case eExperimentQuickTimeImageFormat:
            return std::string( "Import Images");
        case eExperimentAllFormat:
            return std::string( "Not Applicable");
    }

    return empty;
}

// Set data generator comment
void rcPersistenceManagerImpl::setGeneratorComment( const std::string& comment )
{
    rcLock lock( mMutex );
    mGeneratorComment = comment;
}

// Get data generator comment
const std::string rcPersistenceManagerImpl::generatorComment()
{
    rcLock lock( mMutex );
    return mGeneratorComment;
}

// Set current data import directory
void rcPersistenceManagerImpl::setImportDirectory( const std::string& dir )
{
    rcLock lock( mMutex );
    mImportDirectory = dir;
}

// Get current data import directory
const std::string rcPersistenceManagerImpl::getImportDirectory()
{
    rcLock lock( mMutex );
    return mImportDirectory;
}

/******************************************************************************
*	Persistence manager instantiation
******************************************************************************/

// The implementation of the factory method
rcPersistenceManager* rcPersistenceManagerFactory::getPersistenceManager( const char* implKey )
{
    rmUnused( implKey );
    static rcPersistenceManagerImpl persistenceManager;
    
    return &persistenceManager;
}

//@note output stream function for experimentAttributes

std::ostream& operator<<(std::ostream& o, rcExperimentAttributes& r)
{
	o << r.fileName << std::endl << r.inputName << "[" << r.frameWidth << "," << r.frameHeight << "," << r.frameDepth << "]";
  return o;
}
