#ifndef _LPWIDGET_H
#define _LPWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include <QPoint>
#include <lightplot2d.h>
#include <boost/shared_ptr.hpp>

typedef boost::shared_ptr<CurveData> SharedCurveDataRef;

class LPWidget : public QWidget
{
    Q_OBJECT

public:
    LPWidget(QWidget *parent = 0, const CurveData * cdata = 0);
    void new_plot (SharedCurveDataRef& );
    void new_plot ( const CurveData * );    
    ~LPWidget();
private:
    void demo ();
    LightPlot2D *_plot;
};

#endif // _LPWIDGET_H
