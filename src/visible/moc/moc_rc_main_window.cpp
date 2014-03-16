/****************************************************************************
** Meta object code from reading C++ file 'rc_main_window.h'
**
** Created: Fri Mar 14 16:25:51 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_main_window.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_main_window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcMainWindow[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      45,   39,   13,   13, 0x0a,
      76,   13,   13,   13, 0x0a,
      84,   13,   13,   13, 0x0a,
      93,   91,   13,   13, 0x0a,
     110,   13,   13,   13, 0x0a,
     132,  127,   13,   13, 0x0a,
     159,   13,   13,   13, 0x0a,
     168,   13,   13,   13, 0x0a,
     179,   13,   13,   13, 0x0a,
     195,   13,   13,   13, 0x0a,
     217,   13,   13,   13, 0x0a,
     234,   13,   13,   13, 0x0a,
     243,   13,   13,   13, 0x0a,
     264,   13,   13,   13, 0x0a,
     281,   13,   13,   13, 0x0a,
     306,   13,   13,   13, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcMainWindow[] = {
    "rcMainWindow\0\0recentMovieFile(QString)\0"
    "state\0updateState(rcExperimentState)\0"
    "about()\0help()\0i\0inputSource(int)\0"
    "settingChanged()\0rect\0updateAnalysisRect(rcRect)\0"
    "doSave()\0doExport()\0doExportMovie()\0"
    "doExportNativeMovie()\0doExportMatrix()\0"
    "doOpen()\0requestRecentMovie()\0"
    "doOpenSettings()\0requestCellInfoDisplay()\0"
    "reload_plotter(const CurveData*)\0"
};

const QMetaObject rcMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_rcMainWindow,
      qt_meta_data_rcMainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcMainWindow))
        return static_cast<void*>(const_cast< rcMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int rcMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: recentMovieFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 2: about(); break;
        case 3: help(); break;
        case 4: inputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: settingChanged(); break;
        case 6: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 7: doSave(); break;
        case 8: doExport(); break;
        case 9: doExportMovie(); break;
        case 10: doExportNativeMovie(); break;
        case 11: doExportMatrix(); break;
        case 12: doOpen(); break;
        case 13: requestRecentMovie(); break;
        case 14: doOpenSettings(); break;
        case 15: requestCellInfoDisplay(); break;
        case 16: reload_plotter((*reinterpret_cast< const CurveData*(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void rcMainWindow::recentMovieFile(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
