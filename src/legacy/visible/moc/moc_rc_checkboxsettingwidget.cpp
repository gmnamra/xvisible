/****************************************************************************
** Meta object code from reading C++ file 'rc_checkboxsettingwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_checkboxsettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_checkboxsettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcCheckboxSettingWidget[] = {

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
      34,   25,   24,   24, 0x0a,
      53,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcCheckboxSettingWidget[] = {
    "rcCheckboxSettingWidget\0\0newValue\0"
    "valueChanged(bool)\0settingChanged()\0"
};

void rcCheckboxSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcCheckboxSettingWidget *_t = static_cast<rcCheckboxSettingWidget *>(_o);
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->settingChanged(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcCheckboxSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcCheckboxSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcCheckboxSettingWidget,
      qt_meta_data_rcCheckboxSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcCheckboxSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcCheckboxSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcCheckboxSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcCheckboxSettingWidget))
        return static_cast<void*>(const_cast< rcCheckboxSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcCheckboxSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
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
