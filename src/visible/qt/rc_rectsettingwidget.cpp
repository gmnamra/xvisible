/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_rectsettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <string>
//Added by qt3to4:
#include <Q3GridLayout>

#if WIN32
using namespace std;
#endif

#include <qapplication.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <q3filedialog.h>

#include "rc_rectsettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

const int cSpinboxWidth = 80;

rcRectSettingWidget::rcRectSettingWidget( QWidget* parent, const rcSettingInfo& setting )
    : rcSettingWidget( parent, setting )
{
    _ignoreValueChanged = false;
    
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    int maxX = domain->getExperimentAttributes().frameWidth;
    int maxY = domain->getExperimentAttributes().frameHeight;

    QGridLayout *grid = new QGridLayout (mTopLayout, 3, 2);

    _xWidget = new QSpinBox( 0 , maxX , 1 , this , "x" );
    _xWidget->setPrefix( "x:" );
    _xWidget->setFixedWidth( cSpinboxWidth );
    grid->addWidget( _xWidget, 0, 1);

    _yWidget = new QSpinBox( 0 , maxY , 1 , this , "y" );
    _yWidget->setPrefix( "y:" );
    _yWidget->setFixedWidth( cSpinboxWidth );
    grid->addWidget( _yWidget, 0, 2);

    _wWidget = new QSpinBox( 0 , maxX , 1 , this , "w" );
    _wWidget->setPrefix( "w:" );
    _wWidget->setFixedWidth( cSpinboxWidth );
    grid->addWidget( _wWidget, 1, 1);

    _hWidget = new QSpinBox( 0 , maxY , 1 , this , "h" );
    _hWidget->setPrefix( "h:" );
    _hWidget->setFixedWidth( cSpinboxWidth );
    grid->addWidget( _hWidget, 1, 2);

    mTopLayout->addSpacing( 5 );
        
    _selectWidget = new QPushButton( "Entire" , this );
    _selectWidget->setFixedWidth( 40 );
    QToolTip::add( _selectWidget , "Click to select whole input frame for analysis" );
    grid->addWidget( _selectWidget , 0, 0);
    
    // use the setting description as a tool tip
    QToolTip::add( this , setting.getDescription() );

    // update the text widget with the current value of the setting.
    settingChanged();
    
    // route all spinbox's valueChanged signal to our single
    //	'valueChanged' slot
    connect( _xWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( void ) ) );
    connect( _yWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( void ) ) );
    connect( _wWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( void ) ) );
    connect( _hWidget , SIGNAL( valueChanged( int ) )  ,
             this , SLOT( valueChanged( void ) ) );

    // connect the "select all" button 'clicked' signal to our
    //  'selectAll' slot so we can start a file dialog.
    connect( _selectWidget , SIGNAL( clicked( void ) )  ,
             this , SLOT( selectAll( void ) ) );
    
    // connect this widget to be notified by the updateSettings()
    //	signal from the model domain
    connect( domain , SIGNAL( updateSettings() ) ,
             this   , SLOT( settingChanged() ) );

    // connect this widget to focus rect update signal
    connect( domain , SIGNAL( updateAnalysisRect( const rcRect& ) ) ,
             this   , SLOT( settingChanged( const rcRect& ) ) );
}

rcRectSettingWidget::~rcRectSettingWidget()
{
    QToolTip::remove( this );
    QToolTip::remove( _selectWidget );  
}

// public slots

// Update the setting value with the string just entered in
//	the QLineEdit widget.
void rcRectSettingWidget::valueChanged( void )
{
    if ( _ignoreValueChanged )
        return;

    setMaxValues();
	// set the new value from the state of the spin boxes
	int x = _xWidget->value();
	int y = _yWidget->value();
	int w = _wWidget->value();
	int h = _hWidget->value();
	rcRect newRect( x , y , w , h );

    rcModelDomain* domain = rcModelDomain::getModelDomain();
    int32 maxX = domain->getExperimentAttributes().frameWidth;
    int32 maxY = domain->getExperimentAttributes().frameHeight;

    // Throttle values
    if ( newRect.x() + newRect.width() > maxX )
        newRect.setWidth( maxX - newRect.x() );
    if ( newRect.y() + newRect.height() > maxY )
        newRect.setHeight( maxY - newRect.y() );

    mSetting.setValue( newRect );
	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
    // notify widgets who want to draw this rect
    rcModelDomain::getModelDomain()->notifyAnalysisRect( newRect );
}

// Update the text shown in the QLineEdit widget with the
//	current value in the setting.
void rcRectSettingWidget::settingChanged( void )
{
    setMaxValues();
	rcRect currentValue = mSetting.getValue();

    // Temporarily disable valueChanged updates to avoid
    // getting old setting values by accident
    _ignoreValueChanged = true;
	_xWidget->setValue( currentValue.x() );
	_yWidget->setValue( currentValue.y() );
	_wWidget->setValue( currentValue.width() );
	_hWidget->setValue( currentValue.height() );
    _ignoreValueChanged = false;

    // Update visibility/editability
    updateWidgetState();
}

// Update the text shown in the QLineEdit widget with the
// broadcasted value
void rcRectSettingWidget::settingChanged( const rcRect& currentValue )
{
    setMaxValues();
    mSetting.setValue( currentValue );
    
    // notify everybody about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
}


// Select the whole image
void rcRectSettingWidget::selectAll()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    uint32 maxX = domain->getExperimentAttributes().frameWidth;
    uint32 maxY = domain->getExperimentAttributes().frameHeight;

    rcRect wholeImage( 0, 0, maxX, maxY );

    settingChanged( wholeImage );
    // notify widgets who want to draw this rect
    rcModelDomain::getModelDomain()->notifyAnalysisRect( wholeImage );
}


// private

// set maximum widget values based on engine attributes
void rcRectSettingWidget::setMaxValues()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    uint32 maxX = domain->getExperimentAttributes().frameWidth;
    uint32 maxY = domain->getExperimentAttributes().frameHeight;

    _xWidget->setMaxValue( maxX );
    _yWidget->setMaxValue( maxY );
    _wWidget->setMaxValue( maxX );
    _hWidget->setMaxValue( maxY );
}


