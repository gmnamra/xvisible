/****************************************************************************
** Meta object code from reading C++ file 'rc_mainwindow.h'
**
** Created: Tue Jan 7 14:57:03 2014
**      by: The Qt Meta Object Compiler version 59 (Qt 4.4.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 59
#error "This file was generated using the moc from 4.4.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMainWindow[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   10, // methods
       0,    0, // properties
       0,    0, // enums/sets

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMainWindow[] = {
    "rcMainWindow\0\0settingChanged()\0"
};

const QMetaObject rcMainWindow::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_rcMainWindow,
      qt_meta_data_rcMainWindow, 0 }
};

const QMetaObject *rcMainWindow::metaObject() const
{
    return &staticMetaObject;
}

void *rcMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMainWindow))
        return static_cast<void*>(const_cast< rcMainWindow*>(this));
    return QWidget::qt_metacast(_clname);
}

int rcMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: settingChanged(); break;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
