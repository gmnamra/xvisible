/******************************************************************************
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 ******************************************************************************/

#include <string.h>
//Added by qt3to4:
#include <QLabel>

#if WIN32
using namespace std;
#endif

#include <qapplication.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "rc_playbacksettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcPlaybackSettingWidget::rcPlaybackSettingWidget(QWidget* parent,
						 const rcSettingInfo& setting)
  : rcSettingWidget(parent, setting)
{
  rcModelDomain* domain = rcModelDomain::getModelDomain();

  // use the setting description as a tool tip
  QToolTip::add(mDisplayLabel, setting.getDescription());

  mTopLayout->addSpacing(cUIsettingLabelSpacing);

  _revWidget = new QPushButton("<<" ,this);
  rmAssert(_revWidget);
  _revWidget->setFixedWidth(60);
  QToolTip::add(_revWidget, "Click to play current video in reverse");
  mTopLayout->addWidget(_revWidget);
    
  mTopLayout->addSpacing(1);

  _stopWidget = new QPushButton("-" ,this);
  rmAssert(_stopWidget);
  _stopWidget->setFixedWidth(60);
  QToolTip::add(_stopWidget, "Click to stop playing video");
  mTopLayout->addWidget(_stopWidget);
    
  mTopLayout->addSpacing(1);

  _fwdWidget = new QPushButton(">>" ,this);
  rmAssert(_fwdWidget);
  _fwdWidget->setFixedWidth(60);
  QToolTip::add(_fwdWidget, "Click to play current video");
  mTopLayout->addWidget(_fwdWidget);
    
  mTopLayout->addSpacing(1);

  QLabel* label = new QLabel("Rate:", this);
  label->setFixedWidth(30);
  label->setAlignment(cUINoBreakAlignment);
  mTopLayout->addWidget(label);

  _speedWidget = new QSpinBox(1, rcSettingInfo::eCurrentSpeedMask, 1,
			      this, "speed");
  _speedWidget->setFixedWidth(80);
  _speedWidget->setSuffix("x");
  QToolTip::add(_speedWidget, "Use to adjust real time speed multiplier");
  mTopLayout->addWidget(_speedWidget);

  mTopLayout->addSpacing(5);

  // update the text widget with the current value of the setting.
  settingChanged();
  
  // route all spinbox's valueChanged signal to our single
  //	'valueChanged' slot
  connect(_speedWidget, SIGNAL(valueChanged(int)),
	  this,         SLOT(valueChanged(void)));

  // connect the "rev" button 'clicked' signal to our
  // 'revVideo' slot so we can start a reverse video playback.
  connect(_revWidget,   SIGNAL(clicked(void)),
	   this,        SLOT(revVideo(void)));
  
  // connect the "fwd" button 'clicked' signal to our
  // 'fwdVideo' slot so we can start a video playback.
  connect(_fwdWidget,   SIGNAL(clicked(void)),
	   this,        SLOT(fwdVideo(void)));
  
  // connect the "stop" button 'clicked' signal to our
  // 'stopVideo' slot so we can stop video playback.
  connect(_stopWidget,  SIGNAL(clicked(void)),
	   this,        SLOT(stopVideo(void)));
  
  // route all spinbox's valueChanged signal to our single
  // "valueChanged' slot
  connect(_speedWidget, SIGNAL(valueChanged(int)),
	   this,        SLOT(valueChanged(void)));
  
  // connect this widget to be notified by the updateSettings()
  // signal from the model domain
  connect(domain,       SIGNAL(updateSettings()),
	  this,         SLOT(settingChanged()));
}

rcPlaybackSettingWidget::~rcPlaybackSettingWidget()
{
  QToolTip::remove(this);
  QToolTip::remove(_revWidget);
  QToolTip::remove(_fwdWidget);
  QToolTip::remove(_stopWidget);
  QToolTip::remove(_speedWidget);
}

// Update the text shown in the widget with the
// current value in the setting.
void rcPlaybackSettingWidget::settingChanged()
{
  _currentValueBits = mSetting.getValue();
  int currentSpeed = _currentValueBits & rcSettingInfo::eCurrentSpeedMask;
  bool fwd = (_currentValueBits & rcSettingInfo::eCurrentStateFwd) != 0;
  bool rev = (_currentValueBits & rcSettingInfo::eCurrentStateRev) != 0;
  bool stopped = (_currentValueBits & rcSettingInfo::eCurrentStateStopped) != 0;

  rmAssert(fwd ^ rev); // Exactly one should be true

  _speedWidget->setValue(currentSpeed);
       
  // Disable the widget if the setting is currently not editable.
  if (!mSetting.isEditable())
    setEnabled(false);
  else {
    setEnabled(true);
    _revWidget->setEnabled(stopped);
    _fwdWidget->setEnabled(stopped);
    _speedWidget->setEnabled(stopped);
    _stopWidget->setEnabled(!stopped);
  }

  // hide the widget if the setting is currently not enabled.
  if (isHidden() && mSetting.isEnabled()) {
    show();
  }
  else if (!isHidden() && !mSetting.isEnabled()) {
    hide();
  }
}

// Update the setting value with the string just entered in
// the QLineEdit widget.
void rcPlaybackSettingWidget::valueChanged()
{
  int value = _speedWidget->value() & rcSettingInfo::eCurrentSpeedMask;
  value |= _currentValueBits & rcSettingInfo::eCurrentStateMask;
  _currentValueBits = value;
  mSetting.setValue(value);
  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}

void rcPlaybackSettingWidget::stopVideo()
{
  rmAssert((_currentValueBits & rcSettingInfo::eCurrentStateStopped) == 0);
    
  _currentValueBits |= rcSettingInfo::eCurrentStateStopped;

  mSetting.setValue(_currentValueBits);
  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}

void rcPlaybackSettingWidget::fwdVideo()
{
  rmAssert(_currentValueBits & rcSettingInfo::eCurrentStateStopped);
    
  _currentValueBits |= rcSettingInfo::eCurrentStateFwd;
  _currentValueBits &= ~(rcSettingInfo::eCurrentStateStopped |
			 rcSettingInfo::eCurrentStateRev);

  mSetting.setValue(_currentValueBits);
  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}

void rcPlaybackSettingWidget::revVideo()
{
  rmAssert(_currentValueBits & rcSettingInfo::eCurrentStateStopped);
    
  _currentValueBits |= rcSettingInfo::eCurrentStateRev;
  _currentValueBits &= ~(rcSettingInfo::eCurrentStateStopped |
			 rcSettingInfo::eCurrentStateFwd);

  mSetting.setValue(_currentValueBits);
  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}
