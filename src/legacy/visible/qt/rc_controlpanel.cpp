#include <QtGui/QtGui>


#include "rc_modeldomain.h"
#include "rc_controlpanel.h"

/* XPM */
const char * circle_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 3 1",
"  c opaque",
". c navy",
"X c None",
/* pixels */
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXX       XXXXXXXX",
"XXXXXXXXXXXXXXX           XXXXXX",
"XXXXXXXXXXXXXX   XXXXXXX   XXXXX",
"XXXXXXXXXXXXX   XXXXXXXXX   XXXX",
"XXXXXXXXXXXXX   XXXXXXXXX   XXXX",
"XXXXXXXXXXXX   XXXXXXXXXXX   XXX",
"XXXXXXXXXXXX   XXXXXXXXXXX   XXX",
"XXXXXXXXXXXX   XXXXXXXXXXX   XXX",
"XXXXXXXXXXXX   XXXXXXXXXXX   XXX",
"XXXXXXXXXXXX   XXXXXXXXXXX   XXX",
"XXXXXXXXXXXXX   XXXXXXXXX   XXXX",
"XXXXXXXXXXXXX   XXXXXXXXX   XXXX",
"XXXXXXXXXXXXXX   XXXXXXX   XXXXX",
"XXXXXXXXXXXXXXX           XXXXXX",
"XXXXXXXXXXXXXXXXX       XXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXX........XXXXXXXXXXXXXXXXXXX",
"XXXXXXX......XXXXXXXXXXXXXXXXXXX",
"XXXXXXX......XXXXXXXXXXXXXXXXXXX",
"XXXXXX.......XXXXXXXXXXXXXXXXXXX",
"XXXXX........XXXXXXXXXXXXXXXXXXX",
"XXXX.........XXXXXXXXXXXXXXXXXXX",
"XXX.......XX.XXXXXXXXXXXXXXXXXXX",
"XXX......XXX.XXXXXXXXXXXXXXXXXXX",
"XXX.....XXXXXXXXXXXXXXXXXXXXXXXX",
"XXX....XXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
};

/* XPM */
const char *  circle_small_xpm[] = {
"16 16 4 1",
" 	c None",
".	c #EDA1A1",
"+	c #000000",
"@	c #1F0670",
"    ........    ",
"   ..........   ",
"  .....+++++..  ",
" .....++...++.. ",
"......+.....+...",
"......+.....+...",
"......+.....+...",
"......++...++...",
".......+++++....",
"..@@@@@.........",
"...@@@@.........",
"...@@@@.........",
" .@@@@@........ ",
" @@@..@.......  ",
"@@@..........   ",
"@@  ........    "};


/* XPM */
const char * back_xpm[] = {
/* columns rows colors chars-per-pixel */
"32 32 6 1",
"  c opaque",
". c blue",
"X c cyan",
"o c magenta",
"O c #c0c0c0",
"+ c None",
/* pixels */
"++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++",
"++++              ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++ oooooooooooo ++++++++++++++",
"++++              +++++ ++++++++",
"++++++++++++++++++++++. ++++++++",
"+++++++++++++++++++++.O ++++++++",
"++++++++++++++++++++.OO ...... +",
"+++++++++++++++++++.OOOOOOOOOX +",
"+++++++++++++++++++ XXXXXXXXXX +",
"++++++++++++++++++++ XX        +",
"+++++++++++++++++++++ X ++++++++",
"++++++++++++++++++++++  ++++++++",
"+++++++++++++++++++++++ ++++++++",
"++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++",
"++++++++++++++++++++++++++++++++"
};

/* XPM */
const char *  back_small_xpm[] = {
"16 16 2 1",
" 	c None",
".	c #000000",
"                ",
"                ",
"                ",
"        ...     ",
"       .   .    ",
"      .     .   ",
"     .      .   ",
"  .  .      .   ",
"  . .       .   ",
"  ... ..    .   ",
"  .....         ",
"  ....          ",
"  ...           ",
"  ..            ",
"  .             ",
"                "};


