/****************************************************************************
** Meta object code from reading C++ file 'rc_trackgroupwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_trackgroupwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_trackgroupwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTrackGroupEnableButton[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_rcTrackGroupEnableButton[] = {
    "rcTrackGroupEnableButton\0"
};

void rcTrackGroupEnableButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData rcTrackGroupEnableButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcTrackGroupEnableButton::staticMetaObject = {
    { &QPushButton::staticMetaObject, qt_meta_stringdata_rcTrackGroupEnableButton,
      qt_meta_data_rcTrackGroupEnableButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcTrackGroupEnableButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcTrackGroupEnableButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcTrackGroupEnableButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTrackGroupEnableButton))
        return static_cast<void*>(const_cast< rcTrackGroupEnableButton*>(this));
    return QPushButton::qt_metacast(_clname);
}

int rcTrackGroupEnableButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_rcTrackGroupWidget[] = {

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
      30,   20,   19,   19, 0x0a,
      53,   47,   19,   19, 0x0a,
      84,   19,   19,   19, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTrackGroupWidget[] = {
    "rcTrackGroupWidget\0\0isEnabled\0"
    "setEnabled(bool)\0state\0"
    "updateState(rcExperimentState)\0"
    "updateSettings()\0"
};

void rcTrackGroupWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcTrackGroupWidget *_t = static_cast<rcTrackGroupWidget *>(_o);
        switch (_id) {
        case 0: _t->setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 2: _t->updateSettings(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcTrackGroupWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcTrackGroupWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcTrackGroupWidget,
      qt_meta_data_rcTrackGroupWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcTrackGroupWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcTrackGroupWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcTrackGroupWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTrackGroupWidget))
        return static_cast<void*>(const_cast< rcTrackGroupWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int rcTrackGroupWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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