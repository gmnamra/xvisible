/****************************************************************************
** Meta object code from reading C++ file 'rc_choiceradiobutton.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_choiceradiobutton.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_choiceradiobutton.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcChoiceRadioButton[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      33,   21,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      53,   20,   20,   20, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcChoiceRadioButton[] = {
    "rcChoiceRadioButton\0\0choiceValue\0"
    "choiceSelected(int)\0clickRelay()\0"
};

void rcChoiceRadioButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        rcChoiceRadioButton *_t = static_cast<rcChoiceRadioButton *>(_o);
        switch (_id) {
        case 0: _t->choiceSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->clickRelay(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData rcChoiceRadioButton::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject rcChoiceRadioButton::staticMetaObject = {
    { &QRadioButton::staticMetaObject, qt_meta_stringdata_rcChoiceRadioButton,
      qt_meta_data_rcChoiceRadioButton, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcChoiceRadioButton::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcChoiceRadioButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcChoiceRadioButton::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcChoiceRadioButton))
        return static_cast<void*>(const_cast< rcChoiceRadioButton*>(this));
    return QRadioButton::qt_metacast(_clname);
}

int rcChoiceRadioButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QRadioButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void rcChoiceRadioButton::choiceSelected(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE