/****************************************************************************
** Meta object code from reading C++ file 'rc_filechoicesettingwidget.h'
**
** Created: Sat Mar 15 16:26:27 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_filechoicesettingwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_filechoicesettingwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcFileChoiceSettingWidget[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,   26,   26,   26, 0x0a,
      43,   26,   26,   26, 0x0a,
      58,   26,   26,   26, 0x0a,
      81,   75,   26,   26, 0x0a,
     112,   26,   26,   26, 0x0a,
     131,   26,   26,   26, 0x0a,
     149,   26,   26,   26, 0x0a,
     167,   26,   26,   26, 0x0a,
     191,   26,   26,   26, 0x0a,
     205,   26,   26,   26, 0x0a,
     219,   26,   26,   26, 0x0a,
     234,   26,   26,   26, 0x0a,
     252,  246,   26,   26, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcFileChoiceSettingWidget[] = {
    "rcFileChoiceSettingWidget\0\0browseRequest()\0"
    "valueChanged()\0settingChanged()\0state\0"
    "updateState(rcExperimentState)\0"
    "singleFileBrowse()\0multiFileBrowse()\0"
    "directoryBrowse()\0directoryBrowse4Tiffs()\0"
    "imageImport()\0movieImport()\0tifDirImport()\0"
    "stkImport()\0fname\0recentMovieFile(QString)\0"
};

const QMetaObject rcFileChoiceSettingWidget::staticMetaObject = {
    { &rcSettingWidget::staticMetaObject, qt_meta_stringdata_rcFileChoiceSettingWidget,
      qt_meta_data_rcFileChoiceSettingWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcFileChoiceSettingWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcFileChoiceSettingWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcFileChoiceSettingWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcFileChoiceSettingWidget))
        return static_cast<void*>(const_cast< rcFileChoiceSettingWidget*>(this));
    return rcSettingWidget::qt_metacast(_clname);
}

int rcFileChoiceSettingWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rcSettingWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browseRequest(); break;
        case 1: valueChanged(); break;
        case 2: settingChanged(); break;
        case 3: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 4: singleFileBrowse(); break;
        case 5: multiFileBrowse(); break;
        case 6: directoryBrowse(); break;
        case 7: directoryBrowse4Tiffs(); break;
        case 8: imageImport(); break;
        case 9: movieImport(); break;
        case 10: tifDirImport(); break;
        case 11: stkImport(); break;
        case 12: recentMovieFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
