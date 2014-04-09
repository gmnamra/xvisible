/******************************************************************************
*   Copyright (c) 2002-2003 Reify Corp. All Rights reserved.
*   $Id: rc_filechoicesettingwidget.cpp 7191 2011-02-07 19:38:55Z arman $
*
*
******************************************************************************/

#include <string>

#if WIN32
using namespace std;
#endif

#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_fileutils.h>

#include "rc_filechoicesettingwidget.h"
#include "rc_modeldomain.h"
#include "rc_appconstants.h"

rcFileChoiceSettingWidget::rcFileChoiceSettingWidget( QWidget* parent, const rcSettingInfo& setting )
	: rcSettingWidget( parent, setting )
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
    connect( domain , SIGNAL( imageImport( void ) )  ,
             this , SLOT( imageImport( void ) ) );
    connect( domain , SIGNAL( tifDirImport( void ) )  ,
             this , SLOT( tifDirImport( void ) ) );

    connect( domain , SIGNAL( movieImport( void ) )  ,
             this , SLOT( movieImport( void ) ) );

    connect( domain , SIGNAL( stkImport( void ) )  ,
	             this , SLOT( stkImport( void ) ) );
    
}

rcFileChoiceSettingWidget::~rcFileChoiceSettingWidget()
{
    QToolTip::remove( mFilenameWidget );
}


// browse for single movie file
void rcFileChoiceSettingWidget::singleFileBrowse( void )
{

    string currentValue = mSetting.getValue();
    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    std::string tmp = persistenceManager->fileFormatImportFilter( mFormat );
    QString filter (tmp.c_str());
    tmp = persistenceManager->fileFormatImportCaption( mFormat );
    QString caption (tmp.c_str());

	QString s = QFileDialog::getOpenFileName(
                    currentValue.c_str(),
                    filter,
                    0,
                    caption,
                    caption );

	if (s != QString::null)
	{
		// if a file was selected, update the setting
		mSetting.setValue( s.latin1() );

		// notify everybody (including us) about the change
		rcModelDomain::getModelDomain()->notifySettingChange();
	}
}


// browse for single movie file
void rcFileChoiceSettingWidget::recentMovieFile( const QString& s )
{
   
    string currentValue = mSetting.getValue();
	if (s != QString::null)
	{
		// if a file was selected, update the setting
		mSetting.setValue( s.latin1() );
        
		// notify everybody (including us) about the change
		rcModelDomain::getModelDomain()->notifySettingChange();
	}
}

// browse for multiple image files
void rcFileChoiceSettingWidget::multiFileBrowse( void )
{

    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    std::string tmp = persistenceManager->fileFormatImportFilter( mFormat );
    QString filter (tmp.c_str());
    tmp = persistenceManager->fileFormatImportCaption( mFormat );
    QString caption (tmp.c_str());

	QStringList list = QFileDialog::getOpenFileNames(
        filter,
        "",
        0,
        caption,
        caption );

    // Iterate all selected files
    QString selectedFiles;
    QStringList::Iterator it = list.begin();
    while (it != list.end())
	{
        QString& s = *it;
        if (s != QString::null)
        {
            // Create a list of semicolon separated file names
            selectedFiles += s;
            selectedFiles +=";";
        }
        ++it;
	}

    if (!selectedFiles.isEmpty())
	{
		// if a file was selected, update the setting
       mSetting.setValue( selectedFiles.latin1() );

		// notify everybody (including us) about the change
		rcModelDomain::getModelDomain()->notifySettingChange();
    }
}

// browse for directory
void rcFileChoiceSettingWidget::directoryBrowse( void )
{
	string currentValue = mSetting.getValue();
	QString s = QFileDialog::getExistingDirectory(
                    currentValue.c_str(),
                    0,
                    mSetting.getTag(),
                    mSetting.getDisplayName() );

	if (s != QString::null)
	{
		// if a directory was selected, update the setting
		mSetting.setValue( s.latin1() );

		// notify everybody (including us) about the change
		rcModelDomain::getModelDomain()->notifySettingChange();
	}
}


