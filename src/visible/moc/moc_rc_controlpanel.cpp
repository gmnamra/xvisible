/****************************************************************************
** Meta object code from reading C++ file 'rc_controlpanel.h'
**
** Created: Sat Mar 15 16:26:23 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_controlpanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_controlpanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcControlPanel[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   16,   15,   15, 0x0a,
      52,   16,   15,   15, 0x0a,
      88,   82,   15,   15, 0x0a,
     124,  119,   15,   15, 0x0a,
     151,   15,   15,   15, 0x0a,
     170,  168,   15,   15, 0x0a,
     187,   15,   15,   15, 0x0a,
     219,  206,   15,   15, 0x0a,
     246,   15,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcControlPanel[] = {
    "rcControlPanel\0\0time\0"
    "updateElapsedTime(rcTimestamp)\0"
    "updateCursorTime(rcTimestamp)\0state\0"
    "updateState(rcExperimentState)\0rect\0"
    "updateAnalysisRect(rcRect)\0settingChanged()\0"
    "i\0inputSource(int)\0updateCursorTime()\0"
    "trueIsSelect\0updateSelectionState(bool)\0"
    "selectAll()\0"
};

const QMetaObject rcControlPanel::staticMetaObject = {
    { &Q3Frame::staticMetaObject, qt_meta_stringdata_rcControlPanel,
      qt_meta_data_rcControlPanel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcControlPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcControlPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcControlPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcControlPanel))
        return static_cast<void*>(const_cast< rcControlPanel*>(this));
    return Q3Frame::qt_metacast(_clname);
}

int rcControlPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateElapsedTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 1: updateCursorTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 2: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 3: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 4: settingChanged(); break;
        case 5: inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: updateCursorTime(); break;
        case 7: updateSelectionState((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: selectAll(); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
