#ifndef UI_RCSTATUSBAR_H
#define UI_RCSTATUSBAR_H


#include <QtGui/QtGui>
#include <QtCore/QtCore>


#include <rc_model.h>

class rcStatusBar : public QStatusBar
{
    Q_OBJECT

public:
    rcStatusBar( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcStatusBar();

public slots:
     void updateState( rcExperimentState state );
     void updateStatus( const char* statusString );
 
signals:

protected:

private:
};


#endif // UI_RCSTATUSBAR_H
