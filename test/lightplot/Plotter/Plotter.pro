TARGET = Plotter
TEMPLATE = app

LP2D_LIB =
LP2D_INCLUDE =

unix {
    LP2D_LIB = /usr/local/lightplot2d-1.0.0/lib
    LP2D_INCLUDE = /usr/local/lightplot2d-1.0.0/src
    LIBS += -L$${LP2D_LIB} -llightplot2d
}



INCLUDEPATH += $${LP2D_INCLUDE}

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

OBJECTS_DIR = obj
MOC_DIR = moc
DESTDIR = bin




