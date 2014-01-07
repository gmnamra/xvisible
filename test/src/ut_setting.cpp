// ut_setting.cpp

#include <iostream>
#include <string>

#include <rc_setting.h>
#include <rc_unittest.h>


using namespace std;

const rcSettingChoice frameRateChoices[] = 
{
	rcSettingChoice(	30		,	"30"	, "capture at 30 fps" ),
	rcSettingChoice(	15		,	"15"	, "capture at 30 fps" ),
	rcSettingChoice(	10		,	"10"	, "capture at 30 fps" ),
	rcSettingChoice(	5		,	"5"		, "capture at 30 fps" ),
	rcSettingChoice(	2		,	"2"		, "capture at 30 fps" ),
	rcSettingChoice(	1		,	"1"		, "capture at 30 fps" ),
	rcSettingChoice( 0 , 0 , 0 )
};

const int cFrameRateSettingId = 0;
const int cInputFileNameSettingId = 1;
const int cEnableBlueFilterSettingId = 2;

const rcSettingInfoSpec captureSettings[] =
{
	{
		cFrameRateSettingId,
		"frame-rate",
		"Input frame rate",
		"Sets the capture frame rate",
		eMenuChoice,
		0,
		frameRateChoices
	},
	{
		cInputFileNameSettingId,
		"input-file",
		"Input file to analyze",
		"Choose a movie file to analyze",
		eFileChoice,
		0,
		0
	},
	{
		cEnableBlueFilterSettingId,
		"blue-filter-enabled",
		"Enable the blue filter",
		"Filters out blue from the analyzed video",
		eCheckbox,
		0,
		0
	},
	{ 0 , 0 , 0 , 0 , 0 , 0 , 0 }
};

const rcSettingCategorySpec captureSettingsSpec =
{
	"capture"	, "Capture"		, "Settings that control video capturing"	, captureSettings
};

class rcCaptureEngineSettingsExample : public rcSettingSource
{
public:
	rcCaptureEngineSettingsExample()
        {
            _settings = rcSettingCategory( this , &captureSettingsSpec );
            _frameRate = 30;
            _inputFile = "temp.mov";
            _blueFilterEnabled = true;
        }
    virtual ~rcCaptureEngineSettingsExample() { };

	// return the setting category for the capture engine
	rcSettingCategory getSettings( void )
        {
            return _settings;
        }

	//////////// rcSettingSource implementation //////////////////
	// get the current value of a setting from the setting source
	virtual const rcValue getSettingValue( int settingId )
        {
            switch (settingId)
            {
                case cFrameRateSettingId:
                    return _frameRate;

                case cInputFileNameSettingId:
                    return _inputFile;

                case cEnableBlueFilterSettingId:
                    return _blueFilterEnabled;
            }

            return 0;
        }

	// set the new value of a setting to the setting source.
	virtual void setSettingValue( int settingId, const rcValue& value )
        {
            switch (settingId)
            {
                case cFrameRateSettingId:
                    _frameRate = value;
                    break;
                    
                case cInputFileNameSettingId:
                {
                    _inputFile = value.stringValue();
                }
                break;
                
                case cEnableBlueFilterSettingId:
                    _blueFilterEnabled = value;
                    break;
            }
            
        }

	// return whether this setting is current enabled.
	virtual bool isSettingEnabled( int settingId )
        {
	        rcUNUSED( settingId );
            return true;
        }

	// return whether this setting is current changable.
	virtual bool isSettingEditable( int settingId )
        {
            rcUNUSED( settingId );
            return true;
        }

    // return whether this setting is currently persistable.
	virtual bool isSettingPersistable( int settingId )
        {
            rcUNUSED( settingId );
            return true;
        }
private:
	int			      _frameRate;
	std::string		  _inputFile;
	bool			  _blueFilterEnabled;
	rcSettingCategory _settings;
};

void dumpSettings( const rcSettingCategory& settings );


int ut_setting ( std::string& foo )
{
    rcUNUSED( foo );
	int mErrors = 0;

	rcValue intValue = 1;
	
	const rcValue floatValue = 2.0f;
	rcValue boolValue = false;
	rcValue nullValue;
	rcValue stringValue = "whatever";

	int i = intValue;
	mErrors += (i == 1);
	float f = floatValue;
	mErrors += (f == 2.0f);

	f = intValue;
	f = i;

	std::string str = floatValue;

	nullValue = intValue;
	nullValue = floatValue;
	nullValue = stringValue;
	{
		rcValue anotherString = "blah";
		nullValue = anotherString;
		if (nullValue == "blah")
			cout << nullValue << endl;
	}

	rcCaptureEngineSettingsExample	captureEngine;
	rcSettingCategory settings = captureEngine.getSettings();
	settings.dump( cout );
	rcValue blahValue = "blah.txt";
	try
	{
		settings[ "frame-rate" ] = 15;
		int frameRate = settings[ cFrameRateSettingId ];
		rcUNUSED( frameRate );
		settings[ cInputFileNameSettingId ] = 5.4f;
		std::string fileName = settings[ "input-file" ];

		settings.getSettingInfo( 1 ).setValue( "blah.txt" );
		settings.getSettingInfo( 2 ).setValue( false );
		rcValue rv = settings.getSettingInfo( 1 ).getValue();
		mErrors += (rv.equals (blahValue));
		
	}
	catch (exception& x)
	{
		cout << x.what();
	}

	settings.dump( cout );

	return 0;
}

void dumpSettings( const rcSettingCategory& settings )
{
	cout << endl << "Dumping " << settings.getName() << " settings (" << settings.getDescription() << ")" << endl;
	int nSettings = settings.getNSettings();
	for (int i = 0; i < nSettings; i++)
	{
		rcSettingInfo setting = settings.getSettingInfo( i );
		cout << "  " << setting.getDisplayName() << " = " << setting.getValue() << endl;
	}
}
