/******************************************************************************
*	rcExperimentImpl.h
*
*	This file contains the declaration for the implementation of
*	the rcExperiment interface.
******************************************************************************/

#ifndef rcEXPERIMENTIMPL_H
#define rcEXPERIMENTIMPL_H

#include <vector>

#if WIN32
using namespace std;
#endif

#include <rc_types.h>
#include <rc_thread.h>
#include <rc_setting.h>
#include <rc_engine.h>
#include <rc_model.h>
#include <rc_movieconverter.h>
#include <rc_framegrabber.h>

#include "rc_trackgroupimpl.h"

using namespace legacy_qtime;

/******************************************************************************
*	Constants
******************************************************************************/

// settings id constants
const int cFileNameSettingId = 0;
const int cUserNameSettingId = 1;
const int cTitleSettingId = 2;
const int cCommentsSettingId = 3;
const int cInputImageSourceNameSettingId = 4;
const int cMicroscopeMagnificationSettingId = 5;
const int cOtherMagnificationSettingId = 6;
const int cCellTypeSettingId = 7;
const int cCellTreatmentSettingId = 8;
const int cCellTemperatureSettingId = 9;
const int cCellCO2SettingId = 10;
const int cCellO2SettingId = 11;
const int cCellTreatment2SettingId = 12;
const int cImagingModeSettingId = 13;

/******************************************************************************
*	Class definitions
******************************************************************************/

class rcExperimentImpl : public rcExperiment, rcSettingSource
{
public:
	// default constructor (creates a new, empty experiment)
	rcExperimentImpl( void );

	// this constructor loads a previously run experiment from a file.
	rcExperimentImpl( const char* filename );

	// destructor
	virtual ~rcExperimentImpl( void );

	// save the experiment data to a file
    // returns errno
	int saveTo( const char* filename,
                rcExperimentFileFormat format,
                const rcMovieConverterOptions& opt,
                rcCarbonLock* lock, 
                rcProgressIndicator* progress );

	// load template into newly created experiment
	void loadTemplateData( const char* filename );

	// save current settings information to a template file
	void saveTemplateData( const char* filename );

    // load an experiment from a tree and make it current.
    // return number of errors
	int loadExperiment( const rcXMLElementTree& tree, rcExperimentImportMode mode,
                        rcEngineObserver* observer, rcProgressIndicator* progress );
    
	// create a new track group
	rcTrackGroupImpl* createTrackGroup( const char* tag ,
                                        const char* name , 
                                        const char* description,
                                        rcGroupSemantics type );

	///////////////// rcExperiment implementation ///////////////////

	// get the number of experiment settings categories
	virtual int getNSettingCategories( void );

	// get a settings category
	virtual rcSettingCategory getSettingCategory( int categoryNo );

	// get the number of experiment data track groups.
	virtual int getNTrackGroups( void );

	// get a particular experiment data track group
	virtual rcTrackGroup* getTrackGroup( int trackGroupNo );

	// get the maximum track length
	virtual rcTimestamp getLength( void );

	///////////////// rcSettingSource implementation /////////////////

	// get the current value of a setting from the setting source
	virtual const rcValue getSettingValue( int settingId );

	// set the new value of a setting to the setting source.
	virtual void setSettingValue( int settingId , const rcValue& value );

	// return whether this setting is currently enabled.
	virtual bool isSettingEnabled( int settingId );

	// return whether this setting is current changable.
	virtual bool isSettingEditable( int settingId );
    
    // return whether this setting is current persistable.
	virtual bool isSettingPersistable( int settingId );

    ///////////////// other implementation /////////////////

    // set editability for all settings
    void editableSettings( bool enabled );
    
private:
    // Set settings to unspecified state
    void unspecifySettings();
    // Export current movie to QuickTime or .rfymov
    int exportMovie( const std::string& outputFile,
                     const rcMovieConverterOptions& baseOpt,
                     rcExperimentFileFormat format,
                     rcCarbonLock* lock, 
                     rcProgressIndicator* progress );
    
    rcMutex                     _settingMutex;
    rcMutex                     _trackMutex;
    vector<rcSettingCategory>	_settings;
	vector<rcTrackGroupImpl*>	_trackGroups;
    bool                        _settingsEditable;
    
    std::string					_fileName;         // cFileNameSettingId
	std::string					_userName;         // cUserNameSettingId
	std::string					_title;            // cTitleSettingId
	std::string					_comments;         // cCommentsSettingId
    std::string                    _inputImageSource; // cInputImageSourceNameSettingId
    std::string					_microscopeMag;    // cMicroscopeMagnificationSettingId
    std::string					_otherMag;         // cOtherMagnificationSettingId
    std::string                    _cellType;         // cCellTypeSettingId
    std::string                    _cellTreatment1;   // cCellTreatmentSettingId
    std::string                    _cellTreatment2;   // cCellTreatment2SettingId

    // New settings
    std::string                    _cellTemperature;  // cCellTemperatureSettingId
    std::string                    _cellCO2;          // cCellCO2SettingId
    std::string                    _cellO2;           // cCellO2SettingId
    std::string                    _imagingMode;      // cImagingModeSettingId
};

#endif // rcEXPERIMENTIMPL_H
