/****************************************************************************
** Meta object code from reading C++ file 'rc_filesavesettingwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_filesavesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_filesavesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcFileSaveSettingWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

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

void rcFileSaveSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcFileSaveSettingWidget *_t = static_cast<rcFileSaveSettingWidget *>(_o);
        switch (_id) {
        case 0: _t->browseRequest(); break;
        case 1: _t->valueChanged(); break;
        case 2: _t->settingChanged(); break;
        case 3: _t->updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 4: _t->movieSave(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcFileSaveSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcFileSaveSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcFileSaveSettingWidget,
      qt_meta_data_rcFileSaveSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcFileSaveSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcFileSaveSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
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
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE