/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_spinboxsettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <string>

#include <QtGui/QtGui>
#include <QtCore/QtCore>

#if WIN32
using namespace std;
#endif

#include "rc_spinboxsettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"
#include <math.h>

rcSpinBox::rcSpinBox(double multiplier, bool valueDivisor,
		     int minValue, int maxValue, int step,
		     QWidget* parent, const char* name)
  : QSpinBox(minValue, maxValue, step, parent, name), _multiplier(multiplier),
    _valueDivisor(valueDivisor)
{
  rmAssert(_multiplier);
}

int rcSpinBox::mapTextToValue(bool* ok)
{
  bool myOk;

  if (!ok)
    ok = &myOk;

  double inputNum = cleanText().toDouble(ok);

  if (!*ok)
    return 0;

  rmAssert(_multiplier != 0);
  inputNum = inputNum / _multiplier;

  if (_valueDivisor) {
    if (inputNum == 0) {
      *ok = 0;
      return 0;
    }
    else {
      return (int)round(1/inputNum); // Assume QT insures value is in range
    }
  }

  return (int)round(inputNum); // Assume QT insures value is in range
}

QString rcSpinBox::mapValueToText(int v)
{

  QString retVal;

  if (_valueDivisor)
  {
    if (v == 0) // Prevent divide-by-zero error
      v = 1;

    retVal = QString::number(_multiplier/v);
  }
  else
    retVal = QString::number(_multiplier*v);

  return retVal;
}

rcSpinboxSettingWidget::rcSpinboxSettingWidget( QWidget* parent, const rcSettingInfo& setting )
  : rcSettingWidget( parent, setting ), _rcSpinboxWidget(0)
{
	rcModelDomain* domain = rcModelDomain::getModelDomain();

	rcSpinBoxArgs* argsP = (rcSpinBoxArgs*)setting.getXArgs();
	if (argsP)
	{
	  _spinboxWidget = _rcSpinboxWidget =
	    new rcSpinBox(argsP->multiplier, argsP->valueDivisor, argsP->minValue,
			  argsP->maxValue, 1 , this , "val" );
	  _spinboxWidget->setFixedWidth( 90 );
      if ( !argsP->updateMultiplier )
          _rcSpinboxWidget = 0;
	}
	else
	{
	  _spinboxWidget = new QSpinBox( INT_MIN, INT_MAX , 1 , this , "val" );
	  _spinboxWidget->setFixedWidth( 70 );
	}
	mTopLayout->addWidget( _spinboxWidget );

    // Dummy label to force left-justified layout
    QLabel* label = new QLabel( "" , this );
    mTopLayout->addWidget( label );
    
	// use the setting description as a tool tip
	QToolTip::add( this , setting.getDescription() );

	// update the text widget with the current value of the setting.
	settingChanged();

    // route all spinbox's valueChanged signal to our single
	//	'valueChanged' slot
	connect( _spinboxWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( void ) ) );
    
	// connect this widget to be notified by the updateSettings()
	//	signal from the model domain
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );

	if (_rcSpinboxWidget) {
        // This is an rcSpinBox needing multiplier updates.
        // Hook up its special signal
        connect( domain, SIGNAL( updateMultiplier(double) ),
                 this, SLOT( settingChanged(double) ) );
    }
}

rcSpinboxSettingWidget::~rcSpinboxSettingWidget()
{
    QToolTip::remove( this );
}

// public slots

// Update the setting value with the string just entered in
//	the QLineEdit widget.
void rcSpinboxSettingWidget::valueChanged( void )
{
	int value = _spinboxWidget->value();

	mSetting.setValue( value );
	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// Update the text shown in the widget with the
// current value in the setting.
void rcSpinboxSettingWidget::settingChanged( void )
{
	int currentValue = mSetting.getValue();

	_spinboxWidget->setValue( currentValue );

    // Update visibility/editability
    updateWidgetState();
}

// Update the text shown in the widget with the
// current value in the setting.
void rcSpinboxSettingWidget::settingChanged( double multiplier )
{
  rmAssert(_rcSpinboxWidget);

  _rcSpinboxWidget->setMultiplier(multiplier);

  /* Do stupid trick to convince QT to display setting value based
   * upon the new multiplier.
   */
  int currentValue = mSetting.getValue();
  int stupidValue = (currentValue) != 1 ? 1 : 2;

  _rcSpinboxWidget->setValue( stupidValue );
  _rcSpinboxWidget->setValue( currentValue );
}
