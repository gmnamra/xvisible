#ifndef UI_RCFILESAVESETTINGWIDGET_H
#define UI_RCFILESAVESETTINGWIDGET_H

#include <rc_model.h>

#include "rc_settingwidget.h"

class QString;
class QLineEdit;
class QPushButton;

class rcFileSaveSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcFileSaveSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcFileSaveSettingWidget();

public slots:
    void browseRequest( void );
    void valueChanged( void );
    void settingChanged( void );
    void updateState( rcExperimentState state );
    void movieSave( void );
    
protected:

private:

    QLineEdit*	  mFilenameWidget;
};


#endif // UI_RCFILECHOICESETTINGWIDGET_H
