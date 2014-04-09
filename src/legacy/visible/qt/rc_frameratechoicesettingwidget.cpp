/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_frameratechoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/


#include <QtGui/QtGui>
#include <QtCore/QtCore>



#include "rc_frameratechoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

const int cFrameRateWidth = 80;

rcFramerateChoiceSettingWidget::rcFramerateChoiceSettingWidget( QWidget* parent,
                                                                const rcSettingInfo& setting ) :
        rcSettingWidget( parent, setting, false )
{
    // We create our own laytout without using base class layout
    QBoxLayout* vLayout = new QVBoxLayout( this );
	QBoxLayout* layout = new QHBoxLayout( vLayout );

	QLabel* label = new QLabel( setting.getDisplayName() , this );
	label->setFixedWidth( cUIsettingLabelWidth );

	label->setAlignment( cUIWordBreakAlignment );
	layout->addWidget( label );
	layout->addSpacing( cUIsettingLabelSpacing );

	_widget = new QComboBox(false,  this , setting.getTag() );
	int nChoices = setting.getNChoices();
	for (int i = 0; i < nChoices; i++)
	{
		rcSettingChoice choice = setting.getChoice( i );
        QString text( choice.getText() );
        // Add one space before text for better layout
		_widget->insertItem( QString( " %1" )
                             .arg( text ) );
	}
	layout->addWidget( _widget );
    layout->addSpacing( 10 );
    
	QToolTip::add( _widget , setting.getDescription() );

    // A text widget for arbitrary frame rates
    _framerateWidget = new rcLineEdit( this , setting.getTag() );
    _framerateWidget->setFixedWidth( cFrameRateWidth );
    QToolTip::add( _framerateWidget , setting.getDescription() );
    float value = mSetting.getValue();
    _framerateWidget->setText( QString( "%1" )
                               .arg( value ) );
    layout->addWidget( _framerateWidget );
    layout->addStretch();    

    vLayout->addSpacing( 10 );
    // A text widget for arbitrary frame intervals
    // Frame interval in seconds is 1/framerate
    layout = new QHBoxLayout( vLayout );
    label = new QLabel( "Frame interval (seconds)" , this );
    label->setFixedWidth( cUIsettingLabelWidth );
    label->setAlignment( Qt::AlignRight | Qt::AlignVCenter | cUIWordBreakAlignment );
    layout->addWidget( label );
    layout->addSpacing( cUIsettingLabelSpacing );
    
    _intervalWidget = new rcLineEdit( this , "frame interval" );
    _intervalWidget->setFixedWidth( cFrameRateWidth );
    QToolTip::add( _intervalWidget , "Sets the input frame interval" );
    value = mSetting.getValue();
    value = 1 / value;
    _intervalWidget->setText( QString( "%1" )
                              .arg( value ) );
    layout->addWidget( _intervalWidget );
  
    
	layout->addStretch();
	// update the combo box with the current value of the setting.
	settingChanged();

    connect( _widget , SIGNAL( activated( int ) ) , this , SLOT( choiceSelected( int ) ) );
        
    connect( _intervalWidget , SIGNAL( textCommited( const QString& ) ) ,
             this    , SLOT( intervalChanged( const QString& ) ) );
    
    connect( _framerateWidget , SIGNAL( textCommited( const QString& ) ) ,
             this    , SLOT( settingChanged( const QString& ) ) );
        
	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );
}

rcFramerateChoiceSettingWidget::~rcFramerateChoiceSettingWidget()
{
    QToolTip::remove( _widget );
    QToolTip::remove( _framerateWidget );
    QToolTip::remove( _intervalWidget );
}

