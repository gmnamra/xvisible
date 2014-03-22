/****************************************************************************
** Meta object code from reading C++ file 'rc_scalartrackwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_scalartrackwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_scalartrackwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcScalarTrackWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      31,   21,   20,   20, 0x0a,
      53,   48,   20,   20, 0x0a,
      84,   48,   20,   20, 0x0a,
     120,  114,   20,   20, 0x0a,
     151,   20,   20,   20, 0x0a,
     167,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcScalarTrackWidget[] = {
    "rcScalarTrackWidget\0\0isEnabled\0"
    "setEnabled(bool)\0time\0"
    "updateElapsedTime(rcTimestamp)\0"
    "updateCursorTime(rcTimestamp)\0state\0"
    "updateState(rcExperimentState)\0"
    "updateDisplay()\0updateSettings()\0"
};

void rcScalarTrackWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcScalarTrackWidget *_t = static_cast<rcScalarTrackWidget *>(_o);
        switch (_id) {
        case 0: _t->setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->updateElapsedTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 2: _t->updateCursorTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 3: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 4: _t->updateDisplay(); break;
        case 5: _t->updateSettings(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcScalarTrackWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcScalarTrackWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcScalarTrackWidget,
      qt_meta_data_rcScalarTrackWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcScalarTrackWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcScalarTrackWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcScalarTrackWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcScalarTrackWidget))
        return static_cast<void*>(const_cast< rcScalarTrackWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int rcScalarTrackWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
