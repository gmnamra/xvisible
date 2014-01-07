!include( ../../../products.pri ) {    error( "Shared build configuration file products.pri not found")}

TARGET = rc_dcam
TEMPLATE = lib

INCLUDEPATH += ../../include
INCLUDEPATH += ../../../util/include
INCLUDEPATH += ../../../visual/include
DEPENDPATH = $$INCLUDEPATH

HEADERS =  ../../include/rc_dcam.h 

SOURCES = rc_dcam.cpp 

macx:QMAKE_CXXFLAGS_RELEASE += -faltivec -force_cpusubtype_ALL
macx:QMAKE_CXXFLAGS_DEBUG += -faltivec -force_cpusubtype_ALL


# Disable Qt linking
QMAKE_LIBS_QT =

CONFIG += staticlib
OBJECTS_DIR = obj
DESTDIR = ../../lib