// The combo box widget's activated(int) signal is connected
//	to this slot, so when a menu item is selected we need to
//	set the setting with the choice value corresponding to the
//	index of menu item.
void rcFramerateChoiceSettingWidget::choiceSelected( int choiceIndex )
{
	int nChoices = mSetting.getNChoices();
	if ((choiceIndex >= 0) && (choiceIndex < nChoices))
	{
		rcSettingChoice choice = mSetting.getChoice( choiceIndex );
		mSetting.setValue( choice.getFloatValue() );
	}

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
//	combo box item based on the value of the setting.
void rcFramerateChoiceSettingWidget::settingChanged( void )
{
	float currentValue = mSetting.getValue();
	int nChoices = mSetting.getNChoices();
	int i = findChoice( currentValue );

    if ( i >= nChoices ) {
        // find "custom" value 0
        i = findChoice( 0 );
    }
	if (i < nChoices) 
		_widget->setCurrentItem( i );

    _framerateWidget->setText( QString( "%1" )
                          .arg( currentValue ) );
    _intervalWidget->setText( QString( "%1" )
                              .arg( 1.0/currentValue ) );

    // Update visibility/editability
    updateWidgetState();
}

// This slot causes us to update the currently displayed
//  combo box item based on the value of the setting.
void rcFramerateChoiceSettingWidget::settingChanged( const QString& value )
{
    bool ok = false;
    float currentValue = value.toFloat( &ok );   

    if ( ok && currentValue > 0 ) {
        mSetting.setValue( currentValue );
    } else {
        // Illegal value, revert to previous value
    }
    // notify everybody about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
// combo box item based on the value of the interval setting.
void rcFramerateChoiceSettingWidget::intervalChanged( const QString& value )
{
    bool ok = false;
    float currentValue = value.toFloat( &ok );

    if ( ok && currentValue > 0 ) {
        // Use interval value to set frame rate
        // TODO: frame rate does not remain accurate if frame interval is
        // several minutes. Higher accuracy needed.
        currentValue = 1.0 / currentValue;
        mSetting.setValue( currentValue );
    } else {
        // Illegal value, revert to previous value
    }
    // notify everybody about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
}

// private

int rcFramerateChoiceSettingWidget::findChoice( float value )
{
    int nChoices = mSetting.getNChoices();
    int i;

    for (i = 0; i < nChoices; i++)
    {
        rcSettingChoice choice = mSetting.getChoice( i );
        if (choice.getFloatValue() == value)
            break;
    }

    return i;
}
        
//////////////// Rate Choice




rcRateChoiceSettingWidget::rcRateChoiceSettingWidget( QWidget* parent,
                                                                const rcSettingInfo& setting ) :
  rcSettingWidget( parent, setting, false )
{
  // We create our own laytout without using base class layout
  QBoxLayout* vLayout = new QVBoxLayout( this );
  QBoxLayout* layout = new QHBoxLayout( vLayout );

  QLabel* label = new QLabel( setting.getDisplayName() , this );
  label->setFixedWidth( cUIsettingLabelWidth );

  label->setAlignment( cUIWordBreakAlignment );
  layout->addWidget( label );
  layout->addSpacing( cUIsettingLabelSpacing );

  _widget = new QComboBox(false,  this , setting.getTag() );
  int nChoices = setting.getNChoices();
  for (int i = 0; i < nChoices; i++)
    {
      rcSettingChoice choice = setting.getChoice( i );
      QString text( choice.getText() );
      // Add one space before text for better layout
      _widget->insertItem( QString( " %1" )
			   .arg( text ) );
    }
  layout->addWidget( _widget );
  layout->addSpacing( 10 );
  QToolTip::add( _widget , setting.getDescription() );

  // A text widget for arbitrary frame rates
  _rateWidget = new rcLineEdit( this , setting.getTag() );
  _rateWidget->setFixedWidth( cFrameRateWidth / 2 );
  QToolTip::add( _rateWidget , setting.getDescription() );
  float value = mSetting.getValue();
  _rateWidget->setText( QString( "%1" ).arg( value ) );
  layout->addWidget( _rateWidget );
  layout->addStretch();    

  vLayout->addSpacing( 10 );
    
  layout->addStretch();
  // update the combo box with the current value of the setting.
  settingChanged();

  connect( _widget , SIGNAL( activated( int ) ) , this , SLOT( choiceSelected( int ) ) );
        
  connect( _rateWidget , SIGNAL( textCommited( const QString& ) ) ,
	   this    , SLOT( settingChanged( const QString& ) ) );
        
  // connect this widget to be notified by the updateSettings()
  //	signal from the model domain
  rcModelDomain* domain = rcModelDomain::getModelDomain();
  connect( domain , SIGNAL( updateSettings() ) ,
	   this   , SLOT( settingChanged() ) );
}

rcRateChoiceSettingWidget::~rcRateChoiceSettingWidget()
{
  QToolTip::remove( _widget );
  QToolTip::remove( _rateWidget );
}

// The combo box widget's activated(int) signal is connected
//	to this slot, so when a menu item is selected we need to
//	set the setting with the choice value corresponding to the
//	index of menu item.


void rcRateChoiceSettingWidget::choiceSelected( int choiceIndex )
{
  int nChoices = mSetting.getNChoices();
  if ((choiceIndex >= 0) && (choiceIndex < nChoices))
    {
      rcSettingChoice choice = mSetting.getChoice( choiceIndex );
      mSetting.setValue( choice.getValue() );
    }

  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}

// This slot causes us to update the currently displayed
//	combo box item based on the value of the setting.
void rcRateChoiceSettingWidget::settingChanged( void )
{
  int currentValue = mSetting.getValue();
  int nChoices = mSetting.getNChoices();
  int i = findChoice( currentValue );

  if ( i >= nChoices ) {
    // find "custom" value 0
    i = findChoice( 0 );
  }
  if (i < nChoices) 
    _widget->setCurrentItem( i );

  _rateWidget->setText( QString( "%1" )
			     .arg( currentValue ) );
  
  // Update visibility/editability
  updateWidgetState();
}

// This slot causes us to update the currently displayed
//  combo box item based on the value of the setting.
void rcRateChoiceSettingWidget::settingChanged( const QString& value )
{
  bool ok = false;
  float currentValue = value.toFloat( &ok );   
  if ( ok && currentValue > 0 ) {
    mSetting.setValue( currentValue );
  } else {
    // Illegal value, revert to previous value
  }
  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}


// private

int rcRateChoiceSettingWidget::findChoice( float value )
{
  int nChoices = mSetting.getNChoices();
  int i;

  for (i = 0; i < nChoices; i++)
    {
      rcSettingChoice choice = mSetting.getChoice( i );
      if (choice.getFloatValue() == value)
	break;
    }

  //@note hack to return custom
  if (i == nChoices) return 1; 
  return i;
}
        
