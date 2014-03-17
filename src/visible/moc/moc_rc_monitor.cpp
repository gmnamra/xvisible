/****************************************************************************
** Meta object code from reading C++ file 'rc_monitor.h'
**
** Created: Sun Mar 16 19:20:08 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_monitor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_monitor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMonitor[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   11,   10,   10, 0x0a,
      57,   48,   10,   10, 0x0a,
     106,   10,   10,   10, 0x0a,
     127,  122,   10,   10, 0x0a,
     154,   10,   10,   10, 0x0a,
     205,  200,   10,   10, 0x0a,
     229,  200,   10,   10, 0x0a,
     259,   10,   10,   10, 0x0a,
     290,   10,   10,   10, 0x0a,
     315,  310,   10,   10, 0x0a,
     341,   10,   10,   10, 0x0a,
     359,   10,   10,   10, 0x0a,
     376,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMonitor[] = {
    "rcMonitor\0\0image\0updateDisplay(const rcWindow*)\0"
    "graphics\0updateDisplay(const rcVisualGraphicsCollection*)\0"
    "updateDisplay()\0rect\0updateAnalysisRect(rcRect)\0"
    "updateAnalysisRectRotation(rcAffineRectangle)\0"
    "time\0updateTime(rcTimestamp)\0"
    "updateCursorTime(rcTimestamp)\0"
    "updateState(rcExperimentState)\0"
    "updateScale(double)\0size\0"
    "updateMonitorSize(rcRect)\0scaleChanged(int)\0"
    "settingChanged()\0saturationChanged(bool)\0"
};

const QMetaObject rcMonitor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcMonitor,
      qt_meta_data_rcMonitor, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcMonitor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcMonitor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcMonitor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMonitor))
        return static_cast<void*>(const_cast< rcMonitor*>(this));
    return QWidget::qt_metacast(_clname);
}

int rcMonitor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateDisplay((*reinterpret_cast< const rcWindow*(*)>(_a[1]))); break;
        case 1: updateDisplay((*reinterpret_cast< const rcVisualGraphicsCollection*(*)>(_a[1]))); break;
        case 2: updateDisplay(); break;
        case 3: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 4: updateAnalysisRectRotation((*reinterpret_cast< const rcAffineRectangle(*)>(_a[1]))); break;
        case 5: updateTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 6: updateCursorTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 7: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 8: updateScale((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: updateMonitorSize((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 10: scaleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: settingChanged(); break;
        case 12: saturationChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
