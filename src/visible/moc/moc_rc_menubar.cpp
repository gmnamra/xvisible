/****************************************************************************
** Meta object code from reading C++ file 'rc_menubar.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_menubar.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_menubar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMenuBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   11,   10,   10, 0x0a,
      48,   10,   10,   10, 0x0a,
      56,   10,   10,   10, 0x0a,
      65,   63,   10,   10, 0x0a,
      87,   82,   10,   10, 0x0a,
     114,   10,   10,   10, 0x0a,
     151,   10,   10,   10, 0x0a,
     160,   10,   10,   10, 0x0a,
     171,   10,   10,   10, 0x0a,
     187,   10,   10,   10, 0x0a,
     209,   10,   10,   10, 0x0a,
     226,   10,   10,   10, 0x0a,
     235,   10,   10,   10, 0x0a,
     252,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMenuBar[] = {
    "rcMenuBar\0\0state\0updateState(rcExperimentState)\0"
    "about()\0help()\0i\0inputSource(int)\0"
    "rect\0updateAnalysisRect(rcRect)\0"
    "reload_plotter2d(const CurveData2d*)\0"
    "doSave()\0doExport()\0doExportMovie()\0"
    "doExportNativeMovie()\0doExportMatrix()\0"
    "doOpen()\0doOpenSettings()\0"
    "requestCellInfoDisplay()\0"
};

void rcMenuBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcMenuBar *_t = static_cast<rcMenuBar *>(_o);
        switch (_id) {
        case 0: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: _t->about(); break;
        case 2: _t->help(); break;
        case 3: _t->inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 5: _t->reload_plotter2d((*reinterpret_cast< const CurveData2d*(*)>(_a[1]))); break;
        case 6: _t->doSave(); break;
        case 7: _t->doExport(); break;
        case 8: _t->doExportMovie(); break;
        case 9: _t->doExportNativeMovie(); break;
        case 10: _t->doExportMatrix(); break;
        case 11: _t->doOpen(); break;
        case 12: _t->doOpenSettings(); break;
        case 13: _t->requestCellInfoDisplay(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcMenuBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcMenuBar::staticMetaObject = {
    { &QMenuBar::staticMetaObject, qt_meta_stringdata_rcMenuBar,
      qt_meta_data_rcMenuBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcMenuBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcMenuBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcMenuBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMenuBar))
        return static_cast<void*>(const_cast< rcMenuBar*>(this));
    return QMenuBar::qt_metacast(_clname);
}

int rcMenuBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenuBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
