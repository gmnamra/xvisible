/****************************************************************************
** Meta object code from reading C++ file 'rc_frameratechoicesettingwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_frameratechoicesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_frameratechoicesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcFramerateChoiceSettingWidget[] = {

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
      44,   32,   31,   31, 0x0a,
      64,   31,   31,   31, 0x0a,
      87,   81,   31,   31, 0x0a,
     111,   81,   31,   31, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcFramerateChoiceSettingWidget[] = {
    "rcFramerateChoiceSettingWidget\0\0"
    "choiceValue\0choiceSelected(int)\0"
    "settingChanged()\0value\0settingChanged(QString)\0"
    "intervalChanged(QString)\0"
};

void rcFramerateChoiceSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcFramerateChoiceSettingWidget *_t = static_cast<rcFramerateChoiceSettingWidget *>(_o);
        switch (_id) {
        case 0: _t->choiceSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->settingChanged(); break;
        case 2: _t->settingChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->intervalChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcFramerateChoiceSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcFramerateChoiceSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcFramerateChoiceSettingWidget,
      qt_meta_data_rcFramerateChoiceSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcFramerateChoiceSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcFramerateChoiceSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcFramerateChoiceSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcFramerateChoiceSettingWidget))
        return static_cast<void*>(const_cast< rcFramerateChoiceSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcFramerateChoiceSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
static const uint qt_meta_data_rcRateChoiceSettingWidget[] = {

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
      39,   27,   26,   26, 0x0a,
      59,   26,   26,   26, 0x0a,
      82,   76,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcRateChoiceSettingWidget[] = {
    "rcRateChoiceSettingWidget\0\0choiceValue\0"
    "choiceSelected(int)\0settingChanged()\0"
    "value\0settingChanged(QString)\0"
};

void rcRateChoiceSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcRateChoiceSettingWidget *_t = static_cast<rcRateChoiceSettingWidget *>(_o);
        switch (_id) {
        case 0: _t->choiceSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->settingChanged(); break;
        case 2: _t->settingChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcRateChoiceSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcRateChoiceSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcRateChoiceSettingWidget,
      qt_meta_data_rcRateChoiceSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcRateChoiceSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcRateChoiceSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcRateChoiceSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcRateChoiceSettingWidget))
        return static_cast<void*>(const_cast< rcRateChoiceSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcRateChoiceSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
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