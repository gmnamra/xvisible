/****************************************************************************
** Meta object code from reading C++ file 'rc_textlinesettingwidget.h'
**
** Created: Tue Jan 7 14:57:17 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_textlinesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_textlinesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTextLineSettingWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      30,   25,   24,   24, 0x0a,
      52,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTextLineSettingWidget[] = {
    "rcTextLineSettingWidget\0\0text\0"
    "valueChanged(QString)\0settingChanged()\0"
};

const QMetaObject rcTextLineSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcTextLineSettingWidget,
      qt_meta_data_rcTextLineSettingWidget, 0 }
};

const QMetaObject *rcTextLineSettingWidget::metaObject() const
{
    return &staticMetaObject;
}

void *rcTextLineSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTextLineSettingWidget))
        return static_cast<void*>(const_cast< rcTextLineSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcTextLineSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: valueChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: settingChanged(); break;
        }
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
