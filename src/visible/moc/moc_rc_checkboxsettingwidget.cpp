/****************************************************************************
** Meta object code from reading C++ file 'rc_checkboxsettingwidget.h'
**
** Created: Thu Feb 13 00:41:16 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_checkboxsettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_checkboxsettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcCheckboxSettingWidget[] = {

 // content:
       4,       // revision
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

const QMetaObject rcCheckboxSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcCheckboxSettingWidget,
      qt_meta_data_rcCheckboxSettingWidget, 0 }
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
        switch (_id) {
        case 0: valueChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: settingChanged(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
