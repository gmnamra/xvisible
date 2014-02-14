/****************************************************************************
** Meta object code from reading C++ file 'rc_settingpanel.h'
**
** Created: Thu Feb 13 10:02:36 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_settingpanel.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_settingpanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcSettingPanel[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      22,   16,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcSettingPanel[] = {
    "rcSettingPanel\0\0state\0"
    "updateState(rcExperimentState)\0"
};

const QMetaObject rcSettingPanel::staticMetaObject = {
    { &QTabWidget::staticMetaObject, qt_meta_stringdata_rcSettingPanel,
      qt_meta_data_rcSettingPanel, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcSettingPanel::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcSettingPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcSettingPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcSettingPanel))
        return static_cast<void*>(const_cast< rcSettingPanel*>(this));
    return QTabWidget::qt_metacast(_clname);
}

int rcSettingPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
