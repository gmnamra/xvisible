#include "widget.h"
#include <lightplot2d.h>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVector<QPointF> data0;
    QVector<QPointF> data1;
    for (int i = 0; i < 100; i++) {
        double x = double(0.01 * i);
        double y = exp(-(x-0.5)*(x-0.5)*16.0);
        data0.append(QPointF(x, y));
        data1.append(QPointF(x, sin(25.0*x)*y));
    }
    CurveData cd0(data0, "exp(-16(<i>x</i>-1/2)<sup>2</sup>)");
    cd0.setColor(Qt::red);
    cd0.setSymbol(Circle);
    CurveData cd1(data1, "sin(25<i>x</i>)exp(-16(<i>x</i>-1/2)<sup>2</sup>)");
    cd1.setColor(Qt::blue);
    cd1.setSymbol(Rhombus);
    cd1.setPenStyle(Qt::DashLine);
    _plot = new LightPlot2D;
    _plot->addCurveData(cd0);
    _plot->addCurveData(cd1);

    _plot->setMinY(-1.0);
    _plot->setMaxY(1.1);
    _plot->setIncrementY(0.25);

    _plot->setNameXAxis("<i><font color=#CD5C5C>Axis</font> </i><font color=#708090>X</font> :)");
    _plot->setLegendOpacity(220);

    _plot->setLegendFont(QFont("Serif", 12));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_plot);
}

Widget::~Widget()
{

}
