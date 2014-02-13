/*
    File    :   legendwidget.h
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
#ifndef LEGENDWIDGET_H
#define LEGENDWIDGET_H

#include <QWidget>
#include <QLabel>
#include "lightplot2d_global.h"

class LightPlot2D;

class LIGHTPLOT2D_EXPORT LegendWidget : public QWidget
{
Q_OBJECT
public:
    explicit LegendWidget(QWidget *parent = 0);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void setOpacity(int opacity);
    void updateLegend();
    const QImage &image() const;

    int opacity() const;
    bool frameIsShow() const;
    bool legendIsVisible() const;

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

signals:

public slots:
    void showFrame(bool isShow);
    void setVisibleLegend(bool isVisible);

private:
    enum {Margin = 10, MarkerSize = 40};
    mutable QLabel _label;
    LightPlot2D *_plotter;
    int _opacity;
    bool _showFrame;
    bool _isVisible;
    QImage _img;
};

#endif // LEGENDWIDGET_H
