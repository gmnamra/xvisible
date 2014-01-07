/******************************************************************************
 * @file	rc_setting.h
 *
 *	This file declares classes that support an abstract representation
 *	of settings used to control the operation of various parts of the
 *	application.
 *
 *	An individual setting specifies a meta-data tag name,
 *	a display name, and description, as well as a generic specification
 *	of the type of UI widget that should display and control it.
 *
 *	A number of settings can be grouped into a settings category, which
 *	specifies a meta-data tag name, display name and description. 
 *
 *	A pure-virtual interface class is specified that is implemented by
 *	classes that have settings they wish to have controlled from the UI.
 *
 ******************************************************************************/

#ifndef _rcSETTING_H_
#define _rcSETTING_H_

#if WIN32
using namespace std;
#endif

#include "rc_value.h"
#include "rc_uitypes.h"
#include <rc_atomic.h>

/******************************************************************************
 *	Forward declarations
 ******************************************************************************/
class rcSettingInfo;
struct rcSettingCategorySpec;
struct rcSettingInfoSpec;
struct rcSettingChoice;


/******************************************************************************
 *	Constants
 ******************************************************************************/

// values returned by 'rcSettingInfo::getDisplayType()'.
enum rcSettingDisplayType
  {
    eTextLine = 0	 // setting is a single line of text.
    , eTextArea		 // setting may contain multiple lines of text
    , eCheckbox		 // setting is an on/off value
    , eRadioChoice	 // setting is a multi-choice value
    , eMenuChoice		 // setting is a multi-choice value
    , eFileChoice		 // setting is a file name (text + "browse" button)
    , eMultiFileChoice // setting is a semicolon separated list of file names (text + "browse" button)
    , eDirectoryChoice // setting is a directory name (text + "browse" button)
    , eRect			 // setting is a rectangle (text + "define" button)
    , ePoint		 	 // setting is a point (text + "define" button)
    , eSpinbox         // setting is a spinbox with a numeric value
    , eFramerateChoice // setting is a multi-choice with two connected text fields for custom choices
    , eFileSaveChoice  // setting is a file name (text + "browse" button)
    , ePlaybackChoice  // setting is a playback control ("rev" + "fwd" +
    // "stop" buttons and a speed selector)
    , eThumbWheel      // setting is a thumb wheel (a round knob)
    , eTextChoice      // setting is a multi-choice with a connected text field for custom choices
    , eNumericChoice   // setting is a multi-choice with a connected numeric field for custom choices
    , ePairOfChoices     // pair of settings
    , eRateChoice // setting is a multi-choice with two connected text fields for custom choices 
  };


/******************************************************************************
 *	rcSettingSource (pure-virtual interface definition)
 *
 *	Classes that need to have settings controlled implement this interface
 *	and provide a way to return one or more instances of rcSettingCategory.
 ******************************************************************************/
class RFY_API rcSettingSource
{
 public:
  // virtual dtor is required
  virtual ~rcSettingSource() { };
    
  // get the current value of a setting from the setting source
  virtual const rcValue getSettingValue( int settingId ) = 0;

  // set the new value of a setting to the setting source.
  virtual void setSettingValue( int settingId , const rcValue& value ) = 0;

  // return whether this setting is currently enabled (visible).
  virtual bool isSettingEnabled( int settingId ) = 0;

  // return whether this setting is current changable.
  virtual bool isSettingEditable( int settingId ) = 0;

  // return whether this setting should be persisted
  virtual bool isSettingPersistable( int settingId ) = 0;
};


/******************************************************************************
 *	rcSettingInfo
 *
 *	Describes a single setting.  Specifies the meta-data tag name,
 *	display type, name and desciption, and provides the methods
 *	needed to get and set the setting's value.
 ******************************************************************************/
class RFY_API rcSettingInfo
{
 public:
  // default constructor
  rcSettingInfo( void );

  // create a new setting given a setting source and a specification.
  rcSettingInfo( rcSettingSource* source , const rcSettingInfoSpec* spec );

  // get the meta-data element tag for the setting
  const char* getTag( void ) const;

  // get the setting's display name
  const char* getDisplayName( void ) const;

  // the description of the choice (for tooltip/help?)
  const char* getDescription( void ) const;

  // the description of the choice (for tooltip/help?)
  const void* getXArgs( void ) const;

  // return whether the setting is current enabled (visible).
  const bool isEnabled( void ) const;

  // return whether the setting is current editable.
  const bool isEditable( void ) const;

  // return whether this setting should be persisted
  const bool isPersistable( void ) const;
    
  // get the setting ui type (see below)
  const int getDisplayType( void ) const;

  // get the settings group id (settings with identitical id are put in a groupBox
  const int getGroupId ( void ) const;

  // for multi-choice settings, get the number of choices
  const int getNChoices( void ) const;

  // for multi-choice settings, return a particular choice
  const rcSettingChoice getChoice( int choiceNo ) const;
	
  // get the value
  const rcValue getValue( void ) const;

  // set the value
  void setValue( const rcValue& value ) const;

  // assignment operator sets the underlying value
  const rcSettingInfo& operator = ( const rcValue& value ) const { setValue( value ); return *this; }

  // cast to rcValue gets the underlying value
  operator const rcValue() const { return getValue(); }

  // cast to int gets the underlying value
  operator int() const { return getValue(); }

  // cast to int gets the underlying value
  operator float() const { return getValue(); }

  // cast to int gets the underlying value
  operator bool() const { return getValue(); }

