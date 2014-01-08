/****************************************************************************
** Meta object code from reading C++ file 'rc_radiochoicesettingwidget.h'
**
** Created: Tue Jan 7 14:57:10 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_radiochoicesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_radiochoicesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcRadioChoiceSettingWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      40,   28,   27,   27, 0x0a,
      60,   27,   27,   27, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcRadioChoiceSettingWidget[] = {
    "rcRadioChoiceSettingWidget\0\0choiceValue\0"
    "choiceSelected(int)\0settingChanged()\0"
};

const QMetaObject rcRadioChoiceSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcRadioChoiceSettingWidget,
      qt_meta_data_rcRadioChoiceSettingWidget, 0 }
};

const QMetaObject *rcRadioChoiceSettingWidget::metaObject() const
{
    return &staticMetaObject;
}

void *rcRadioChoiceSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcRadioChoiceSettingWidget))
        return static_cast<void*>(const_cast< rcRadioChoiceSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcRadioChoiceSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: choiceSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: settingChanged(); break;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
