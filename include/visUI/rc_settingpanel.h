#ifndef UI_RCSETTINGPANEL_H
#define UI_RCSETTINGPANEL_H

#include <qtabwidget.h>

#include <rc_model.h>
//    void requestTrackingDisplayGL();


class rcSettingPanel : public QTabWidget
{
    Q_OBJECT

public:
    rcSettingPanel( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcSettingPanel();

public slots:
    void updateState( rcExperimentState state );
    void updatePages( void );

    
signals:

protected:

private:
    
};


#endif // UI_RCSETTINGPANEL_H
