MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.5.sdk
if( !exists( $$MAC_SDK) ) {
  error("The selected Mac OSX SDK does not exist at $$MAC_SDK!")
}
QMAKE_MAC_SDK = $$MAC_SDK

QT += qt3support

# enable options
DEFINES += rc_build_option_capture rc_build_option_aci rc_build_option_morphometry rc_build_option_paced_myocyte rc_build_option_selected_myocyte rc_build_option_general_cell rc_build_option_general_fluorescence rc_build_option_cell_lineage 

DEFINES += MAC_OS_X_VERSION_MIN_RQUIRED=MAC_OS_X_VERSION_10_5 MAC_OS_X_VERSION_MAX_ALLOWED=MAC_OS_X_VERSION_10_5


  # INCLUDE_DIR
  isEmpty( INCLUDE_DIR )
{
    INCLUDE_DIR = ..
    message( Using .. as INCLUDE_DIR )
}

 # SOURCES_DIR
  isEmpty( SOURCES_DIR )
{
    SOURCES_DIR = ..
    message( Using .. as SOURCE_DIR )
}


HEADERS = $${INCLUDE_DIR}/include/visUI/rc_about.h \
           $${INCLUDE_DIR}/include/visUI/rc_appconstants.h \
           $${INCLUDE_DIR}/include/visUI/rc_settingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_checkboxsettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_choiceradiobutton.h \
           $${INCLUDE_DIR}/include/visUI/rc_controlpanel.h \
           $${INCLUDE_DIR}/include/visUI/rc_filechoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_filesavesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_lineedit.h \
           $${INCLUDE_DIR}/include/visUI/rc_mainwindow.h \
           $${INCLUDE_DIR}/include/visUI/rc_menubar.h \
           $${INCLUDE_DIR}/include/visUI/rc_menuchoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_modeldomain.h \
           $${INCLUDE_DIR}/include/visUI/rc_modifierkeys.h \
           $${INCLUDE_DIR}/include/visUI/rc_monitor.h \
           $${INCLUDE_DIR}/include/visUI/rc_radiochoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_settingpage.h \
           $${INCLUDE_DIR}/include/visUI/rc_settingpanel.h \
           $${INCLUDE_DIR}/include/visUI/rc_textareasettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_textedit.h \
           $${INCLUDE_DIR}/include/visUI/rc_textlinesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_timedisplay.h \
           $${INCLUDE_DIR}/include/visUI/rc_rectsettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_scalartrackwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_trackpanel.h \
           $${INCLUDE_DIR}/include/visUI/rc_trackmanager.h \
           $${INCLUDE_DIR}/include/visUI/rc_timeline.h \
	   $${INCLUDE_DIR}/include/visUI/rc_statusbar.h \
           $${INCLUDE_DIR}/include/visUI/rc_spinboxsettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_frameratechoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_trackgroupwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_xmlparser.h \
           $${INCLUDE_DIR}/include/visUI/rc_playbacksettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_customeventmanager.h \
           $${INCLUDE_DIR}/include/visUI/rc_imagecanvasgl.h \
           $${INCLUDE_DIR}/include/visUI/rc_cellinfowidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_thumbwheelsettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_textchoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_numericchoicesettingwidget.h \
           $${INCLUDE_DIR}/include/visUI/rc_trackrender.h\
           $${INCLUDE_DIR}/include/visUI/lpwidget.h\


SOURCES = $${SOURCES_DIR}/src/visible/qt/rc_about.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_settingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_checkboxsettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_controlpanel.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_filechoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_filesavesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_mainwindow.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_menubar.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_menuchoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_modeldomain.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_modifierkeys.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_monitor.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_radiochoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_settingpage.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_settingpanel.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_textareasettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_textlinesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_rectsettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_scalartrackwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_trackpanel.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_trackmanager.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_timeline.cpp \
	   $${SOURCES_DIR}/src/visible/qt/rc_statusbar.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_spinboxsettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_frameratechoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_trackgroupwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_xmlparser.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_playbacksettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_customeventmanager.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_imagecanvasgl.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_cellinfowidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_thumbwheelsettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_textchoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_numericchoicesettingwidget.cpp \
           $${SOURCES_DIR}/src/visible/qt/rc_trackrender.cpp\
           $${SOURCES_DIR}/src/visible/qt/lpwidget.cpp

INCLUDEPATH =   $${INCLUDE_DIR}/include/util  $${INCLUDE_DIR}/include/visual  $${INCLUDE_DIR}/include/analysis  $${INCLUDE_DIR}/include/stlplus  $${INCLUDE_DIR}/include/test  $${INCLUDE_DIR}/include/baseUI  $${INCLUDE_DIR}/include/visUI  $${INCLUDE_DIR}/include/visiCore $${INCLUDE_DIR}/usr/local/boost/include  $${INCLUDE_DIR}/usr/local/lightplot_i386/include

QT += opengl xml


OBJECTS_DIR = $${SOURCES_DIR}/src/visible/.obj

# install
target.path = $${SOURCES_DIR}/src/visible/moc
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS visible_qt_moc.pro
sources.path = $${SOURCES_DIR}/src
INSTALLS += target sources


