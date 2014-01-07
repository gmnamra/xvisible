

TARGET = test
TEMPLATE = app

INCLUDEPATH += ../../include ../../../../../util ../../../../../visual
DEPENDPATH = $$INCLUDEPATH

#HEADERS += ASC_DCAM_API.h 

SOURCES += test.cpp

DESTDIR = ../../bin

LIBS += -lrc_util -L../../../util/lib -lrc_visual -L../../../visual/lib -lrc_dcam -L../../lib 
macx:LIBS += -framework Carbon -framework QuickTime -framework CoreFoundation -framework IOKit 
# Note: it is assumed that ASC_DCAM_DEV framework has been properly installed
macx:LIBS += -framework ASC_DCAM_DEV

OBJECTS_DIR = obj

TARGETDEPS += ../../lib/librc_dcam.a ../../../visual/lib/librc_visual.a ../../../util/lib/librc_util.a

# Disable Qt linking
QMAKE_LIBS_QT =

#
# Custom post-link operations
#

