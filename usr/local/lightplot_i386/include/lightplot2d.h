/*
    File    :   lightplot2d.h
    Project :   LightPlot2D

    Last modified July 8, 2010

    Copyright (C) 2010 Avdeev Maxim (avdeyev_mv@mail.ru)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#ifndef LIGHTPLOT2D_H
#define LIGHTPLOT2D_H

#include <QtGui/QWidget>
#include <QtCore/QtCore>

#include "curvedata.h"
#include "scale.h"
#include "grid.h"
#include "lightplot2d_global.h"

class RotatedLabel;
class QLabel;
class LegendWidget;

class LIGHTPLOT2D_EXPORT LightPlot2D : public QWidget
{
    Q_OBJECT

public:
    LightPlot2D(QWidget *parent = 0);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void addCurveData(const CurveData &curveData);
    void replaceCurveData(const CurveData &curveData, int index);
    void setMinX(const double &minX);
    void setMaxX(const double &maxX);
    void setMinY(const double &minY);
    void setMaxY(const double &maxY);
    void setIncrementX(const double &increment);
    void setIncrementY(const double &increment);
    void setMinorTicksX(int minorTicks);
    void setMinorTicksY(int minorTicks);
    void setScaleX(const Scale &scaleX);
    void setScaleY(const Scale &scaleY);
    void setGridX(const Grid &grid);
    void setGridY(const Grid &grid);
    void setTitle(const QString &text);
    void setNameXAxis(const QString &text);
    void setNameYAxis(const QString &text);
    void setTitleFont(const QFont &font);
    void setXAxisFont(const QFont &font);
    void setYAxisFont(const QFont &font);
    void setLegendFont(const QFont &font);
    void setLegendOpacity(int opacity);
    void setCurrentPointLabelPalette(const QPalette &palette);
    void setCurrentPointLabelFont(const QFont &font);

    QPixmap toPixmap(const QSize &s);
    QString title() const;
    QString nameXAxis() const;
    QString nameYAxis() const;
    QFont titleFont() const;
    QFont xAxisFont() const;
    QFont yAxisFont() const;
    QFont legendFont() const;
    QFont currentPointLabelFont() const;
    QPalette currentPointLabelPalette() const;
    int legendOpacity() const;
    bool legendFrameIsShow() const;
    const Scale &scaleX() const;
    const Scale &scaleY() const;
    const Grid &gridX() const;
    const Grid &gridY() const;
    CurveData curveData(int index) const;
    const QList<CurveData> &curvesList() const;
    const QList<bool> &showCurvesList() const;

    friend class LegendWidget;

signals:
    void zoomChanged(int zoom, int maxZoom);
    void removedCurve(int);

public slots:
    void showMajorGridX(bool show);
    void showMajorGridY(bool show);
    void showMinorGridX(bool show);
    void showMinorGridY(bool show);
    void showCurve(int index, bool show);
    void zoomIn();
    void zoomOut();
    void removeCurve(int id);
    void clearAllCurves();
    void updatePlot();
    void showLegend(bool show);
    void showLegendFrame(bool show);
    void showCross(bool show);
    void showCurrentPointLabel(bool show);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    enum {
        SizeMajorTick = 8,
        SizeMinorTick = 4,
        LeftMargin = 10,
        TopMargin = 10,
        BottomMargin = 10,
        RightMargin = 20,
        LabelMargin = 1,
        MaxLabelWidth = 60,
        MaxLabelHeight = 20
    };
    static const double epsilon;

    QPixmap pixmap;
    QList<CurveData> _curvesList;
    QList<bool> _showCurvesList;
    QVector<Scale> _zoomXAxis;
    QVector<Scale> _zoomYAxis;
    QPoint _posOnLegend;
    QPoint _posLegend;
    QPoint _posCursor;
    QRect _rubberBandRect;
    QRect _plotRect;

    bool _rubberBandIsShow;
    bool _crossIsShow;
    bool _curPointLabelIsShow;
    bool _legendIsMove;
    int _curZoom;

    QLabel *_currentPointLabel;
    RotatedLabel *_xLabel;
    RotatedLabel *_yLabel;
    RotatedLabel *_title;
    LegendWidget *_legendWidget;

    Scale _curScaleX;
    Scale _curScaleY;
    Grid _gridX;
    Grid _gridY;

    QRect plotRect(const QSize &viewPortSize);
    void updateRubberBandRegion();
    void refreshPixmap();
    void drawPlot(QPainter *painter);
    void drawSymbol(QPainter *painter, Symbol symbol, qreal size,
                    QPointF pos, const QColor &clr);
    void drawCross(QPainter *painter, const QPoint &pos);

};

#endif // LIGHTPLOT2D_H
