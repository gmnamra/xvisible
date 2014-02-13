#include "lpwidget.h"
#include <lightplot2d.h>
#include <QVBoxLayout>

LPWidget::LPWidget(QWidget *parent, const CurveData* data0_ptr)
: QWidget(parent, Qt::WDestructiveClose | Qt::WResizeNoErase)
{
    
    if (data0_ptr != 0)
    {
        CurveData cd0(*data0_ptr); 
        cd0.setColor(Qt::red);
        _plot = new LightPlot2D;
        _plot->addCurveData(cd0);
        
        _plot->setMinY(-1.0);
        _plot->setMaxY(1.1);
        _plot->setIncrementY(0.25);
        
        _plot->setNameXAxis("<i><font color=#CD5C5C>Axis</font> </i><font color=#708090>X</font> :)");
        _plot->setLegendOpacity(220);
        
        _plot->setLegendFont(QFont("Serif", 12));
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(_plot);
    }

}

void LPWidget::load (const CurveData* cv)
{
    CurveData cd0(*cv); 
    cd0.setColor(Qt::red);
    _plot = new LightPlot2D;
    _plot->addCurveData(cd0);
    
    _plot->setMinY(-1.0);
    _plot->setMaxY(1.1);
    _plot->setIncrementY(0.25);
    
    _plot->setNameXAxis("<i><font color=#CD5C5C>Axis</font> </i><font color=#708090>X</font> :)");
    _plot->setLegendOpacity(220);
    
    _plot->setLegendFont(QFont("Serif", 12));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_plot);

}
LPWidget::~LPWidget()
{
    
}
