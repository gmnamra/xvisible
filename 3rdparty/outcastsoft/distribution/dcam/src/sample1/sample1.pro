!include( ../../../products.pri ) {    
error( "Shared build configuration file products.pri not found")
}

TARGET = ASC_DCAM_Sample1
TEMPLATE = app

DEPENDPATH = $$INCLUDEPATH

HEADERS += ASC_DCAM_API.h

SOURCES += ASC_DCAM_Sample1.cpp

DESTDIR = ../../bin

macx:LIBS += -framework Carbon -framework QuickTime -framework CoreFoundation -framework IOKit 
# Note: it is assumed that ASC_DCAM_DEV framework has been properly installed
macx:LIBS += -framework ASC_DCAM_DEV

OBJECTS_DIR = obj

# Disable Qt linking
QMAKE_LIBS_QT =

#
# Custom post-link operations
#

