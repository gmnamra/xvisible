/****************************************************************************
** Meta object code from reading C++ file 'rc_textedit.h'
**
** Created: Sun Mar 16 19:20:19 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_textedit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_textedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcTextEdit[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcTextEdit[] = {
    "rcTextEdit\0\0text\0textCommited(QString)\0"
    "relayTextCommited()\0"
};

const QMetaObject rcTextEdit::staticMetaObject = {
    { &Q3TextEdit::staticMetaObject, qt_meta_stringdata_rcTextEdit,
      qt_meta_data_rcTextEdit, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcTextEdit::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcTextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcTextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcTextEdit))
        return static_cast<void*>(const_cast< rcTextEdit*>(this));
    return Q3TextEdit::qt_metacast(_clname);
}

int rcTextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Q3TextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: textCommited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: relayTextCommited(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void rcTextEdit::textCommited(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
