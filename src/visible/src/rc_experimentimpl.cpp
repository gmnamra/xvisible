// rc_experimentimpl.cpp

#include <strstream>

    //#include <rc_movieconverter.h>

#include "rc_scalartrackimpl.h"
#include "rc_videotrackimpl.h"
#include "rc_csvexporter.h"
#include "rc_nativeexporter.h"
#include "rc_nativeimporter.h"
#include "rc_experimentimpl.h"

#if WIN32
using namespace std;
#endif

/******************************************************************************
*	Setting specs
******************************************************************************/

// choices for specifying the file name filter for native input file selection
const rcSettingChoice inputFileFilterChoices[] =
{
    rcSettingChoice( -1, "unused", "unused" ),
    rcSettingChoice( 0 , 0 , 0 )
};

// Choices for selecting the lens magnification factor
static const rcSettingChoice lensMagChoices[] = 
{
    rcSettingChoice( 1,	 "1x"	, "1x magnification" ),
    rcSettingChoice( 4,	 "4x"	, "4x magnification" ),
    rcSettingChoice( 10, "10x"	, "10x magnification" ),
    rcSettingChoice( 20, "20x"	, "20x magnification" ),
    rcSettingChoice( 40, "40x"	, "40x magnification" ),
    rcSettingChoice( 60, "60x"	, "60x magnification" ),
    rcSettingChoice( 100,"100x", "100x magnification" ),
    rcSettingChoice( 0,  "Other", "other magnification" ),
    rcSettingChoice( -1, "Unspecified",  "unspecified magnification" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for selecting the other magnification factor
static const rcSettingChoice otherMagChoices[] = 
{
    rcSettingChoice( 1,	   "1x"	, "1x magnification" ),
    rcSettingChoice( 0.7,  "0.7x"	, "0.7x magnification" ),
    rcSettingChoice( 0.6,  "0.6x"	, "0.6x magnification" ),
    rcSettingChoice( 0.45, "0.45x"	, "0.45x magnification" ),
    rcSettingChoice( 0.35, "0.35x"	, "0.35x magnification" ),
    rcSettingChoice( 0,    "Other", "other magnification" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified magnification" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for temperature
static const rcSettingChoice temperatureChoices[] = 
{
    rcSettingChoice( 37, "37 C"	,  "37 degrees Celsius" ),
    rcSettingChoice( 21, "Room temp", "room temperature" ),
    rcSettingChoice( 0,  "Other"	,  "other temperature" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified temperature" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for CO2
static const rcSettingChoice CO2Choices[] = 
{
    rcSettingChoice( 5, "5%"	,  "5 percent" ),
    rcSettingChoice( 0.035, "Atmospheric", "room CO2" ),
    rcSettingChoice( 0,  "Other"	,  "other CO2" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified CO2" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for O2
static const rcSettingChoice O2Choices[] = 
{
    rcSettingChoice( 21, "Atmospheric", "room O2" ),
    rcSettingChoice( 0,  "Other"	,  "other O2" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified O2" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for selecting the imaging mode
static const rcSettingChoice imgModeChoices[] = 
{
    rcSettingChoice( 1,	 "Phase Contrast", "Phase Contrast" ),
    rcSettingChoice( 2,	 "DIC"	, "DIC" ),
    rcSettingChoice( 3,	 "Bright Field"	, "Bright Field" ),
    rcSettingChoice( 4,	 "Dark Field"	, "Dark Field" ),
    rcSettingChoice( 5,	 "Hybrid"	, "Hybrid" ),
    rcSettingChoice( 6,	 "Epifluoresence"	, "Epifluoresence" ),
    rcSettingChoice( 0,  "Other", "other mode" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified mode" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// Choices for selecting cell/organism type
static const rcSettingChoice cellTypeChoices[] = 
{
    rcSettingChoice( 1,  "HL-60", "HL-60" ),
    rcSettingChoice( 2,  "Differentiated HL-60", "Differentiated HL-60" ),
    rcSettingChoice( 3,  "HeLa", "HeLa" ),
    rcSettingChoice( 4,  "SK-OV-3", "SK-OV-3" ),
    rcSettingChoice( 5,  "HUVEC", "HUVEC" ),
    rcSettingChoice( 6,  "U20S", "U20S" ),
    rcSettingChoice( 7,  "U20S rev-GFP", "U20S rev-GFP" ),
    rcSettingChoice( 8,  "3T3", "3T3" ),
    rcSettingChoice( 9,  "Swiss 3T3", "Swiss 3T3" ),
    rcSettingChoice( 10, "C. elegans", "C.elegans" ),
    rcSettingChoice( 11, "D. melanogaster", "D. melanogaster" ),
    rcSettingChoice( 12, "D. rerio", "D. rerio" ),
    rcSettingChoice( 0,  "Other",  "other type" ),
    rcSettingChoice( -1,  "Unspecified",  "unspecified type" ),
	rcSettingChoice( 0 , 0 , 0 )
};

// capture setting specs
const rcSettingInfoSpec generalSettings[] =
{
    {	// setting to show input image data file name
        cInputImageSourceNameSettingId,
		"image-sorce",
		"Input image source name",
		"Where experiment image input data comes from",
		eTextLine,
		0,
        0, 1
	},
	{	// setting to enter the data file name
		cFileNameSettingId,
		"file",
		"Result file name",
		"Where results are stored",
		eTextLine,
		0,
		inputFileFilterChoices, 1
	},
	{	// setting to enter the experiment title
		cTitleSettingId,
		"name",
		"Experiment title",
		"Title of the experiment",
		eTextLine,
		0,
		0, 2
	},
    {	// setting to enter the name of the experimenter
		cUserNameSettingId,
		"user",
		"User name",
		"Name of the experimenter",
		eTextLine,
		0,
		0, 2
	},
    {	// setting to enter cell treatment (chemical, physical etc.)
		cCellTreatmentSettingId,
		"cell-treatment",
		"Treatment 1",
		"Treatment 1 (chemical, physical etc.)",
		eTextLine,
		0,
		0, 2
	},
    {	// setting to enter cell treatment (chemical, physical etc.)
		cCellTreatment2SettingId,
		"cell-treatment2",
		"Treatment 2",
		"Treatment 2 (chemical, physical etc.)",
		eTextLine,
		0,
		0, 2
	},
    {	// setting to enter cell type
		cCellTypeSettingId,
		"cell-type",
		"Cell/organism",
		"Information about cell/organism",
		eTextChoice,
		0,
		cellTypeChoices, 2
	},
    {	// setting to enter experiment temperature
		cImagingModeSettingId,
		"imaging-mode",
		"Imaging mode",
		"Imaging mode",
		eTextChoice,
        0,
		imgModeChoices, 3
	},
    {	// setting to enter experiment temperature
		cMicroscopeMagnificationSettingId,
		"microscope-mag",
		"Magnification",
		"Lens magnification factor",
		eNumericChoice,
        0,
		lensMagChoices, 3
	},
    {	// setting to enter experiment temperature
		cOtherMagnificationSettingId,
		"other-mag",
		"2nd Magnification",
		"Other magnification factor",
		eNumericChoice,
        0,
		otherMagChoices, 3
	},

    {	// setting to enter experiment temperature
		cCellTemperatureSettingId,
		"cell-temperature",
		"Temperature",
		"Temperature in degrees Celsius",
		eNumericChoice,
        0,
		temperatureChoices, 4
	},
    {	// setting to enter experiment CO2 level
		cCellCO2SettingId,
		"cell-co2",
		"CO2",
		"CO2 level %",
		eNumericChoice,
        0,
		CO2Choices, 4
	},
    {	// setting to enter experiment O2 level
		cCellO2SettingId,
		"cell-o2",
		"O2",
		"O2 level %",
		eNumericChoice,
        0,
		O2Choices, 4
	},
	{	// setting to enter comments about the experiment title
		cCommentsSettingId,
		"description",
		"Comments",
		"Enter experiment comments here",
		eTextLine,
        0,
		0, 5
	},
    { 0 , 0 , 0 , 0 , 0 , 0 , 0, 0 }
};

// the capture setting category spec
const rcSettingCategorySpec generalSettingsSpec =
{
	"experiment-settings" , "Experiment" , "Experiment settings"	, generalSettings
};

// default constructor (creates a new, empty experiment)
rcExperimentImpl::rcExperimentImpl( void )
{
    unspecifySettings();

    rcLock lock( _settingMutex );
	rcSettingCategory generalSettings( this , &generalSettingsSpec );
	//_settings.push_back( generalSettings );

	rcEngine* engine = rcEngineFactory::getEngine();
	for (int i = 0; i < engine->getNSettingCategories(); i++)
		_settings.push_back( engine->getSettingCategory( i ) );

    _settingsEditable = true;
}

// this constructor loads a previously run experiment from a file.
rcExperimentImpl::rcExperimentImpl( const char* filename )
{
	rcUNUSED( filename );
}

// destructor
rcExperimentImpl::~rcExperimentImpl( void )
{
    rcLock lock( _trackMutex );
    _title = "Deleted";
    
    for ( uint32 i = 0; i < _trackGroups.size(); i++ ) {
        if ( _trackGroups[i] ) {
            delete _trackGroups[i];
            _trackGroups[i] = 0;
        }
    }
}

// Export current movie to QuickTime or .rfymov
int rcExperimentImpl::exportMovie( const std::string& outputFile,
                                   const rcMovieConverterOptions& baseOpt,
                                   rcExperimentFileFormat format,
                                   rcCarbonLock* lock, 
                                   rcProgressIndicator* progress )
{
    int err = 0;

    
    if ( !_inputImageSource.empty() ) {
        rcMovieConverterError error = eMovieConverterErrorUnknown;
        
        switch( format ) {
            case eExperimentQuickTimeMovieFormat:
            {
                rcMovieConverterOptionsQT opt( baseOpt );
                rcMovieConverterToQT converter( opt, lock, progress );
                error = converter.convert( _inputImageSource, outputFile );
            }
            break;

            case eExperimentNativeMovieFormat:
            {
                rcMovieConverterOptionsRfy opt( baseOpt );
                opt.channelConversion( rcSelectAll );
                rcMovieConverterToRfy converter( opt, lock, progress );
                error = converter.convert( _inputImageSource, outputFile );
            }
            break;

            default:
                break;
        }

        if ( error != eMovieConverterErrorOK ) {
            cerr << rcMovieConverter::getErrorString( error ) << endl;
            if ( error == eMovieConverterErrorUnsupported )
                err = ENOTSUP;
            else
                err = EIO;
        }
    } else {
        cerr << "Export error: no input image file defined" << endl;
    }

    return err;
}

// save the experiment data to a file
int rcExperimentImpl::saveTo( const char* filename,
                              rcExperimentFileFormat format,
                              const rcMovieConverterOptions& opt,
                              rcCarbonLock* lock, 
                              rcProgressIndicator* progress )
{
    rmAssert( filename != 0 );
    
    int err = 0;

    // Get application (generator) info
    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    const std::string comment = persistenceManager->generatorComment();
    const std::string ext = persistenceManager->fileFormatExportExtension( format );

    std::string nameWithExt = filename;
    // Add file name extension if necessary
    if ( nameWithExt.find( ext ) == std::string::npos )
        nameWithExt += ext;
             
    switch ( format ) {
        case eExperimentCSVFormat:
        {
            rcCSVExporter exporter( nameWithExt.c_str(), this, comment.c_str(), progress );
            err = exporter.exportExperiment();
        }
        break;

        case eExperimentNativeFormat:
        {
            // Remember filename
            _fileName = nameWithExt.c_str();
            rcNativeExporter exporter( nameWithExt.c_str(), this, format, comment.c_str(), progress );
            err = exporter.exportExperiment();
        }
        break;

#if 0
        case eExperimentNativeGraphicsFormat:
        {
            rcNativeExporter exporter( nameWithExt.c_str(), this, format, comment.c_str(), progress );
            err = exporter.exportExperiment();
        }
        break;
#endif

        case eExperimentNativeMovieFormat:
        case eExperimentQuickTimeMovieFormat:
            err = exportMovie( nameWithExt, opt, format, lock, progress );
            break;
            
        default:
            rmAssert( 0 );
            err = ENOTSUP;
            break;
    }

    return err;
}

// load an experiment from a tree and make it current.
// return number of errors
int rcExperimentImpl::loadExperiment( const rcXMLElementTree& tree, rcExperimentImportMode mode,
                                      rcEngineObserver* observer, rcProgressIndicator* progress )
{
    vector<std::string> importableSettings;

    unspecifySettings();
    
    switch ( mode ) {
        case eExperimentImportExperimentSettings:
        {
            rcLock lock( _settingMutex );
            // Load only settings from this experiment
            std::string expName( generalSettingsSpec._name );
            importableSettings.push_back( expName );
        }
            break;

        default:
            break;
    }
    
    rcNativeImporter importer( this, observer, progress );
    int errors = importer.importExperiment( tree, importableSettings );

    return errors;
}

// load template into newly created experiment
void rcExperimentImpl::loadTemplateData( const char* filename )
{
	rcUNUSED( filename );
}

// save current settings information to a template file
void rcExperimentImpl::saveTemplateData( const char* filename )
{
	rcUNUSED( filename );
}

// create a new track
rcTrackGroupImpl* rcExperimentImpl::createTrackGroup( const char* tag ,
											          const char* name ,
                                                      const char* description,
                                                      rcGroupSemantics type )
{
	rcTrackGroupImpl* trackGroup = new rcTrackGroupImpl( tag , name , description, type );
	_trackGroups.push_back( trackGroup );
	return trackGroup;
}

///////////////// rcExperiment implementation ///////////////////

// get the number of experiment settings categories
int rcExperimentImpl::getNSettingCategories( void )
{
    rcLock lock( _settingMutex );
	return _settings.size();
}

// get a settings category
rcSettingCategory rcExperimentImpl::getSettingCategory( int categoryNo )
{
    rcLock lock( _settingMutex );
	return _settings[ categoryNo ];
}

// get the number of experiment data track groups.
int rcExperimentImpl::getNTrackGroups( void )
{
    rcLock lock( _trackMutex );
	return _trackGroups.size();
}

// get a particular experiment data track group
rcTrackGroup* rcExperimentImpl::getTrackGroup( int trackNo )
{
    rcLock lock( _trackMutex );
    if ( trackNo <  int(_trackGroups.size()) )
        return _trackGroups[ trackNo ];
    else
        return 0;
}

// get the maximum track length
rcTimestamp rcExperimentImpl::getLength( void )
{
	return 0.0;
}

///////////////// rcSettingSource implementation /////////////////

// get the current value of a setting from the setting source
const rcValue rcExperimentImpl::getSettingValue( int settingId )
{
    rcLock lock( _settingMutex );
     
	switch (settingId)
	{
	case cFileNameSettingId:
		return _fileName;

	case cUserNameSettingId:
		return _userName;

	case cTitleSettingId:
		return _title;

	case cCommentsSettingId:
		return _comments;

    case cInputImageSourceNameSettingId:
        return _inputImageSource;

    case cMicroscopeMagnificationSettingId:
        return _microscopeMag;

    case cOtherMagnificationSettingId:
        return _otherMag;

    case cCellTypeSettingId:
        return _cellType;
            
    case cCellTreatmentSettingId:
        return _cellTreatment1;

    case cCellTreatment2SettingId:
        return _cellTreatment2;
        
    case cCellTemperatureSettingId:
        return _cellTemperature;

    case cCellCO2SettingId:
        return _cellCO2;
        
    case cCellO2SettingId:
        return _cellO2;

    case cImagingModeSettingId:
        return _imagingMode;
    }
    
	return "unknown setting";
}

// set the new value of a setting to the setting source.
void rcExperimentImpl::setSettingValue( int settingId , const rcValue& v )
{
    rcLock lock( _settingMutex );
    std::string value = v;
    
	switch (settingId)
	{
	case cFileNameSettingId:
		_fileName = value;
		break;

	case cUserNameSettingId:
		_userName = value;
		break;

	case cTitleSettingId:
		_title = value;
		break;

	case cCommentsSettingId:
		_comments = value;
		break;

    case cInputImageSourceNameSettingId:
        _inputImageSource = value;
        break;

    case cCellTreatmentSettingId:
        _cellTreatment1 = value;
        break;

    case cCellTreatment2SettingId:
        _cellTreatment2 = value;
        break;

    case cMicroscopeMagnificationSettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _microscopeMag = value;
    }
    break;

    case cOtherMagnificationSettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _otherMag = value;
    }
    break;

    case cCellTypeSettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _cellType = value;
    }
    break;
   
    case cCellTemperatureSettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _cellTemperature = value;
    }
    break;

    case cCellCO2SettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _cellCO2 = value;
    }
    break;
        
    case cCellO2SettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _cellO2 = value;
    }
    break;

    case cImagingModeSettingId:
    {
        // Empty string must be ignored for menu-based settings
        if ( value.size() )
            _imagingMode = value;
    }
    break;
    }
}

// return whether this setting is currently enabled.
bool rcExperimentImpl::isSettingEnabled( int settingId )
{
    rcLock lock( _settingMutex );
    rmUnused( settingId );
    
	return true;
}

// return whether this setting is current changable.
bool rcExperimentImpl::isSettingEditable( int settingId )
{
    rcLock lock( _settingMutex );
    
    if ( !_settingsEditable )
        return false;
    
	switch (settingId)
	{
        // Read-only setting, this is set by data import
        case cInputImageSourceNameSettingId:
            return false;

        // Read-only setting, this is set by file save dialog
        case cFileNameSettingId:
            return false;
            
        default:
            break;
	}

    return true;
}

// return whether this setting is currently persistable
bool rcExperimentImpl::isSettingPersistable( int settingId )
{
    rcLock lock( _settingMutex );
	switch (settingId)
	{
        case cInputImageSourceNameSettingId:
        case cFileNameSettingId:
            return true;
            
        default:
            break;
	}
    
    // Most settings are persisted in a movie header
	return false;
}

///////////////// other implementation /////////////////

// set editability for all settings
void rcExperimentImpl::editableSettings( bool enabled )
{
    rcLock lock( _settingMutex );
    _settingsEditable = enabled;
}

///////////////// private /////////////////

// Set settings to unspecified state
void rcExperimentImpl::unspecifySettings()
{
     rcLock lock( _settingMutex );
    
    _title = "Untitled";
    _cellTemperature = "-1.0";
    _microscopeMag = "-1.0";
    _otherMag = "-1.0";
    _cellCO2 = "-1.0";
    _cellO2 = "-1.0";
    _imagingMode = "Unspecified";
    _cellType = "Unspecified";   
}
