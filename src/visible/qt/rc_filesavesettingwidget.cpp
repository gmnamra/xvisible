/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_filesavesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <string>

#if WIN32
using namespace std;
#endif

#include <qapplication.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qtooltip.h>
#include <q3filedialog.h>

#include "rc_filesavesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcFileSaveSettingWidget::rcFileSaveSettingWidget(QWidget* parent,
						 const rcSettingInfo& setting )
  : rcSettingWidget(parent, setting)
{
  mFilenameWidget = new QLineEdit( this , setting.getTag() );
  mTopLayout->addWidget( mFilenameWidget );

  mTopLayout->addSpacing( 5 );

  // use the setting description as a tool tip
  QToolTip::add( mFilenameWidget , setting.getDescription() );

  mFilenameWidget->setReadOnly( true );

  // update the text widget with the current value of the setting.
  settingChanged();

  // connect this widget to be notified by the updateSettings()
  //	signal from the model domain
  rcModelDomain* domain = rcModelDomain::getModelDomain();
  connect( domain , SIGNAL( updateSettings() ) ,
	   this   , SLOT( settingChanged() ) );

  connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
	   this   , SLOT( updateState( rcExperimentState ) ) );

  // Import signals (from File menu)
  connect( domain , SIGNAL( movieSave( void ) )  ,
	   this , SLOT( movieSave( void ) ) );
}

rcFileSaveSettingWidget::~rcFileSaveSettingWidget()
{
    QToolTip::remove( mFilenameWidget );
}

// public slots

void rcFileSaveSettingWidget::browseRequest( void )
{
    QString defaultName( mSetting.getValue().stringValue ().c_str() );

  if ( defaultName.isEmpty() ) {
      rcModelDomain* domain = rcModelDomain::getModelDomain();
      // Use experiment title as default name
      rcExperimentAttributes attr = domain->getExperimentAttributes();
      defaultName = attr.title.c_str();
      // We need to replace slashes with something else
      defaultName.replace( QChar('/'), "-" );
  }
  rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    std::string tmp = persistenceManager->fileFormatExportFilter(eExperimentNativeMovieFormat );
    QString filter (tmp.c_str());
    tmp = persistenceManager->fileFormatExportCaption(eExperimentNativeMovieFormat );
    QString caption (tmp.c_str());


  QString s = Q3FileDialog::getSaveFileName(defaultName,
                                           filter,
                                           0,
                                           mSetting.getTag(),
                                           caption,
                                           &filter);
  if (s != QString::null)
  {
    std::string tmp = persistenceManager->fileFormatExportExtension(eExperimentNativeMovieFormat );
    QString ext (tmp.c_str());
    // Force to lowercase for case-insensitive comparison
    QString sLower = s.lower();
    ext = ext.lower();
    if (!sLower.endsWith( ext ))
      s += ext;

    // warn if file already exists
    if (QFile::exists(s))
    {
      if (QMessageBox::warning( 0,
				cAppName,
				"File already exists: overwrite?",
				QMessageBox::Ok , QMessageBox::Cancel)
	  == QMessageBox::Cancel)
	return;
    }

    // if a file was selected, update the setting
    mSetting.setValue( s.latin1() );

    // notify everybody (including us) about the change
    rcModelDomain::getModelDomain()->notifySettingChange();
  }
}

// Update the setting value with the string just entered in
//	the QLineEdit widget.
void rcFileSaveSettingWidget::valueChanged( void )
{
  mSetting.setValue( mFilenameWidget->text().latin1() );

  // notify everybody about the change
  rcModelDomain::getModelDomain()->notifySettingChange();
}

// Update the text shown in the QLineEdit widget with the
//	current value in the setting.
void rcFileSaveSettingWidget::settingChanged( void )
{
	string currentValue = mSetting.getValue();
	mFilenameWidget->setText( currentValue.c_str() );

    updateWidgetState();
}

void rcFileSaveSettingWidget::updateState( rcExperimentState state )
{
  // The widget is read-only so it can always remain enabled

  switch( state ) {
  case eExperimentEmpty:          // experiment was created but not run
    setEnabled( true );
    break;

  case eExperimentRunning:        // experiment is running
    setEnabled( true );
    break;

  case eExperimentPlayback:       // experiment is playing back
    setEnabled( true );
    break;

  case eExperimentEnded:          // experiment has ended
    setEnabled( true );
    break;

  case eExperimentLocked:         // experiment has ended and been saved
    setEnabled( true );
    break;

  default:
    break;
  }
}

void rcFileSaveSettingWidget::movieSave( void )
{
  browseRequest();
}
