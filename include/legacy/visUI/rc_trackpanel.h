#ifndef RC_TRACK_PANEL_H
#define RC_TRACK_PANEL_H


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_timestamp.h>
#include <rc_model.h>

class QLabel;
class QWidget;

class rcTrackPanel : public QWidget
{
    Q_OBJECT

public:
    rcTrackPanel( QWidget* parent=0, const char* name=0);
    ~rcTrackPanel();

public slots:
	void updateState( rcExperimentState state );
    void updateCamera( bool live, bool storage );
    void updateSource( int source );
    void updateTrackGroups();
    
signals:

protected:

private:
    // repopulate the panel with track group widgets
    void updateTrackGroups( bool cameraInput, bool cameraStorage );
    QScrollArea       * m_scrollarea;
    QWidget*          mContents;
    rcExperimentState mOldState;
    int               mManagedGroups;
    bool              mLiveInput;
    bool              mLiveStorage;
};


#endif // RC_TRACK_PANEL_H
