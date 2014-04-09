/****************************************************************************
** Meta object code from reading C++ file 'rc_main_window.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_main_window.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMainWindow[] = {

 // content:
       6,       // revision
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
     107,  102,   13,   13, 0x0a,
     134,   13,   13,   13, 0x0a,
     143,   13,   13,   13, 0x0a,
     154,   13,   13,   13, 0x0a,
     170,   13,   13,   13, 0x0a,
     192,   13,   13,   13, 0x0a,
     215,   13,   13,   13, 0x0a,
     224,   13,   13,   13, 0x0a,
     233,   13,   13,   13, 0x0a,
     250,   13,   13,   13, 0x0a,
     270,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMainWindow[] = {
    "rcMainWindow\0\0state\0updateState(rcExperimentState)\0"
    "about()\0help()\0i\0inputSource(int)\0"
    "settingChanged()\0rect\0updateAnalysisRect(rcRect)\0"
    "doSave()\0doExport()\0doExportMovie()\0"
    "doExportNativeMovie()\0enableExportSmMatrix()\0"
    "doOpen()\0doExit()\0doOpenSettings()\0"
    "importRecentMovie()\0"
    "reload_plotter(const CurveData*)\0"
};

void rcMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcMainWindow *_t = static_cast<rcMainWindow *>(_o);
        switch (_id) {
        case 0: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: _t->about(); break;
        case 2: _t->help(); break;
        case 3: _t->inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->settingChanged(); break;
        case 5: _t->updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 6: _t->doSave(); break;
        case 7: _t->doExport(); break;
        case 8: _t->doExportMovie(); break;
        case 9: _t->doExportNativeMovie(); break;
        case 10: _t->enableExportSmMatrix(); break;
        case 11: _t->doOpen(); break;
        case 12: _t->doExit(); break;
        case 13: _t->doOpenSettings(); break;
        case 14: _t->importRecentMovie(); break;
        case 15: _t->reload_plotter((*reinterpret_cast< const CurveData*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcMainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_rcMainWindow,
      qt_meta_data_rcMainWindow, &staticMetaObjectExtraData }
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
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
