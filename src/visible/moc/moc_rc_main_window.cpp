/****************************************************************************
** Meta object code from reading C++ file 'rc_main_window.h'
**
** Created: Sat Mar 15 16:26:35 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_main_window.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMainWindow[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x0a,
      51,   13,   13,   13, 0x0a,
      59,   13,   13,   13, 0x0a,
      68,   66,   13,   13, 0x0a,
      85,   13,   13,   13, 0x0a,
     102,   13,   13,   13, 0x0a,
     121,  116,   13,   13, 0x0a,
     148,   13,   13,   13, 0x0a,
     157,   13,   13,   13, 0x0a,
     168,   13,   13,   13, 0x0a,
     184,   13,   13,   13, 0x0a,
     206,   13,   13,   13, 0x0a,
     225,   13,   13,   13, 0x0a,
     234,   13,   13,   13, 0x0a,
     251,   13,   13,   13, 0x0a,
     271,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMainWindow[] = {
    "rcMainWindow\0\0state\0updateState(rcExperimentState)\0"
    "about()\0help()\0i\0inputSource(int)\0"
    "settingChanged()\0newSmMatrix()\0rect\0"
    "updateAnalysisRect(rcRect)\0doSave()\0"
    "doExport()\0doExportMovie()\0"
    "doExportNativeMovie()\0doExportSmMatrix()\0"
    "doOpen()\0doOpenSettings()\0importRecentMovie()\0"
    "reload_plotter(const CurveData*)\0"
};

const QMetaObject rcMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_rcMainWindow,
      qt_meta_data_rcMainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMainWindow))
        return static_cast<void*>(const_cast< rcMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int rcMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: about(); break;
        case 2: help(); break;
        case 3: inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: settingChanged(); break;
        case 5: newSmMatrix(); break;
        case 6: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 7: doSave(); break;
        case 8: doExport(); break;
        case 9: doExportMovie(); break;
        case 10: doExportNativeMovie(); break;
        case 11: doExportSmMatrix(); break;
        case 12: doOpen(); break;
        case 13: doOpenSettings(); break;
        case 14: importRecentMovie(); break;
        case 15: reload_plotter((*reinterpret_cast< const CurveData*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
