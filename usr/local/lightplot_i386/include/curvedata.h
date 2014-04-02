/*
    File    :   curvedata.h
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

#ifndef CURVEDATA_H
#define CURVEDATA_H

#include <QtCore/QtCore>


enum Symbol{NoSymbol = 0, Cross, Circle, Square, TriangleUp, TriangleDown, Rhombus};

class CurveData
{
public:
    CurveData();
    CurveData(const CurveData &other);
    CurveData(const QVector<QPointF> &data, const QString &legend);
    ~CurveData();

    void setColor(const QColor &color);
    void setPenWidth(qreal penWidth);
    void setPenStyle(Qt::PenStyle penStyle);
    void setSymbol(Symbol symbol);
    void setSymbolSize(int size);
    void setLegend(const QString &legend);
    void setData(const QVector<QPointF> &data);

    QColor color() const;
    qreal penWidth() const;
    Qt::PenStyle penStyle() const;
    Symbol symbol() const;
    int symbolSize() const;
    QString legend() const;
    QVector<QPointF> data() const;

private:
    QVector<QPointF> _data;
    QColor _color;
    qreal _penWidth;
    Qt::PenStyle _penStyle;
    Symbol _symbol;
    int _symbolSize;
    QString _legend;
};

QDataStream &operator<<(QDataStream &out, const CurveData &curveData);
QDataStream &operator>>(QDataStream &in, CurveData &curveData);

inline CurveData::CurveData()
{
    _penWidth = 2.0;
    _symbolSize = 3;
    _penStyle = Qt::SolidLine;
    _symbol = NoSymbol;
}
//-----------------------------------------------------------------------------

inline CurveData::CurveData(const CurveData &other)
{
    _data = other.data();
    _color = other.color();
    _penWidth = other.penWidth();
    _symbolSize = other.symbolSize();
    _penStyle = other.penStyle();
    _symbol = other.symbol();
    _legend = other.legend();
}
//-----------------------------------------------------------------------------

inline CurveData::CurveData(const QVector<QPointF> &data, const QString &legend)
    : _data(data), _legend(legend)
{
    _penWidth = 2.0;
    _symbolSize = 3;
    _penStyle = Qt::SolidLine;
    _symbol = NoSymbol;
}
//-----------------------------------------------------------------------------

inline CurveData::~CurveData()
{
    _data.clear();
}
//-----------------------------------------------------------------------------

inline void CurveData::setColor(const QColor &color)
{
    _color = color;
}
//-----------------------------------------------------------------------------

inline void CurveData::setPenWidth(qreal penWidth)
{
    _penWidth = penWidth;
}
//-----------------------------------------------------------------------------

inline void CurveData::setPenStyle(Qt::PenStyle penStyle)
{
    _penStyle = penStyle;
}
//-----------------------------------------------------------------------------

inline void CurveData::setSymbol(Symbol symbol)
{
    _symbol = symbol;
}
//-----------------------------------------------------------------------------

inline void CurveData::setSymbolSize(int size)
{
    if (size > 0) {
        _symbolSize = size;
    }
}
//-----------------------------------------------------------------------------

inline void CurveData::setLegend(const QString &legend)
{
    _legend = legend;
}
//-----------------------------------------------------------------------------

inline void CurveData::setData(const QVector<QPointF> &data)
{
    _data = data;
}
//-----------------------------------------------------------------------------

inline QColor CurveData::color() const
{
    return _color;
}
//-----------------------------------------------------------------------------

inline qreal CurveData::penWidth() const
{
    return _penWidth;
}
//-----------------------------------------------------------------------------

inline Qt::PenStyle CurveData::penStyle() const
{
    return _penStyle;
}
//-----------------------------------------------------------------------------

inline Symbol CurveData::symbol() const
{
    return _symbol;
}
//-----------------------------------------------------------------------------

inline int CurveData::symbolSize() const
{
    return _symbolSize;
}
//-----------------------------------------------------------------------------

inline QString CurveData::legend() const
{
    return _legend;
}
//-----------------------------------------------------------------------------

inline QVector<QPointF> CurveData::data() const
{
    return _data;
}
//-----------------------------------------------------------------------------

inline QDataStream &operator<<(QDataStream &out, const CurveData &curveData)
{
    out << curveData.data() << curveData.color() << curveData.legend()
            << int(curveData.penStyle()) << curveData.penWidth()
            << int(curveData.symbol()) << curveData.symbolSize();
    return out;
}
//-----------------------------------------------------------------------------

inline QDataStream &operator>>(QDataStream &in, CurveData &curveData)
{
    QVector<QPointF> data;
    QColor color;
    qreal penWidth;
    int penStyle;
    int symbolSize;
    int symbol;
    QString legend;

    in >> data >> color >> legend >> penStyle >> penWidth >> symbol >> symbolSize;
    curveData.setData(data);
    curveData.setColor(color);
    curveData.setLegend(legend);
    curveData.setPenStyle((Qt::PenStyle)penStyle);
    curveData.setSymbol((Symbol)symbol);
    curveData.setPenWidth(penWidth);
    curveData.setSymbolSize(symbolSize);

    return in;
}
//-----------------------------------------------------------------------------

#endif // CURVEDATA_H
