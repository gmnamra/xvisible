!include( ../../../../products.pri ) {
    error( "Shared build configuration file products.pri not found")
}

TARGET = license-generator
TEMPLATE = app
INCLUDEPATH += ../../../../util/include 
INCLUDEPATH += ../../../../blitz

DEPENDPATH = $$INCLUDEPATH

HEADERS = ../../../../util/include/rc_security.h

SOURCES = rc_main.cpp 

DESTDIR = ../../bin

# Disable Qt linking
QMAKE_LIBS_QT =

LIBS += -lrc_util -L../../../../util/lib 

macx:LIBS += -framework CoreFoundation -framework IOKit

TARGETDEPS += ../../../../util/lib/librc_util.a 

OBJECTS_DIR = obj

release {
# Strip symbols from optimized build
QMAKE_POST_LINK += test -e $(TARGET) && strip -u -r $(TARGET)
}

debug {
# No stripping
QMAKE_POST_LINK = 
}
