

WCROOT=/Users/arman/gwc/vqt105/visibleqt105



Config += i386



MAC_SDK  = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk
if( !exists( $$MAC_SDK) ) {
  error("The selected Mac OSX SDK does not exist at $$MAC_SDK!")
}
QMAKE_MAC_SDK = $$MAC_SDK

QT += qt3support

# enable options
DEFINES += rc_build_option_capture rc_build_option_aci rc_build_option_morphometry rc_build_option_paced_myocyte rc_build_option_selected_myocyte rc_build_option_general_cell rc_build_option_general_fluorescence rc_build_option_cell_lineage 

DEFINES += MAC_OS_X_VERSION_MIN_RQUIRED=MAC_OS_X_VERSION_10_8 MAC_OS_X_VERSION_MAX_ALLOWED=MAC_OS_X_VERSION_10_8


 

HEADERS = $${WCROOT}/include/visUI/rc_about.h \
           $${WCROOT}/include/visUI/rc_appconstants.h \
           $${WCROOT}/include/visUI/rc_settingwidget.h \
           $${WCROOT}/include/visUI/rc_checkboxsettingwidget.h \
           $${WCROOT}/include/visUI/rc_choiceradiobutton.h \
           $${WCROOT}/include/visUI/rc_controlpanel.h \
           $${WCROOT}/include/visUI/rc_filechoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_filesavesettingwidget.h \
           $${WCROOT}/include/visUI/rc_lineedit.h \
           $${WCROOT}/include/visUI/rc_main_window.h \
           $${WCROOT}/include/visUI/rc_menuchoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_modeldomain.h \
           $${WCROOT}/include/visUI/rc_modifierkeys.h \
           $${WCROOT}/include/visUI/rc_monitor.h \
           $${WCROOT}/include/visUI/rc_radiochoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_settingpage.h \
           $${WCROOT}/include/visUI/rc_settingpanel.h \
           $${WCROOT}/include/visUI/rc_textareasettingwidget.h \
           $${WCROOT}/include/visUI/rc_textedit.h \
           $${WCROOT}/include/visUI/rc_textlinesettingwidget.h \
           $${WCROOT}/include/visUI/rc_timedisplay.h \
           $${WCROOT}/include/visUI/rc_rectsettingwidget.h \
           $${WCROOT}/include/visUI/rc_scalartrackwidget.h \
           $${WCROOT}/include/visUI/rc_trackpanel.h \
           $${WCROOT}/include/visUI/rc_trackmanager.h \
           $${WCROOT}/include/visUI/rc_timeline.h \
	   $${WCROOT}/include/visUI/rc_statusbar.h \
           $${WCROOT}/include/visUI/rc_spinboxsettingwidget.h \
           $${WCROOT}/include/visUI/rc_frameratechoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_trackgroupwidget.h \
           $${WCROOT}/include/visUI/rc_xmlparser.h \
           $${WCROOT}/include/visUI/rc_playbacksettingwidget.h \
           $${WCROOT}/include/visUI/rc_customeventmanager.h \
           $${WCROOT}/include/visUI/rc_imagecanvasgl.h \
           $${WCROOT}/include/visUI/rc_thumbwheelsettingwidget.h \
           $${WCROOT}/include/visUI/rc_textchoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_numericchoicesettingwidget.h \
           $${WCROOT}/include/visUI/rc_trackrender.h\
           $${WCROOT}/include/visUI/lpwidget.h\


SOURCES = $${WCROOT}/src/visible/qt/rc_about.cpp \
           $${WCROOT}/src/visible/qt/rc_settingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_checkboxsettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_controlpanel.cpp \
           $${WCROOT}/src/visible/qt/rc_filechoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_filesavesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_main_window.cpp \
           $${WCROOT}/src/visible/qt/rc_menuchoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_modeldomain.cpp \
           $${WCROOT}/src/visible/qt/rc_modifierkeys.cpp \
           $${WCROOT}/src/visible/qt/rc_monitor.cpp \
           $${WCROOT}/src/visible/qt/rc_radiochoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_settingpage.cpp \
           $${WCROOT}/src/visible/qt/rc_settingpanel.cpp \
           $${WCROOT}/src/visible/qt/rc_textareasettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_textlinesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_rectsettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_scalartrackwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_trackpanel.cpp \
           $${WCROOT}/src/visible/qt/rc_trackmanager.cpp \
           $${WCROOT}/src/visible/qt/rc_timeline.cpp \
	   $${WCROOT}/src/visible/qt/rc_statusbar.cpp \
           $${WCROOT}/src/visible/qt/rc_spinboxsettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_frameratechoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_trackgroupwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_xmlparser.cpp \
           $${WCROOT}/src/visible/qt/rc_playbacksettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_customeventmanager.cpp \
           $${WCROOT}/src/visible/qt/rc_imagecanvasgl.cpp \
           $${WCROOT}/src/visible/qt/rc_thumbwheelsettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_textchoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_numericchoicesettingwidget.cpp \
           $${WCROOT}/src/visible/qt/rc_trackrender.cpp\
           $${WCROOT}/src/visible/qt/lpwidget.cpp

INCLUDEPATH =   $${WCROOT}/include/util  $${WCROOT}/include/visual  $${WCROOT}/include/analysis  $${WCROOT}/include/stlplus  $${WCROOT}/include/test  $${WCROOT}/include/baseUI  $${WCROOT}/include/visUI  $${WCROOT}/include/visiCore $${WCROOT}/usr/local/boost/include  $${WCROOT}/usr/local/lightplot_i386/include $${WCROOT}/usr/local/Cinder/include $${WCROOT}/usr/local/OpenCv_2.4_i386/include

QT += opengl xml


OBJECTS_DIR = $${WCROOT}/build/qt_moc/.obj


# install
target.arch = i386

target.path = $${WCROOT}/src/visible/moc
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS visible_qt_moc.pro
sources.path = $${WCROOT}/src
INSTALLS += target sources
macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.8
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -stdlib=libc++


QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8