rcControlPanel::rcControlPanel( QWidget* parent, const char* name, Qt::WFlags f )
	: QFrame( parent, name )
{
    rcUNUSED( f );

    _lastState = eExperimentEmpty;
    _selectionState = false;

    rcModelDomain* domain = rcModelDomain::getModelDomain();

    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    QBoxLayout *topLayout = new QVBoxLayout( this , 0, 0 );
    QBoxLayout* layout = new QHBoxLayout( topLayout, 0, 0 );
    QPushButton* button;

    button = new QPushButton( "Select Entire Frame" , this , "select entire frame"  );
    QToolTip::add( button, "Select focus area" );
    layout->addWidget( button );
    connect( button , SIGNAL( clicked( void ) ) ,
            domain , SLOT( requestProcess( void ) ) );
    // TODO: button should activate only after succesful data import
    button->setEnabled( false );
    _SellectAllButton  = button;
    
    
    button = new QPushButton( "Analyze" , this , "analyzeButton"  );
    QToolTip::add( button, "Analyze focus area" );
    layout->addWidget( button );
    connect( button , SIGNAL( clicked( void ) ) ,
             domain , SLOT( requestProcess( void ) ) );
    // TODO: button should activate only after succesful data import
    button->setEnabled( false );
    _processButton = button;

    button = new QPushButton( "Start Capture" , this , "startButton" );
    QToolTip::add( button, "Specify file name and start video capture" );
	layout->addWidget( button );
	connect( button , SIGNAL( clicked( void ) ) , 
			 domain , SLOT( requestMovieSave( void ) ) );
    button->setEnabled( false );
    button->hide();
    _startButton = button;
    button->hide();
    
	button = new QPushButton( "Stop Capture" , this , "stopButton"  );
    QToolTip::add( button, "Stop video capture" );
	layout->addWidget( button );
	connect( button , SIGNAL( clicked( void ) ) ,
			 domain , SLOT( requestStop( void ) ) );
    button->setEnabled( false );
    button->hide();
    _stopButton = button;

    button = new QPushButton( "Stop Analysis" , this , "stopAnalysisButton"  );
    QToolTip::add( button, "Stop analysis" );
	layout->addWidget( button );
	connect( button , SIGNAL( clicked( void ) ) ,
			 domain , SLOT( requestStop( void ) ) );
    button->setEnabled( false );
    button->hide();
    _stopAnalysisButton = button;
	
    
    // Selection cursor time
	_cursorTime = new QLabel( this );
    _cursorTime->setTextFormat( Qt::PlainText );
	_cursorTime->setAlignment(Qt::AlignCenter);
	_cursorTime->setFrameStyle( Panel );
	_cursorTime->setFrameShadow( Sunken );
    _cursorTime->setText( timestampToString( 0.0 ) );
    QToolTip::add( _cursorTime, "Current selection time" );
	layout->addWidget( _cursorTime );

    // Total elapsed time
    _elapsedTime = new QLabel( this );
    _elapsedTime->setTextFormat( Qt::PlainText );
    _elapsedTime->setAlignment(Qt::AlignCenter );
    _elapsedTime->setFrameStyle( Panel );
    _elapsedTime->setFrameShadow( Sunken );
    _elapsedTime->setText( timestampToString( 0.0 ) );
    QToolTip::add( _elapsedTime, "Total elapsed time" );
    layout->addWidget( _elapsedTime );

    // Model connections
	connect( domain , SIGNAL( elapsedTime( const rcTimestamp& ) ) ,
			 this   , SLOT( updateElapsedTime( const rcTimestamp& ) ) );
    connect( domain , SIGNAL( cursorTime( const rcTimestamp& ) ) ,
             this   , SLOT( updateCursorTime( const rcTimestamp& ) ) );
    connect( domain , SIGNAL( newState( rcExperimentState ) ) ,
             this   , SLOT( updateState( rcExperimentState ) ) );
    connect( domain , SIGNAL( updateAnalysisRect( const rcRect& ) ) ,
             this   , SLOT( updateAnalysisRect( const rcRect& ) ) );
    connect( domain , SIGNAL( updateSettings() ) ,
             this   , SLOT( settingChanged() ) );
    connect( domain , SIGNAL( updateInputSource( int ) ) ,
             this   , SLOT( inputSource( int ) ) );

    connect( domain , SIGNAL( updateSelectionState( bool ) ) ,
             this   , SLOT( updateSelectionState ( bool ) ) );


}

rcControlPanel::~rcControlPanel()
{
    QToolTip::remove( _processButton );
    QToolTip::remove( _startButton );
    QToolTip::remove( _stopButton );
    QToolTip::remove( _elapsedTime );
    QToolTip::remove( _SellectAllButton );    
}

//public slots:
void rcControlPanel::updateElapsedTime( const rcTimestamp& time )
{
    _elapsedTime->setText( timestampToString( time ) );
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    //    rcExperimentAttributes attr = domain->getExperimentAttributes();
    //    if ( attr.liveInput )
    if (domain->operatingOnLiveInput ()) updateCursorTime();
}

void rcControlPanel::updateCursorTime( const rcTimestamp& time )
{
    if (time.secs() >= 0.0) {
        // Historical time
        _cursorTime->setText( timestampToString( time ) );
    } else {
        updateCursorTime();
    }
}

