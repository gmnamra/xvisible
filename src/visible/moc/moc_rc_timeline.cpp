/****************************************************************************
** Meta object code from reading C++ file 'rc_timeline.h'
**
** Created: Tue Jan 7 14:57:25 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_timeline.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_timeline.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTimeline[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x0a,
      41,   12,   11,   11, 0x0a,
      77,   71,   11,   11, 0x0a,
     118,  108,   11,   11, 0x0a,
     155,   11,   11,   11, 0x0a,
     186,   11,   11,   11, 0x0a,
     202,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTimeline[] = {
    "rcTimeline\0\0time\0updateTime(rcTimestamp)\0"
    "updateCursorTime(rcTimestamp)\0state\0"
    "updateState(rcExperimentState)\0start,end\0"
    "updateRange(rcTimestamp,rcTimestamp)\0"
    "updateScale(rcResultScaleMode)\0"
    "updateDisplay()\0updateSettings()\0"
};

const QMetaObject rcTimeline::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcTimeline,
      qt_meta_data_rcTimeline, 0 }
};

const QMetaObject *rcTimeline::metaObject() const
{
    return &staticMetaObject;
}

void *rcTimeline::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTimeline))
        return static_cast<void*>(const_cast< rcTimeline*>(this));
    return QWidget::qt_metacast(_clname);
}

int rcTimeline::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 1: updateCursorTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 2: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 3: updateRange((*reinterpret_cast< const rcTimestamp(*)>(_a[1])),(*reinterpret_cast< const rcTimestamp(*)>(_a[2]))); break;
        case 4: updateScale((*reinterpret_cast< rcResultScaleMode(*)>(_a[1]))); break;
        case 5: updateDisplay(); break;
        case 6: updateSettings(); break;
        }
        _id -= 7;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
