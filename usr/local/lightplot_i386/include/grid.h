/*
    File    :   grid.h
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
#ifndef GRID_H
#define GRID_H

#include <QPen>

class Grid
{
public:
    Grid();
    Grid(const Grid &other);
    void setPenOfMajorGrid(const QPen &pen);
    void setPenOfMinorGrid(const QPen &pen);
    void showMajorGrid(bool show);
    void showMinorGrid(bool show);

    QPen penOfMajorGrid() const;
    QPen penOfMinorGrid() const;
    bool showMajorGrid() const;
    bool showMinorGrid() const;

    int operator==(const Grid &grid) const;
    int operator!=(const Grid &grid) const;

private:
    QPen _penOfMajorGrid;
    QPen _penOfMinorGrid;
    bool _showMajorGrid;
    bool _showMinorGrid;
};

QDataStream &operator<<(QDataStream &out, const Grid &grid);
QDataStream &operator>>(QDataStream &in, Grid &grid);

inline Grid::Grid()
{
    _penOfMajorGrid.setColor(Qt::gray);
    _penOfMajorGrid.setWidth(0);
    _penOfMajorGrid.setStyle(Qt::SolidLine);
    _penOfMinorGrid.setColor(Qt::gray);
    _penOfMinorGrid.setWidth(0);
    _penOfMinorGrid.setStyle(Qt::DotLine);
    _showMajorGrid = true;
    _showMinorGrid = true;
}
//-----------------------------------------------------------------------------

inline Grid::Grid(const Grid &other)
{
    _penOfMajorGrid = other.penOfMajorGrid();
    _penOfMinorGrid = other.penOfMinorGrid();
    _showMajorGrid = other.showMajorGrid();
    _showMinorGrid = other.showMinorGrid();
}
//-----------------------------------------------------------------------------

inline void Grid::setPenOfMajorGrid(const QPen &pen)
{
    _penOfMajorGrid = pen;
}
//-----------------------------------------------------------------------------

inline void Grid::setPenOfMinorGrid(const QPen &pen)
{
    _penOfMinorGrid = pen;
}
//-----------------------------------------------------------------------------

inline void Grid::showMajorGrid(bool show)
{
    _showMajorGrid = show;
}
//-----------------------------------------------------------------------------

inline void Grid::showMinorGrid(bool show)
{
    _showMinorGrid = show;
}
//-----------------------------------------------------------------------------

inline QPen Grid::penOfMajorGrid() const
{
    return _penOfMajorGrid;
}
//-----------------------------------------------------------------------------

inline QPen Grid::penOfMinorGrid() const
{
    return _penOfMinorGrid;
}
//-----------------------------------------------------------------------------

inline bool Grid::showMajorGrid() const
{
    return _showMajorGrid;
}
//-----------------------------------------------------------------------------

inline bool Grid::showMinorGrid() const
{
    return _showMinorGrid;
}
//-----------------------------------------------------------------------------

inline int Grid::operator==(const Grid &grid) const
{
    return ((_penOfMajorGrid == grid.penOfMajorGrid()) &&
            (_penOfMinorGrid == grid.penOfMinorGrid()) &&
            (_showMajorGrid == grid.showMajorGrid()) &&
            (_showMinorGrid == grid.showMinorGrid()));
}
//-----------------------------------------------------------------------------

inline int Grid::operator!=(const Grid &grid) const
{
    return ((_penOfMajorGrid != grid.penOfMajorGrid()) ||
            (_penOfMinorGrid != grid.penOfMinorGrid()) ||
            (_showMajorGrid != grid.showMajorGrid()) ||
            (_showMinorGrid != grid.showMinorGrid()));
}
//-----------------------------------------------------------------------------

inline QDataStream &operator<<(QDataStream &out, const Grid &grid)
{
    out << grid.penOfMajorGrid() << grid.penOfMinorGrid()
            << grid.showMajorGrid() << grid.showMinorGrid();
    return out;
}
//-----------------------------------------------------------------------------

inline QDataStream &operator>>(QDataStream &in, Grid &grid)
{
    QPen penOfMajorGrid;
    QPen penOfMinorGrid;
    bool showMajorGrid;
    bool showMinorGrid;
    in >> penOfMajorGrid >> penOfMinorGrid >> showMajorGrid >> showMinorGrid;
    grid.setPenOfMajorGrid(penOfMajorGrid);
    grid.setPenOfMinorGrid(penOfMinorGrid);
    grid.showMajorGrid(showMajorGrid);
    grid.showMinorGrid(showMinorGrid);
    return in;
}
//-----------------------------------------------------------------------------

#endif // GRID_H
