/****************************************************************************
** Meta object code from reading C++ file 'rc_filesavesettingwidget.h'
**
** Created: Tue Jan 7 14:57:01 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_filesavesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_filesavesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcFileSaveSettingWidget[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      25,   24,   24,   24, 0x0a,
      41,   24,   24,   24, 0x0a,
      56,   24,   24,   24, 0x0a,
      79,   73,   24,   24, 0x0a,
     110,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcFileSaveSettingWidget[] = {
    "rcFileSaveSettingWidget\0\0browseRequest()\0"
    "valueChanged()\0settingChanged()\0state\0"
    "updateState(rcExperimentState)\0"
    "movieSave()\0"
};

const QMetaObject rcFileSaveSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcFileSaveSettingWidget,
      qt_meta_data_rcFileSaveSettingWidget, 0 }
};

const QMetaObject *rcFileSaveSettingWidget::metaObject() const
{
    return &staticMetaObject;
}

void *rcFileSaveSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcFileSaveSettingWidget))
        return static_cast<void*>(const_cast< rcFileSaveSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcFileSaveSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browseRequest(); break;
        case 1: valueChanged(); break;
        case 2: settingChanged(); break;
        case 3: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 4: movieSave(); break;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
