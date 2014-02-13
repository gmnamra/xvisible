/*
    File    :   lightplot2d_global.h
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
#ifndef LIGHTPLOT2D_GLOBAL_H
#define LIGHTPLOT2D_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIGHTPLOT2D_LIBRARY)
#  define LIGHTPLOT2D_EXPORT Q_DECL_EXPORT
#else
#  define LIGHTPLOT2D_EXPORT Q_DECL_IMPORT
#endif

#endif // LIGHTPLOT2D_GLOBAL_H