  // cast to int gets the underlying value
  operator std::string() const { return getValue(); }

  enum {
    eCurrentSpeedMask     = 0x00FFFFFF,
    eCurrentStateMask     = 0xE0000000,
    eCurrentStateRev      = 0x80000000,
    eCurrentStateFwd      = 0x40000000,
    eCurrentStateStopped  = 0x20000000
  };

 private:
  // assert we're initialized.
  void assertInitialized( void ) const;

  rcSettingSource*			_source;
  const rcSettingInfoSpec*	_spec;
  int							_nChoices;
};


/******************************************************************************
 *	rcSettingInfoSpec
 *
 *	Convenience structure to allow static  specification of a setting.
 ******************************************************************************/
struct RFY_API rcSettingInfoSpec
{
  int							_id;
  const char*				_tag;
  const char*				_name;
  const char*				_description;
  int		         			_displayType;
  void*                       _xArgs;   // Extra, widget specific args
  const rcSettingChoice*		_choices;
  int                                   _gid; 
};


/******************************************************************************
 *	rcSpinBoxArgs
 *
 *	Definition of the special arguments required to create a rcSpinBox.
 *	
 ******************************************************************************/
struct RFY_API rcSpinBoxArgs
{
  double                 multiplier;
  bool                   valueDivisor;
  bool                   updateMultiplier;
  int                    minValue;
  int                    maxValue;
};  

struct rcSpinPairBoxArgs
{
  rcIPair                    minMaxOne;
  rcIPair                    minMaxTwo;
};  

/******************************************************************************
 *	rcThumbWheelArgs
 *
 *	Definition of the special arguments required to create a rcThumbWheel.
 *	
 ******************************************************************************/
struct rcThumbWheelArgs
{
  int32 minValue;         // Minimum range value
  int32 maxValue;         // Maximum range value
  double transmissionRatio; // Ratio of mouse movement to value change
  bool orientation;         // Widget orientation: horizontal = 0, vertical = 1
  bool tracking;            // Mouse tracking
};

/******************************************************************************
 *	rcTextAreaArgs
 *
 *	Definition of the special arguments required to create a rcTextArea.
 *	
 ******************************************************************************/
struct rcTextAreaArgs
{
  int32 minLines;         // Minimum number of text lines to show
};

/******************************************************************************
 *	rcSettingCategory
 *
 *	Definition of a settings category, which contains a collection of
 *	individual settings and specifies its own meta-data tag, display
 *	name, and description.
 ******************************************************************************/
class RFY_API rcSettingCategory
{
 public:
  // constructor for a null (empty) category
  rcSettingCategory( void );

  // constructor given a setting source and a specification.
  rcSettingCategory( rcSettingSource* source , const rcSettingCategorySpec* spec );

  // get the meta-data element tag for the category
  const char* getTag() const;

  // get the short "display name" of the category
  const char* getName() const;

  // get the long description of the category
  const char* getDescription() const;

  // returns the number of settings in this category.
  const int getNSettings( void ) const;

  // return the info for a setting given the index
  const rcSettingInfo getSettingInfo( int settingNo ) const;

  // return the info for a setting given the setting tag
  const rcSettingInfo getSettingInfo( const std::string& settingTag ) const;

  // dump state of setting category to output stream
  void dump( ostream& stream ) const;

  // indexing operator given setting index
  const rcSettingInfo operator [] ( int settingNo ) const
  {
    return getSettingInfo( settingNo );
  }

  // indexing operator give setting tag
  const rcSettingInfo operator [] ( const std::string& settingTag ) const
  {
    return getSettingInfo( settingTag );
  }

 private:
  // assert we're initialized.
  void assertInitialized( void ) const;

  rcSettingSource*				_source;
  const rcSettingCategorySpec*	_spec;
  int								_nSettings;
};


/******************************************************************************
 *	rcSettingCategorySpec
 *
 *	Convenience structure to allow static specification of a settings catagory
 ******************************************************************************/
struct rcSettingCategorySpec
{
  const char*				_tag;
  const char*				_name;
  const char*				_description;
  const rcSettingInfoSpec*	_settingInfoSpecs;
};


/******************************************************************************
 *	rcSettingChoice
 *
 *	Settings that are of type eRadioChoice or eMenuChoice need to specify
 *	the choices.
 ******************************************************************************/
struct RFY_API rcSettingChoice
{
  public:
  // ctors
  rcSettingChoice() :
    _value( 0 ),
       _text( 0 ),
       _description( 0 ) { };
    
  rcSettingChoice( int value, const char* text, const char* descr ) :
    _value( float(value) ),
       _text( text ),
       _description( descr ) { };

  rcSettingChoice( float value, const char* text, const char* descr ) :
    _value( value ),
       _text( text ),
       _description( descr ) { };
    
  rcSettingChoice( double value, const char* text, const char* descr ) :
    _value( float(value) ),
       _text( text ),
       _description( descr ) { };

  // virtual dtor is required
  virtual ~rcSettingChoice() { };

  // the value that corresponds with this choice
  virtual int getValue( void ) const
  {
    return (int)_value; // Coerce the float to an int
  }

  // the value that corresponds with this choice
  virtual float getFloatValue( void ) 
  {
    return _value;
  }
    
  // the text displayed for the choice
  virtual const char* getText( void ) const
  {
    return _text;
  }

  // the description of the choice (for tooltip/help?)
  virtual const char* getDescription( void ) const
  {
    return _description;
  }

  float			_value;
  const char*	_text;
  const char*	_description;
};

#endif // _rcSETTING_H_
