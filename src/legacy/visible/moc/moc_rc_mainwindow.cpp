/****************************************************************************
** Meta object code from reading C++ file 'rc_mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_mainwindow.h' doesn't include <QObject>."
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
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,
      31,   13,   13,   13, 0x0a,
      64,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMainWindow[] = {
    "rcMainWindow\0\0settingChanged()\0"
    "reload_plotter(const CurveData*)\0"
    "reload_plotter2d(const CurveData2d*)\0"
};

void rcMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcMainWindow *_t = static_cast<rcMainWindow *>(_o);
        switch (_id) {
        case 0: _t->settingChanged(); break;
        case 1: _t->reload_plotter((*reinterpret_cast< const CurveData*(*)>(_a[1]))); break;
        case 2: _t->reload_plotter2d((*reinterpret_cast< const CurveData2d*(*)>(_a[1]))); break;
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
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE