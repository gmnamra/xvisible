#ifndef _LPWIDGET_H
#define _LPWIDGET_H

#include <QtGui/QWidget>
#include <QVector>
#include <QPoint>
#include <lightplot2d.h>



class LPWidget : public QWidget
{
    Q_OBJECT

public:
    LPWidget(QWidget *parent = 0, const CurveData* cv = 0);
    void load (const CurveData* cv);
    ~LPWidget();
private:
    LightPlot2D *_plot;
};

#endif // _LPWIDGET_H
