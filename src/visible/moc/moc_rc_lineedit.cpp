/****************************************************************************
** Meta object code from reading C++ file 'rc_lineedit.h'
**
** Created: Tue Jan 7 14:57:02 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_lineedit.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_lineedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcLineEdit[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
      17,   12,   11,   11, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,   11,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcLineEdit[] = {
    "rcLineEdit\0\0text\0textCommited(QString)\0"
    "relayTextCommited()\0"
};

const QMetaObject rcLineEdit::staticMetaObject = {
    { &QLineEdit::staticMetaObject, qt_meta_stringdata_rcLineEdit,
      qt_meta_data_rcLineEdit, 0 }
};

const QMetaObject *rcLineEdit::metaObject() const
{
    return &staticMetaObject;
}

void *rcLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcLineEdit))
        return static_cast<void*>(const_cast< rcLineEdit*>(this));
    return QLineEdit::qt_metacast(_clname);
}

int rcLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: textCommited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: relayTextCommited(); break;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void rcLineEdit::textCommited(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
