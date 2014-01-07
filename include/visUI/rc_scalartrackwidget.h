#ifndef rcSCALAR_TRACK_WIDGET_H
#define rcSCALAR_TRACK_WIDGET_H

#include <qwidget.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLabel>

#include <rc_timestamp.h>
#include <rc_model.h>

class QLabel;
class QCheckBox;
class QPushButton;

class rcScalarTrackWidget : public QWidget
{
    Q_OBJECT

public:
    rcScalarTrackWidget( QWidget* parent , int trackGroupNo , int trackNo );
    ~rcScalarTrackWidget();

public slots:
    void setEnabled( bool isEnabled );
	void updateElapsedTime( const rcTimestamp& time );
	void updateCursorTime( const rcTimestamp& time ); 
	void updateState( rcExperimentState state );
    void updateDisplay( void );
    void updateSettings( void );
    
signals:

protected:
    void mousePressEvent( QMouseEvent* mouseEvent );
    void mouseReleaseEvent( QMouseEvent* mouseEvent );
    void mouseMoveEvent( QMouseEvent* mouseEvent );
    void keyPressEvent( QKeyEvent* keyEvent );
    void keyReleaseEvent( QKeyEvent* keyEvent );
    
private:
    // called to update the value label, etc.
    void updateTrackDisplay( void );
    
    int             _trackGroupNo;
    int             _trackNo;
    QCheckBox*      _checkbox;
    QLabel*         _legend;
    QLabel*         _name;
    QLabel*         _value;
    rcTimestamp     _elapsedTime;
    rcTimestamp     _cursorTime;
    rcTimestamp     _currentTime;
};

#endif // rcSCALAR_TRACK_WIDGET_H
