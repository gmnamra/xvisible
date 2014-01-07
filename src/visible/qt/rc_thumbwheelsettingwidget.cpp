/*
 *
 *$Header $
 *$Id $
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */

#include "rc_thumbwheelsettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QLabel>
#include <math.h>


rcThumbWheelSettingWidget::rcThumbWheelSettingWidget( QWidget *parent, const rcSettingInfo& setting )
        : rcSettingWidget( parent, setting )
{
    mWheel = new QSlider (0, 360, 1, 0 , Qt::Horizontal,  this );
    mTopLayout->addWidget( mWheel );

    mTopLayout->addSpacing( cUIsettingLabelSpacing );
    // Label to display setting value
    mValueLabel = new QLabel( " " , this );
    mTopLayout->addWidget( mValueLabel );
        
    //    mWheel->setMinimumHeight( 10 );
    //    mWheel->setMaximumWidth( 180 );
   
    const rcThumbWheelArgs* argsP = static_cast<const rcThumbWheelArgs*>(setting.getXArgs());

	if ( argsP )
	  {
	  mWheel->setMinValue ( argsP->minValue);
	  mWheel->setMaxValue (argsP->maxValue );
	  mWheel->setOrientation( (argsP->orientation ? Qt::Vertical : Qt::Horizontal ) );
	}

    // use the setting description as a tool tip
	QToolTip::add( this , setting.getDescription() );

    // route thumbwheel's valueChanged signal to our single
	// valueChanged slot
	connect( mWheel , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( int ) ) );

    // connect this widget to be notified by the updateSettings()
	// signal from the model domain
	rcModelDomain* domain = rcModelDomain::getModelDomain();
	connect( domain , SIGNAL( updateSettings() ) ,
			 this   , SLOT( settingChanged() ) );

	//	connect( domain , SIGNAL( updateAnalysisRectRotation( float ) ) ,
	//			 this   , SLOT( updateRotation( float ) ) );

    
    updateLabel();
}

/*!
  Destructs the wheel.
*/

rcThumbWheelSettingWidget::~rcThumbWheelSettingWidget()
{
    QToolTip::remove( this );
}

// slots

/*!
  Makes valueChanged() available as a slot.
*/

void rcThumbWheelSettingWidget::valueChanged( int v )
{
    bool currentValue = mSetting.getValue();
    
	if ( v != currentValue ) {
		mSetting.setValue( v );
        updateLabel();
    }

    // Do not notify, this is a "real-time" operation and sepcial signal is used
	//rcModelDomain::getModelDomain()->notifySettingChange();
}

/*!
  Setting has changed, update widget. Available as a slot.
*/

void rcThumbWheelSettingWidget::settingChanged()
{
    disconnect( mWheel , SIGNAL( valueChanged( int ) )  ,
                this , SLOT( valueChanged( int ) ) );
        
	int32 currentValue = mSetting.getValue();
    if ( mWheel->value() != currentValue ) {
        mWheel->setValue( currentValue );
        updateLabel();
    }

    // Update visibility/editability
    updateWidgetState();

    connect( mWheel , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( int ) ) );
}

/*!
  Setting has been changed, update widget. Available as a slot.
*/

void rcThumbWheelSettingWidget::updateRotation( float angle )
{
    disconnect( mWheel , SIGNAL( valueChanged( int ) )  ,
                this , SLOT( valueChanged( int ) ) );
        
	mWheel->setValue( static_cast<int32>(angle) );
    updateLabel();
    connect( mWheel , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( int ) ) );
}

// private

/*!
  Update value display label
*/

void rcThumbWheelSettingWidget::updateLabel()
{
    QString str;
    
    // Update value display
    str.sprintf( "%i", mWheel->value() );
    mValueLabel->setText( str );
}