// browse for directory
void rcFileChoiceSettingWidget::directoryBrowse4Tiffs( void )
{

    rcPersistenceManager* persistenceManager = rcPersistenceManagerFactory::getPersistenceManager();
    std::string tmp = persistenceManager->fileFormatImportFilter( mFormat );
    QString filter (tmp.c_str());
    tmp = persistenceManager->fileFormatImportCaption( mFormat );
    QString caption (tmp.c_str());

    string currentValue = mSetting.getValue();
    QString s = QFileDialog::getExistingDirectory(
						  currentValue.c_str(),
						  0,
						  "Select a Directory of Tiff Image Files",
						  "Select a Directory of Tiff Image Files");

    if (s != QString::null)
	{
	  vector<std::string> files;

	  //@note gets tiff files: .tif or .TIF
          std::string tmp ((char *) (s.data ()));
	  rfGetDirEntries (tmp, files);

	  if (files.size())
	    {
	      // Iterate all selected files
	      QString selectedFiles;
	      vector<std::string>::iterator it = files.begin();
	      while (it != files.end())
		{
                    QString sg (it->c_str ());

		  if (sg != QString::null)
		    {
		      // Create a list of semicolon separated file names
		      selectedFiles += sg;
		      selectedFiles +=";";
		    }
		  ++it;
		}

	      if (!selectedFiles.isEmpty())
		{
		  // if a file was selected, update the setting
		  mSetting.setValue( selectedFiles.latin1() );

		  // notify everybody (including us) about the change
		  rcModelDomain::getModelDomain()->notifySettingChange();
		}
	    }
	}
}

// public slots

void rcFileChoiceSettingWidget::browseRequest( void )
{
    switch (mSetting.getDisplayType())
    {
        case eFileChoice:       // setting is a file name (text + "browse" button)
            singleFileBrowse();
            break;

        case eMultiFileChoice:  // setting is a semicolon separated list of file names (text + "browse" button)
            multiFileBrowse();
            break;

        case eDirectoryChoice:  // setting is a directory name (text + "browse" button)
            directoryBrowse4Tiffs();
            break;

        default:
            rmExceptionMacro ( "unknown setting type" );
    }
}

// Update the setting value with the string just entered in
//	the QLineEdit widget.
void rcFileChoiceSettingWidget::valueChanged( void )
{
	mSetting.setValue( mFilenameWidget->text().latin1() );

	// notify everybody about the change
	rcModelDomain::getModelDomain()->notifySettingChange();
}

// Update the text shown in the QLineEdit widget with the
//	current value in the setting.
void rcFileChoiceSettingWidget::settingChanged( void )
{
	string currentValue = mSetting.getValue();
	mFilenameWidget->setText( currentValue.c_str() );

    // Update visibility/editability
    updateWidgetState();
}

void rcFileChoiceSettingWidget::updateState( rcExperimentState state )
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

void rcFileChoiceSettingWidget::movieImport( void )
{
    mFormat = eExperimentQuickTimeMovieFormat;
    switch (mSetting.getDisplayType())
    {
        case eFileChoice:       // setting is a file name (text + "browse" button)
            singleFileBrowse();
            break;

        default:
            // Ignore request if this is not a single-file widget
            break;
    }
}

void rcFileChoiceSettingWidget::stkImport( void )
{

    mFormat = eExperimentMolDevSTKFormat;
    switch (mSetting.getDisplayType())
    {
        case eFileChoice:       // setting is a file name (text + "browse" button)
            singleFileBrowse();
            break;

        default:
            // Ignore request if this is not a single-file widget
            break;
    }
}

void rcFileChoiceSettingWidget::imageImport( void )
{
    mFormat = eExperimentQuickTimeImageFormat;
    switch (mSetting.getDisplayType())
    {
        case eMultiFileChoice:  // setting is a semicolon separated list of file names (text + "browse" button)
            multiFileBrowse();
            break;

        default:
            // Ignore request if this is not a multi-file widget
            break;
    }
}

void rcFileChoiceSettingWidget::tifDirImport( void )
{
    mFormat = eExperimentQuickTimeImageFormat;
    switch (mSetting.getDisplayType())
    {
        case eDirectoryChoice:  // setting is a directory name (text + "browse" button)
            directoryBrowse4Tiffs();
            break;

        default:
            // Ignore request if this is not a multi-file widget
            break;
    }
}
