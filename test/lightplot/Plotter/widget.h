#ifndef WIDGET_H
#define WIDGET_H

#include <QtGui/QWidget>

class LightPlot2D;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();
private:
    LightPlot2D *_plot;
};

#endif // WIDGET_H
