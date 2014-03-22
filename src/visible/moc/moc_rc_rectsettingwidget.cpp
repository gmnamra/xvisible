/****************************************************************************
** Meta object code from reading C++ file 'rc_rectsettingwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_rectsettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_rectsettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcRectSettingWidget[] = {

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
      21,   20,   20,   20, 0x0a,
      36,   20,   20,   20, 0x0a,
      53,   20,   20,   20, 0x0a,
      76,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcRectSettingWidget[] = {
    "rcRectSettingWidget\0\0valueChanged()\0"
    "settingChanged()\0settingChanged(rcRect)\0"
    "selectAll()\0"
};

void rcRectSettingWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcRectSettingWidget *_t = static_cast<rcRectSettingWidget *>(_o);
        switch (_id) {
        case 0: _t->valueChanged(); break;
        case 1: _t->settingChanged(); break;
        case 2: _t->settingChanged((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 3: _t->selectAll(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcRectSettingWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcRectSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcRectSettingWidget,
      qt_meta_data_rcRectSettingWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcRectSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcRectSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcRectSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcRectSettingWidget))
        return static_cast<void*>(const_cast< rcRectSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcRectSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
