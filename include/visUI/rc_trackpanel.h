#ifndef rcTRACK_PANEL_H
#define rcTRACK_PANEL_H

#include <q3scrollview.h>
//Added by qt3to4:
#include <QLabel>

#include <rc_timestamp.h>
#include <rc_model.h>

class QLabel;
class QWidget;

class rcTrackPanel : public Q3ScrollView
{
    Q_OBJECT

public:
    rcTrackPanel( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcTrackPanel();

public slots:
	void updateState( rcExperimentState state );
    void updateCamera( bool live, bool storage );
    void updateSource( int source );
    void updateTrackGroups();
    
signals:

protected:

    void keyPressEvent( QKeyEvent* keyEvent );
    
private:
    // repopulate the panel with track group widgets
    void updateTrackGroups( bool cameraInput, bool cameraStorage );
    
    QWidget*          mContents;
    rcExperimentState mOldState;
    int               mManagedGroups;
    bool              mLiveInput;
    bool              mLiveStorage;
};


#endif // rcTRACK_PANEL_H
