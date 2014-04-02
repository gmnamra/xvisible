/*
    File    :   scale.h
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
#ifndef SCALE_H
#define SCALE_H

#include <QtCore/QtCore>
#include <cmath>

class Scale
{
public:
    Scale();
    Scale(const double &min, const double &max, const double &increment, int minorTicks = 1);
    Scale(const Scale &other);

    void setMin(const double &min);
    void setMax(const double &max);
    void setIncrement(const double &val);
    void setMinorTicks(int val);
    double min() const;
    double max() const;
    double increment() const;
    int minorTicks() const;
    double width() const;
    QVector<double> vectorOfMajorTicks() const;
    QVector<double> vectorOfMinorTicks() const;
    void adjust();

    bool operator==(const Scale &scale) const;
    bool operator!=(const Scale &scale) const;

    friend inline void scrollScale(Scale &scale, int dx);

private:
    double _min, _max;
    double _increment;
    int _minorTicks;
};

inline Scale::Scale()
{
    _min = 0.0;
    _max = 1.0;
    _increment = 0.1;
    _minorTicks = 1;
}
//-----------------------------------------------------------------------------

inline Scale::Scale(const Scale &other)
{
    _min = other.min();
    _max = other.max();
    _increment = other.increment();
    _minorTicks = other.minorTicks();
}
//-----------------------------------------------------------------------------

inline Scale::Scale(const double &min, const double &max, const double &increment, int minorTicks)
    : _min(min), _max(max), _increment(increment), _minorTicks(minorTicks)
{
    if (_increment == 0.0)
        _increment = 0.1;
}
//-----------------------------------------------------------------------------

inline QVector<double> Scale::vectorOfMajorTicks() const
{
    int nStart = (int)ceil(_min/_increment);
    int nEnd = (int)floor(_max/_increment);
    QVector<double> vTicks;
    for (int i = nStart; i <= nEnd; ++i) {
        vTicks.append(double(i * _increment));
    }
    return vTicks;
}
//-----------------------------------------------------------------------------

inline QVector<double> Scale::vectorOfMinorTicks() const
{
    const double dx = _increment/(_minorTicks + 1);
    int nStart = (int)ceil(_min/dx);
    int nEnd = (int)floor(_max/dx);
    QVector<double> vMinorTicks;
    for (int i = nStart; i <= nEnd; ++i) {
        vMinorTicks.append(double(i * dx));
    }
    return vMinorTicks;
}
//-----------------------------------------------------------------------------

inline void Scale::adjust()
{
    const int MinTicks = 5;
    double grossStep = (_max - _min) / MinTicks;
    _increment = pow(10.0, floor(log10(grossStep)));

    if (5 * _increment < grossStep) {
        _increment *= 5;
    } else if (2 * _increment < grossStep) {
        _increment *= 2;
    }
}
//-----------------------------------------------------------------------------

inline void Scale::setMin(const double &min)
{
    _min = min;
}
//-----------------------------------------------------------------------------

inline void Scale::setMax(const double &max)
{
    _max = max;
}
//-----------------------------------------------------------------------------

inline void Scale::setIncrement(const double &val)
{
    if (val > 0.0)
        _increment = val;
}
//-----------------------------------------------------------------------------

inline void Scale::setMinorTicks(int val)
{
    if (val >= 0)
        _minorTicks = val;
}
//-----------------------------------------------------------------------------

inline double Scale::min() const
{
    return _min;
}
//-----------------------------------------------------------------------------

inline double Scale::max() const
{
    return _max;
}
//-----------------------------------------------------------------------------

inline double Scale::width() const
{
    return _max - _min;
}
//-----------------------------------------------------------------------------

inline double Scale::increment() const
{
    return _increment;
}
//-----------------------------------------------------------------------------

inline int Scale::minorTicks() const
{
    return _minorTicks;
}
//-----------------------------------------------------------------------------

inline bool Scale::operator==(const Scale &scale) const
{
    return ((scale.increment() == _increment) &&
            (scale.max() == _max) &&
            (scale.min() == _min) &&
            (scale.minorTicks() == _minorTicks));
}
//-----------------------------------------------------------------------------

inline bool Scale::operator!=(const Scale &scale) const
{
    return ((scale.increment() != _increment) ||
            (scale.max() != _max) ||
            (scale.min() != _min) ||
            (scale.minorTicks() != _minorTicks));
}
//-----------------------------------------------------------------------------

inline void scrollScale(Scale &scale, int dx)
{
    scale._min += dx * scale._increment;
    scale._max += dx * scale._increment;
}
//-----------------------------------------------------------------------------

#endif // SCALE_H
