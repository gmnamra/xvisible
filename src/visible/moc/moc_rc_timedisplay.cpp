/****************************************************************************
** Meta object code from reading C++ file 'rc_timedisplay.h'
**
** Created: Tue Jan 7 14:57:18 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_timedisplay.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_timedisplay.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTimeDisplay[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      20,   15,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTimeDisplay[] = {
    "rcTimeDisplay\0\0time\0newTime(rcTimestamp)\0"
};

const QMetaObject rcTimeDisplay::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_rcTimeDisplay,
      qt_meta_data_rcTimeDisplay, 0 }
};

const QMetaObject *rcTimeDisplay::metaObject() const
{
    return &staticMetaObject;
}

void *rcTimeDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTimeDisplay))
        return static_cast<void*>(const_cast< rcTimeDisplay*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int rcTimeDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: newTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
