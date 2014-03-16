/****************************************************************************
** Meta object code from reading C++ file 'rc_trackmanager.h'
**
** Created: Sat Mar 15 16:27:09 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_trackmanager.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_trackmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTrackManager[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   35,   15,   15, 0x0a,
      72,   15,   15,   15, 0x0a,
      84,   15,   15,   15, 0x0a,
      96,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTrackManager[] = {
    "rcTrackManager\0\0trackhighlighted()\0"
    "state\0updateState(rcExperimentState)\0"
    "addTracks()\0setColors()\0rebuildTracks()\0"
};

const QMetaObject rcTrackManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_rcTrackManager,
      qt_meta_data_rcTrackManager, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcTrackManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcTrackManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcTrackManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTrackManager))
        return static_cast<void*>(const_cast< rcTrackManager*>(this));
    return QObject::qt_metacast(_clname);
}

int rcTrackManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: trackhighlighted(); break;
        case 1: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 2: addTracks(); break;
        case 3: setColors(); break;
        case 4: rebuildTracks(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void rcTrackManager::trackhighlighted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
