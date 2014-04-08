/****************************************************************************
** Meta object code from reading C++ file 'rc_statusbar.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_statusbar.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_statusbar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcStatusBar[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      19,   13,   12,   12, 0x0a,
      63,   50,   12,   12, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcStatusBar[] = {
    "rcStatusBar\0\0state\0updateState(rcExperimentState)\0"
    "statusString\0updateStatus(const char*)\0"
};

void rcStatusBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcStatusBar *_t = static_cast<rcStatusBar *>(_o);
        switch (_id) {
        case 0: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: _t->updateStatus((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcStatusBar::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcStatusBar::staticMetaObject = {
    { &QStatusBar::staticMetaObject, qt_meta_stringdata_rcStatusBar,
      qt_meta_data_rcStatusBar, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcStatusBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcStatusBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcStatusBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcStatusBar))
        return static_cast<void*>(const_cast< rcStatusBar*>(this));
    return QStatusBar::qt_metacast(_clname);
}

int rcStatusBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStatusBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
