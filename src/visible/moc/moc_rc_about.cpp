/****************************************************************************
** Meta object code from reading C++ file 'rc_about.h'
**
** Created: Thu Feb 13 00:41:13 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_about.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_about.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcAbout[] = {

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
       9,    8,    8,    8, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_rcAbout[] = {
    "rcAbout\0\0languageChange()\0"
};

const QMetaObject rcAbout::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_rcAbout,
      qt_meta_data_rcAbout, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcAbout::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcAbout::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcAbout::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcAbout))
        return static_cast<void*>(const_cast< rcAbout*>(this));
    return QDialog::qt_metacast(_clname);
}

int rcAbout::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: languageChange(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