void rcControlPanel::updateState( rcExperimentState state )
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
//    rcExperimentAttributes attr = domain->getExperimentAttributes();

    if ( domain->operatingOnLiveInput () ) {
        // Camera input
        switch( state ) {
            case eExperimentEmpty:
                // Enable start button
                _stopButton->setEnabled( false );
                _stopButton->hide();
                _startButton->setEnabled( true );
                _startButton->show();
                break;
            case eExperimentEnded:
            case eExperimentLocked:
                // Disable stop button
                _stopButton->setEnabled( false );
                _stopButton->hide();
                _startButton->setEnabled( false );
                _startButton->show();
                break;
            case eExperimentRunning:
            case eExperimentPlayback:
                _startButton->setEnabled( false );
                _startButton->hide();
                _stopButton->setEnabled( true );
                _stopButton->show();
                break;
        }
    } else {
        // File input
        switch( state ) {
            case eExperimentEmpty:
                _SellectAllButton->setEnabled( true );
                _SellectAllButton->show();                
                _processButton->setEnabled( true );
                _processButton->show();
                // Disable start + stop button
                _startButton->setEnabled( false );
                _startButton->hide();
                _stopButton->setEnabled( false );
                _stopButton->hide();
                _stopAnalysisButton->setEnabled( false );
                _stopAnalysisButton->hide();

                break;
            case eExperimentEnded:
            case eExperimentLocked:
                _processButton->setEnabled( true );
                _processButton->show();
                _SellectAllButton->setEnabled( true );
                _SellectAllButton->show();                                
                // Disable start + stop button
                _startButton->setEnabled( false );
                _startButton->hide();
                _stopButton->setEnabled( false );
                _stopButton->hide();
                _stopAnalysisButton->setEnabled( false );
                _stopAnalysisButton->hide();

                break;
            case eExperimentRunning:
            case eExperimentPlayback:
                _processButton->setEnabled( false );
                _processButton->hide();
                _SellectAllButton->setEnabled( false );
                _SellectAllButton->show();                                
                // Enable stop button
                _startButton->setEnabled( false );
                _startButton->hide();
                _stopButton->setEnabled( false );
                _stopButton->hide();
                _stopAnalysisButton->setEnabled( true );
                _stopAnalysisButton->show();
                break;
        }
    }
   
    _lastState = state;
    updateAnalysisRect( _analysisRect );
}

void rcControlPanel::updateAnalysisRect( const rcRect& rect )
{
    _analysisRect = rect;
    // Enable/disable process button based on focus area size
    _processButton->setEnabled( _analysisRect.width() > 0 && _analysisRect.height() > 0 && _lastState != eExperimentRunning 
				&& _selectionState == false);
}


void rcControlPanel::settingChanged()
{
}

void rcControlPanel::updateSelectionState (bool v)
{
  cerr << "Slot Called" << endl;
  _selectionState = v;
}


// Input source has been changed
void rcControlPanel::inputSource( int i )
{
    // Hack alert: value 0 is file, greater than 0 camera
    if ( i > 0 ) {
        // Camera input
        _processButton->setEnabled( false );
        _processButton->hide();
        _stopAnalysisButton->setEnabled( false );
        _stopAnalysisButton->hide();
        
        _startButton->setEnabled( true );
        _startButton->show();
        _stopButton->setEnabled( false );
        _stopButton->hide();

    } else {
        // File input
         _processButton->setEnabled( true );
         _processButton->show();
         _stopAnalysisButton->setEnabled( false );
         _stopAnalysisButton->hide();
         _stopButton->setEnabled( false );
         _stopButton->hide();
         _startButton->setEnabled( false );
         _startButton->hide();

    }
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    rcExperimentState state = domain->getExperimentState();
    updateState( state );
        
    updateAnalysisRect( _analysisRect );

    updateCursorTime();
}

void rcControlPanel::updateCursorTime()
{
    // Current time
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    //    rcExperimentAttributes attr = domain->getExperimentAttributes();


    if ( domain->operatingOnLiveInput ()) {
        // Camera source
      if ( domain->storingLiveInput() )
            _cursorTime->setText( "video capture" );
        else
            _cursorTime->setText( "video preview" );
    } else {
        // File source
        _cursorTime->setText( " " );
    }
}

// private

static const double cSecsPerMinute = 60.0;
static const double cSecsPerHour = cSecsPerMinute * 60.0;
static const double cSecsPerDay = cSecsPerHour * 24.0;
static const double cMilliSecsPerSec = 1000.0;

QString rcControlPanel::timestampToString( const rcTimestamp& time )
{
	double dSecs = time.secs();
	int nDays = int(dSecs / cSecsPerDay);
	dSecs -= cSecsPerDay * nDays;
	int nHours = int(dSecs / cSecsPerHour);
	dSecs -= cSecsPerHour * nHours;
	int nMinutes = int(dSecs / cSecsPerMinute);
	dSecs -= cSecsPerMinute * nMinutes;
	int nSecs = int(dSecs);
	dSecs -= nSecs;
	int nMillis = int(dSecs * cMilliSecsPerSec);
	QString string;
	if (nDays == 0)
		string.sprintf( "%02d:%02d:%02d.%03d" , nHours , nMinutes , nSecs , nMillis );
	else
		string.sprintf( "%dd %02d:%02d:%02d.%03d" , nDays , nHours , nMinutes , nSecs , nMillis );
	return string;
}



// Select the whole image
void rcControlPanel::selectAll()
{
    rcModelDomain* domain = rcModelDomain::getModelDomain();
    uint32 maxX = domain->getExperimentAttributes().frameWidth;
    uint32 maxY = domain->getExperimentAttributes().frameHeight;
    
    rcRect wholeImage( 0, 0, maxX, maxY );
    
    // notify widgets who want to draw this rect
    rcModelDomain::getModelDomain()->notifyAnalysisRect( wholeImage );
}




