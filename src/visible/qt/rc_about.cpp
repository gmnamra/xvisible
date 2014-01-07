

#include "rc_About.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

#include "rc_appconstants.h"
#include "rc_modeldomain.h"
#include "rc_menubar.h"
#include "rc_cellinfowidget.h"


rcAbout::rcAbout( QWidget* parent, const char* name )
: QDialog( parent, name, true, 0 )
{
    if ( !name )
		setName( "rcAbout" );

    //    setIcon( QPixmap::fromMimeSource( "logo.png" ) );

    setBackgroundOrigin( QDialog::AncestorOrigin );
    rcAboutLayout = new QGridLayout( this, 1, 1, 11, 6, "rcAboutLayout"); 

    mB_Close = new QPushButton( this, "mB_Close" );

    rcAboutLayout->addWidget( mB_Close, 9, 3 );
    QSpacerItem* spacer = new QSpacerItem( 384, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    rcAboutLayout->addMultiCell( spacer, 9, 9, 0, 2 );

    mHBL_LogoBig = new QHBoxLayout( 0, 0, 6, "mHBL_LogoBig"); 
    QSpacerItem* spacer_2 = new QSpacerItem( 16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    mHBL_LogoBig->addItem( spacer_2 );

    mL_LogoBig = new QLabel( this, "mL_LogoBig" );
    //    mL_LogoBig->setPaletteBackgroundColor( QColor( 76, 105, 126 ) );
    //    mL_LogoBig->setFrameShape( QLabel::Box );
    //    mL_LogoBig->setLineWidth( 4 );
    //    mL_LogoBig->setPixmap( QPixmap::fromMimeSource( "movida_about_logo.png" ) );
    //    mL_LogoBig->setScaledContents( FALSE );
    //    mHBL_LogoBig->addWidget( mL_LogoBig );
    //    QSpacerItem* spacer_3 = new QSpacerItem( 16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    //    mHBL_LogoBig->addItem( spacer_3 );

    rcAboutLayout->addMultiCellLayout( mHBL_LogoBig, 0, 0, 0, 3 );

    mTE_Box = new QTextEdit( this, "mTE_Box" );
  //  mTE_Box->setTextFormat( QTextEdit::AutoAll);
  //  mTE_Box->setWordWrap( QTextEdit::WidgetWidth );
    mTE_Box->setReadOnly( FALSE );
//    mTE_Box->setAutoFormatting( int( QTextEdit::AutoAll ) );

    rcAboutLayout->addMultiCellWidget( mTE_Box, 8, 8, 0, 3 );
    QSpacerItem* spacer_4 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    rcAboutLayout->addItem( spacer_4, 7, 0 );

    mL_GPL = new QLabel( this, "mL_License" );
//    mL_GPL->setAlignment( int( QLabel::AlignVCenter ) );

    rcAboutLayout->addMultiCellWidget( mL_GPL, 6, 6, 0, 3 );
    QSpacerItem* spacer_5 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    rcAboutLayout->addItem( spacer_5, 5, 0 );
    QSpacerItem* spacer_6 = new QSpacerItem( 20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed );
    rcAboutLayout->addItem( spacer_6, 1, 2 );

    mLN_Line0 = new QFrame( this, "mLN_Line0" );
    mLN_Line0->setFrameShape( QFrame::HLine );
    mLN_Line0->setFrameShadow( QFrame::Sunken );
    mLN_Line0->setFrameShape( QFrame::HLine );

    rcAboutLayout->addMultiCellWidget( mLN_Line0, 2, 2, 0, 3 );
    QSpacerItem* spacer_7 = new QSpacerItem( 20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed );
    rcAboutLayout->addItem( spacer_7, 3, 1 );

    mF_Box = new QFrame( this, "mF_Box" );
    mF_Box->setFrameShape( QFrame::StyledPanel );
    mF_Box->setFrameShadow( QFrame::Sunken );
    mF_BoxLayout = new QGridLayout( mF_Box, 1, 1, 11, 6, "mF_BoxLayout"); 

    mVBL_SF = new QVBoxLayout( 0, 0, 6, "mVBL_SF"); 

    mL_Hosted = new QLabel( mF_Box, "mL_Hosted" );
  //  mL_Hosted->setAlignment( int( QLabel::AlignCenter ) );
    mVBL_SF->addWidget( mL_Hosted );

    //    mUL_SFLogo = new URLLabel( mF_Box, "mUL_SFLogo" );
    //    mVBL_SF->addWidget( mUL_SFLogo );

    //    mUL_Project = new URLLabel( mF_Box, "mUL_Project" );
    //    mVBL_SF->addWidget( mUL_Project );

    mF_BoxLayout->addMultiCellLayout( mVBL_SF, 0, 2, 3, 3 );

    mL_Version = new QLabel( mF_Box, "mL_Version" );

    mF_BoxLayout->addWidget( mL_Version, 2, 0 );

    mL_Title = new QLabel( mF_Box, "mL_Title" );

    mF_BoxLayout->addWidget( mL_Title, 0, 0 );

    mL_Copy = new QLabel( mF_Box, "mL_Copy" );

    mF_BoxLayout->addWidget( mL_Copy, 1, 0 );
    QSpacerItem* spacer_8 = new QSpacerItem( 100, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    mF_BoxLayout->addItem( spacer_8, 1, 2 );

    //    mUL_Mail = new URLLabel( mF_Box, "mUL_Mail" );

    //    mF_BoxLayout->addWidget( mUL_Mail, 1, 1 );

    rcAboutLayout->addMultiCellWidget( mF_Box, 4, 4, 0, 3 );
    languageChange();
    resize( QSize(494, 449).expandedTo(minimumSizeHint()) );
 //   clearWState( WState_Polished );

    // signals and slots connections
    connect( mB_Close, SIGNAL( clicked() ), this, SLOT( accept() ) );
#if 0
	mUL_SFLogo->setPixmap( QPixmap::fromMimeSource( "sflogo.png" ) );
	mUL_SFLogo->setScaledContents( FALSE );

	mUL_SFLogo->setAlignment( int( QLabel::AlignCenter ) );
	mUL_Project->setAlignment( int( QLabel::AlignCenter ) );

	mUL_Project->setHighlightedColor(QColor(0,0,0));
	mUL_Project->setSelectedColor(QColor(114,105,126));

	mUL_Mail->setHighlightedColor(QColor(0,0,0));
	mUL_Mail->setSelectedColor(QColor(114,105,126));

	mUL_Project->setUseTips(true);
	mUL_Mail->setUseTips(true);
	mUL_SFLogo->setUseTips(true);
#endif
}

rcAbout::~rcAbout()
{
}

void rcAbout::languageChange()
{
  rcModelDomain* domain = rcModelDomain::getModelDomain();
  QString expirationDate = QString (domain->getLicenseExpirationTime().c_str());
	QString svnversion (" Beta Version ");
  QString title( cAppName );
  QString version( cVersionName );
    
  QString copyRight = "<p>Copyright (c) 2002-2008 Reify Corp. All rights reserved.";
  QString otherRight = "<p>Copyright (c) 1987-1992 Numerical Recipes Software";

  setCaption( tr( "About Visible" ) );

	mTE_Box->setText (copyRight + otherRight);

  mB_Close->setText( tr( "&Close" ) );
  mB_Close->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    
	mL_GPL->setText( tr( "This software is licensed to you under the terms of End User License Agreement for Reify Corporation Software " ) );

	mL_Title->setText( "Visible" );
	mL_Copy->setText( "(C) 2002 - 2008 Reify Corporation" );
	QString licenseText = "<p>License expiration date: ";
	licenseText = licenseText + expirationDate;
	mL_Version->setText(version + svnversion +  licenseText);
}

