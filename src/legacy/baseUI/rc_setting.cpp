/******************************************************************************
* @file	rc_setting.cpp
*
*	This file defines classes that support an abstract representation
*	of settings used to control the operation of various parts of the
*	application.
******************************************************************************/

#include <rc_setting.h>

#if WIN32
using namespace std;
#endif


/******************************************************************************
 *	rcSettingCategory definition
 ******************************************************************************/

// constructor for a null (empty) category
rcSettingCategory::rcSettingCategory( void )
{
  _source = 0;
  _spec = 0;
  _nSettings = 0;
}

// constructor taking a spec and a setting source
rcSettingCategory::rcSettingCategory( rcSettingSource* source , const rcSettingCategorySpec* spec )
{
  _source = source;
  _spec = spec;

  for (_nSettings = 0; ; _nSettings++)
    {
      const rcSettingInfoSpec* infoSpec = &(spec->_settingInfoSpecs[ _nSettings ]);
      if (infoSpec->_name == 0)
	break;
    }
}

// get the meta-data element tag for the category
const char* rcSettingCategory::getTag() const
{
  assertInitialized();
  return _spec->_tag;
}

// get the short "display name" of the category
const char* rcSettingCategory::getName() const
{
  assertInitialized();
  return _spec->_name;
}

// get the long description of the category
const char* rcSettingCategory::getDescription() const
{
  assertInitialized();
  return _spec->_description;
}

// returns the number of settings in this category.
const int rcSettingCategory::getNSettings( void ) const
{
  assertInitialized();
  return _nSettings;
}

// return the info for a setting given the index
const rcSettingInfo rcSettingCategory::getSettingInfo( int settingNo ) const
{
  assertInitialized();
  if ((settingNo < 0) || (settingNo >= _nSettings)) {
    char buf[512];
    snprintf( buf, rmDim(buf), "rcSettingCategory::getSettingInfo: index %i out-of-bounds in category %s",
	      settingNo, getName() );
    throw general_exception( buf );
  }

  const rcSettingInfoSpec* settingInfoSpec = &(_spec->_settingInfoSpecs[ settingNo ]);
  return rcSettingInfo( _source , settingInfoSpec );
}

// return the info for a setting given the setting tag
const rcSettingInfo rcSettingCategory::getSettingInfo( const std::string& settingTag ) const
{
  assertInitialized();
  for (int i = 0; i < _nSettings; i++)
    {
      const rcSettingInfoSpec* settingInfoSpec = &(_spec->_settingInfoSpecs[ i ]);
      if (settingTag == settingInfoSpec->_tag)
	return rcSettingInfo( _source , settingInfoSpec );
    }
  char buf[512];
  snprintf( buf, rmDim(buf), "rcSettingCategory::getSettingInfo: tag %s not found from category %s",
	    settingTag.c_str(), getName() );
  throw general_exception( buf );
}

// dump state of setting category to output stream
void rcSettingCategory::dump( ostream& stream ) const
{
  stream << endl << getName() << " settings (" << getDescription() << ")" << endl;
  int nSettings = getNSettings();
  for (int i = 0; i < nSettings; i++)
    {
      rcSettingInfo setting = getSettingInfo( i );
      stream << "  " << setting.getDisplayName() << " = " << setting.getValue() << endl;
    }
}

// assert we're initialized.
void rcSettingCategory::assertInitialized( void ) const
{
  if ((_source == 0) || (_spec == 0))
    throw general_exception( "rcSettingCategory::assertInitialized failed" );
}


/******************************************************************************
 *	rcSettingInfo definition
 ******************************************************************************/

// default constructor
rcSettingInfo::rcSettingInfo( void )
{
  _source = 0;
  _spec = 0;
}

// definition of a setting
rcSettingInfo::rcSettingInfo( rcSettingSource* source , const rcSettingInfoSpec* spec )
{
  _source = source;
  _spec = spec;
  _nChoices = 0;

  if (spec->_choices != 0)
    {
      for ( ; ; _nChoices++)
	{
	  const rcSettingChoice* choice = &(spec->_choices[ _nChoices ]);
	  if (choice->_text == 0)
	    break;
	}
    }
}

// get the meta-data element tag for the setting
const char* rcSettingInfo::getTag( void ) const
{
  return _spec->_tag;
}

// get the setting's display name
const char* rcSettingInfo::getDisplayName( void ) const
{
  return _spec->_name;
}

// the description of the choice (for tooltip/help?)
const char* rcSettingInfo::getDescription( void ) const
{
  return _spec->_description;
}

// Any extra arguments required to create the widget
const void* rcSettingInfo::getXArgs( void ) const
{
  return _spec->_xArgs;
}

// return whether the setting is current enabled.
const bool rcSettingInfo::isEnabled( void ) const
{
  return _source->isSettingEnabled( _spec->_id );
}

// return whether the setting is current editable.
const bool rcSettingInfo::isEditable( void ) const
{
  return _source->isSettingEditable( _spec->_id );
}

// return whether the setting is currenttly persistable
const bool rcSettingInfo::isPersistable( void ) const
{
  return _source->isSettingPersistable( _spec->_id );
}

// get the setting ui type (see below)
const int rcSettingInfo::getDisplayType( void ) const
{
  return _spec->_displayType;
}

// get the setting ui type (see below)
const int rcSettingInfo::getGroupId ( void ) const
{
  return _spec->_gid;
}

// for multi-choice settings, get the number of choices
const int rcSettingInfo::getNChoices( void ) const
{
  return _nChoices;
}

// for multi-choice settings, return an iterator of ISettingChoice's
const rcSettingChoice rcSettingInfo::getChoice( int choiceNo ) const
{
  if ((choiceNo < 0) || (choiceNo >= _nChoices))
    throw general_exception( "rcSettingInfo::getChoice: index out-of-bounds" );

  return _spec->_choices[ choiceNo ];
}

// get the value
const rcValue rcSettingInfo::getValue( void ) const
{
  return _source->getSettingValue( _spec->_id );
}

// set the value
void rcSettingInfo::setValue( const rcValue& value ) const
{
  _source->setSettingValue( _spec->_id , value );
}

// assert we're initialized.
void rcSettingInfo::assertInitialized( void ) const
{
  if ((_source == 0) || (_spec == 0))
    throw general_exception( "rcSettingInfo::assertInitialized failed" );
}

