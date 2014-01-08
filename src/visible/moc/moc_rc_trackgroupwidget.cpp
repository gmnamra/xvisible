/****************************************************************************
** Meta object code from reading C++ file 'rc_trackgroupwidget.h'
**
** Created: Tue Jan 7 14:57:31 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_trackgroupwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_trackgroupwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTrackGroupEnableButton[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets

       0        // eod
};

static const char qt_meta_stringdata_rcTrackGroupEnableButton[] = {
    "rcTrackGroupEnableButton\0"
};

const QMetaObject rcTrackGroupEnableButton::staticMetaObject = {
    { &QPushButton::staticMetaObject, qt_meta_stringdata_rcTrackGroupEnableButton,
      qt_meta_data_rcTrackGroupEnableButton, 0 }
};

const QMetaObject *rcTrackGroupEnableButton::metaObject() const
{
    return &staticMetaObject;
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
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

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

const QMetaObject rcTrackGroupWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcTrackGroupWidget,
      qt_meta_data_rcTrackGroupWidget, 0 }
};

const QMetaObject *rcTrackGroupWidget::metaObject() const
{
    return &staticMetaObject;
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
        switch (_id) {
        case 0: setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 2: updateSettings(); break;
        }
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
