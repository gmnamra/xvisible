/****************************************************************************
** Meta object code from reading C++ file 'rc_trackpanel.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_trackpanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_trackpanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTrackPanel[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x0a,
      64,   51,   13,   13, 0x0a,
      95,   88,   13,   13, 0x0a,
     113,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTrackPanel[] = {
    "rcTrackPanel\0\0state\0updateState(rcExperimentState)\0"
    "live,storage\0updateCamera(bool,bool)\0"
    "source\0updateSource(int)\0updateTrackGroups()\0"
};

void rcTrackPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcTrackPanel *_t = static_cast<rcTrackPanel *>(_o);
        switch (_id) {
        case 0: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: _t->updateCamera((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 2: _t->updateSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->updateTrackGroups(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcTrackPanel::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcTrackPanel::staticMetaObject = {
    { &Q3ScrollView::staticMetaObject, qt_meta_stringdata_rcTrackPanel,
      qt_meta_data_rcTrackPanel, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcTrackPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcTrackPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcTrackPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTrackPanel))
        return static_cast<void*>(const_cast< rcTrackPanel*>(this));
    return Q3ScrollView::qt_metacast(_clname);
}

int rcTrackPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3ScrollView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE