#ifndef UI_RCSETTINGPANEL_H
#define UI_RCSETTINGPANEL_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include <rc_model.h>

class rcSettingPanel : public QTabWidget
{
    Q_OBJECT

public:
    rcSettingPanel( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcSettingPanel();
    void updatePages( void );
public slots:
	void updateState( rcExperimentState state );

signals:

protected:

private:
//    void updatePages( void );
};


#endif // UI_RCSETTINGPANEL_H
