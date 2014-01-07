!include( ../../products.pri ) {
    error( "Shared build configuration file products.pri not found")
}

TARGET = rfyhostinfo
TEMPLATE = app

INCLUDEPATH += .
DEPENDPATH = $$INCLUDEPATH

# Input
HEADERS += 

SOURCES += rc_main.cpp

# Disable Qt linking
QMAKE_LIBS_QT =

OBJECTS_DIR = obj
DESTDIR = .

macx:LIBS += -framework CoreFoundation -framework IOKit -L/usr/lib -lcurl -lssl -lcrypto -lz

release {
# Strip symbols from optimized build
QMAKE_POST_LINK += test -e $(TARGET) && strip -u -r $(TARGET)
}

debug {
# No stripping
QMAKE_POST_LINK = 
}
