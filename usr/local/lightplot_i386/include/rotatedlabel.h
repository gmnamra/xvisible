/*
    File    :   rotatedlabel.h
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
#ifndef ROTATEDLABEL_H
#define ROTATEDLABEL_H

#include <QWidget>
#include <QLabel>
#include "lightplot2d_global.h"

class LIGHTPLOT2D_EXPORT RotatedLabel : public QWidget
{
public:
    RotatedLabel(const QString &text, QWidget *parent = 0);
    void setAngle(qreal angle);
    void setText(const QString &text);
    void setAlignment(Qt::Alignment);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QString text() const;
    qreal angle() const;
    Qt::Alignment alignment() const;

    void updateLabel();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    void updateBuffer();
    qreal _angle;
    QLabel _label;
    QImage _img;
};

#endif // ROTATEDLABEL_H
