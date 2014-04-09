#ifndef rcTRACK_GROUP_WIDGET_H
#define rcTRACK_GROUP_WIDGET_H


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_timestamp.h>
#include <rc_model.h>

class QPushButton;
class Q3Frame;
class Q3VBox;

class rcTrackGroupEnableButton : public QPushButton
{
    Q_OBJECT

public:
    rcTrackGroupEnableButton( QWidget* parent , const char* name = 0 )
        : QPushButton( parent , name ) {}

    virtual QSize sizeHint () const
    {
        return QSize( 7 , 10 );
    }
};

class rcTrackGroupWidget : public QWidget
{
    Q_OBJECT

public:
    rcTrackGroupWidget( QWidget* parent , int trackGroupNo );
    ~rcTrackGroupWidget();

public slots:
    void setEnabled( bool isEnabled );
	void updateState( rcExperimentState state );
    void updateSettings();
    
signals:

protected:
    
private:
    // called to update the value label, etc.
    void updateTrackGroupDisplay( void );

    int             _trackGroupNo;
    QString         _trackGroupName;
    QPushButton*    _enableButton;
    Q3VBox*          _trackWidgets;
};


#endif // rcTRACK_GROUP_WIDGET_H
