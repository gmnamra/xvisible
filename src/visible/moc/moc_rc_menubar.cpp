/****************************************************************************
** Meta object code from reading C++ file 'rc_menubar.h'
**
** Created: Fri Mar 7 14:40:23 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_menubar.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_menubar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMenuBar[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   11,   10,   10, 0x0a,
      48,   10,   10,   10, 0x0a,
      56,   10,   10,   10, 0x0a,
      65,   63,   10,   10, 0x0a,
      87,   82,   10,   10, 0x0a,
     114,   10,   10,   10, 0x0a,
     123,   10,   10,   10, 0x0a,
     134,   10,   10,   10, 0x0a,
     150,   10,   10,   10, 0x0a,
     172,   10,   10,   10, 0x0a,
     181,   10,   10,   10, 0x0a,
     198,   10,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMenuBar[] = {
    "rcMenuBar\0\0state\0updateState(rcExperimentState)\0"
    "about()\0help()\0i\0inputSource(int)\0"
    "rect\0updateAnalysisRect(rcRect)\0"
    "doSave()\0doExport()\0doExportMovie()\0"
    "doExportNativeMovie()\0doOpen()\0"
    "doOpenSettings()\0requestCellInfoDisplay()\0"
};

const QMetaObject rcMenuBar::staticMetaObject = {
    { &QMenuBar::staticMetaObject, qt_meta_stringdata_rcMenuBar,
      qt_meta_data_rcMenuBar, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcMenuBar::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcMenuBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcMenuBar::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMenuBar))
        return static_cast<void*>(const_cast< rcMenuBar*>(this));
    return QMenuBar::qt_metacast(_clname);
}

int rcMenuBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenuBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: about(); break;
        case 2: help(); break;
        case 3: inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 5: doSave(); break;
        case 6: doExport(); break;
        case 7: doExportMovie(); break;
        case 8: doExportNativeMovie(); break;
        case 9: doOpen(); break;
        case 10: doOpenSettings(); break;
        case 11: requestCellInfoDisplay(); break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
