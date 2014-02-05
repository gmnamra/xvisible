/****************************************************************************
** Meta object code from reading C++ file 'rc_modeldomain.h'
**
** Created: Tue Feb 4 20:57:52 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_modeldomain.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_modeldomain.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcModelDomain[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      42,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      25,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   15,   14,   14, 0x05,
      45,   15,   14,   14, 0x05,
      75,   69,   14,   14, 0x05,
     109,  103,   14,   14, 0x05,
     149,  140,   14,   14, 0x05,
     198,   14,   14,   14, 0x05,
     220,  215,   14,   14, 0x05,
     254,  247,   14,   14, 0x05,
     310,  300,   14,   14, 0x05,
     361,  355,   14,   14, 0x05,
     411,  400,   14,   14, 0x05,
     436,   14,   14,   14, 0x05,
     465,  452,   14,   14, 0x05,
     491,   14,   14,   14, 0x05,
     505,   14,   14,   14, 0x05,
     520,   14,   14,   14, 0x05,
     534,   14,   14,   14, 0x05,
     546,   14,   14,   14, 0x05,
     560,  558,   14,   14, 0x05,
     595,  583,   14,   14, 0x05,
     622,   14,   14,   14, 0x05,
     645,  215,   14,   14, 0x05,
     692,  669,   14,   14, 0x05,
     721,   14,   14,   14, 0x05,
     741,  739,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
     768,   14,   14,   14, 0x0a,
     783,   14,   14,   14, 0x0a,
     797,   14,   14,   14, 0x0a,
     814,   14,   14,   14, 0x0a,
     827,   14,   14,   14, 0x0a,
     848,  843,   14,   14, 0x0a,
     884,   14,   14,   14, 0x0a,
     906,  899,   14,   14, 0x0a,
     942,   14,   14,   14, 0x0a,
     963,   14,   14,   14, 0x0a,
     985,   14,   14,   14, 0x0a,
    1006,   14,   14,   14, 0x0a,
    1025,   14,   14,   14, 0x0a,
    1044,   14,   14,   14, 0x0a,
    1068,   14,   14,   14, 0x0a,
    1080,   14,   14,   14, 0x0a,
    1103,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcModelDomain[] = {
    "rcModelDomain\0\0time\0elapsedTime(rcTimestamp)\0"
    "cursorTime(rcTimestamp)\0state\0"
    "newState(rcExperimentState)\0image\0"
    "updateDisplay(const rcWindow*)\0graphics\0"
    "updateDisplay(const rcVisualGraphicsCollection*)\0"
    "updateSettings()\0rect\0updateAnalysisRect(rcRect)\0"
    "affine\0updateAnalysisRectRotation(rcAffineRectangle)\0"
    "start,end\0updateTimelineRange(rcTimestamp,rcTimestamp)\0"
    "scale\0updateTimelineScale(rcResultScaleMode)\0"
    "multiplier\0updateMultiplier(double)\0"
    "updateDisplay()\0statusString\0"
    "updateStatus(const char*)\0imageImport()\0"
    "tifDirImport()\0movieImport()\0stkImport()\0"
    "movieSave()\0i\0updateInputSource(int)\0"
    "scaleFactor\0updateMonitorScale(double)\0"
    "updateMonitorDisplay()\0updateVideoRect(rcRect)\0"
    "liveCamera,liveStorage\0"
    "updateCameraState(bool,bool)\0"
    "updateDebugging()\0b\0updateSelectionState(bool)\0"
    "requestStart()\0requestStop()\0"
    "requestProcess()\0requestNew()\0"
    "requestNewApp()\0mode\0"
    "requestOpen(rcExperimentImportMode)\0"
    "requestClose()\0format\0"
    "requestSave(rcExperimentFileFormat)\0"
    "requestImageImport()\0requestTifDirImport()\0"
    "requestMovieImport()\0requestSTKImport()\0"
    "requestMovieSave()\0requestInputSource(int)\0"
    "timerTick()\0requestTrackingPause()\0"
    "stopTrackingPause()\0"
};

const QMetaObject rcModelDomain::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_rcModelDomain,
      qt_meta_data_rcModelDomain, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcModelDomain::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcModelDomain::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcModelDomain::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcModelDomain))
        return static_cast<void*>(const_cast< rcModelDomain*>(this));
    if (!strcmp(_clname, "rcExperimentObserver"))
        return static_cast< rcExperimentObserver*>(const_cast< rcModelDomain*>(this));
    return QObject::qt_metacast(_clname);
}

int rcModelDomain::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: elapsedTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 1: cursorTime((*reinterpret_cast< const rcTimestamp(*)>(_a[1]))); break;
        case 2: newState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 3: updateDisplay((*reinterpret_cast< const rcWindow*(*)>(_a[1]))); break;
        case 4: updateDisplay((*reinterpret_cast< const rcVisualGraphicsCollection*(*)>(_a[1]))); break;
        case 5: updateSettings(); break;
        case 6: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 7: updateAnalysisRectRotation((*reinterpret_cast< const rcAffineRectangle(*)>(_a[1]))); break;
        case 8: updateTimelineRange((*reinterpret_cast< const rcTimestamp(*)>(_a[1])),(*reinterpret_cast< const rcTimestamp(*)>(_a[2]))); break;
        case 9: updateTimelineScale((*reinterpret_cast< rcResultScaleMode(*)>(_a[1]))); break;
        case 10: updateMultiplier((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 11: updateDisplay(); break;
        case 12: updateStatus((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        case 13: imageImport(); break;
        case 14: tifDirImport(); break;
        case 15: movieImport(); break;
        case 16: stkImport(); break;
        case 17: movieSave(); break;
        case 18: updateInputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 19: updateMonitorScale((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 20: updateMonitorDisplay(); break;
        case 21: updateVideoRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 22: updateCameraState((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 23: updateDebugging(); break;
        case 24: updateSelectionState((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 25: requestStart(); break;
        case 26: requestStop(); break;
        case 27: requestProcess(); break;
        case 28: requestNew(); break;
        case 29: requestNewApp(); break;
        case 30: requestOpen((*reinterpret_cast< rcExperimentImportMode(*)>(_a[1]))); break;
        case 31: requestClose(); break;
        case 32: requestSave((*reinterpret_cast< rcExperimentFileFormat(*)>(_a[1]))); break;
        case 33: requestImageImport(); break;
        case 34: requestTifDirImport(); break;
        case 35: requestMovieImport(); break;
        case 36: requestSTKImport(); break;
        case 37: requestMovieSave(); break;
        case 38: requestInputSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 39: timerTick(); break;
        case 40: requestTrackingPause(); break;
        case 41: stopTrackingPause(); break;
        default: ;
        }
        _id -= 42;
    }
    return _id;
}

// SIGNAL 0
void rcModelDomain::elapsedTime(const rcTimestamp & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void rcModelDomain::cursorTime(const rcTimestamp & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void rcModelDomain::newState(rcExperimentState _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void rcModelDomain::updateDisplay(const rcWindow * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void rcModelDomain::updateDisplay(const rcVisualGraphicsCollection * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void rcModelDomain::updateSettings()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void rcModelDomain::updateAnalysisRect(const rcRect & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void rcModelDomain::updateAnalysisRectRotation(const rcAffineRectangle & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void rcModelDomain::updateTimelineRange(const rcTimestamp & _t1, const rcTimestamp & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void rcModelDomain::updateTimelineScale(rcResultScaleMode _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void rcModelDomain::updateMultiplier(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void rcModelDomain::updateDisplay()
{
    QMetaObject::activate(this, &staticMetaObject, 11, 0);
}

// SIGNAL 12
void rcModelDomain::updateStatus(const char * _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void rcModelDomain::imageImport()
{
    QMetaObject::activate(this, &staticMetaObject, 13, 0);
}

// SIGNAL 14
void rcModelDomain::tifDirImport()
{
    QMetaObject::activate(this, &staticMetaObject, 14, 0);
}

// SIGNAL 15
void rcModelDomain::movieImport()
{
    QMetaObject::activate(this, &staticMetaObject, 15, 0);
}

// SIGNAL 16
void rcModelDomain::stkImport()
{
    QMetaObject::activate(this, &staticMetaObject, 16, 0);
}

// SIGNAL 17
void rcModelDomain::movieSave()
{
    QMetaObject::activate(this, &staticMetaObject, 17, 0);
}

// SIGNAL 18
void rcModelDomain::updateInputSource(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 18, _a);
}

// SIGNAL 19
void rcModelDomain::updateMonitorScale(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 19, _a);
}

// SIGNAL 20
void rcModelDomain::updateMonitorDisplay()
{
    QMetaObject::activate(this, &staticMetaObject, 20, 0);
}

// SIGNAL 21
void rcModelDomain::updateVideoRect(const rcRect & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 21, _a);
}

// SIGNAL 22
void rcModelDomain::updateCameraState(bool _t1, bool _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 22, _a);
}

// SIGNAL 23
void rcModelDomain::updateDebugging()
{
    QMetaObject::activate(this, &staticMetaObject, 23, 0);
}

// SIGNAL 24
void rcModelDomain::updateSelectionState(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 24, _a);
}
QT_END_MOC_NAMESPACE
