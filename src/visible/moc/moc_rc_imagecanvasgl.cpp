/****************************************************************************
** Meta object code from reading C++ file 'rc_imagecanvasgl.h'
**
** Created: Thu Feb 13 00:42:10 2014
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../include/visUI/rc_imagecanvasgl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rc_imagecanvasgl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_rcImageCanvasGL[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x0a,
      53,   48,   16,   16, 0x0a,
      87,   80,   16,   16, 0x0a,
     135,  133,   16,   16, 0x0a,
     161,   16,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_rcImageCanvasGL[] = {
    "rcImageCanvasGL\0\0updateState(rcExperimentState)\0"
    "rect\0updateAnalysisRect(rcRect)\0affine\0"
    "updateAnalysisRectRotation(rcAffineRectangle)\0"
    "s\0showSaturatedPixels(bool)\0"
    "selectButtonRelease()\0"
};

const QMetaObject rcImageCanvasGL::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_rcImageCanvasGL,
      qt_meta_data_rcImageCanvasGL, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &rcImageCanvasGL::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *rcImageCanvasGL::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *rcImageCanvasGL::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_rcImageCanvasGL))
        return static_cast<void*>(const_cast< rcImageCanvasGL*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int rcImageCanvasGL::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updateState((*reinterpret_cast< rcExperimentState(*)>(_a[1]))); break;
        case 1: updateAnalysisRect((*reinterpret_cast< const rcRect(*)>(_a[1]))); break;
        case 2: updateAnalysisRectRotation((*reinterpret_cast< const rcAffineRectangle(*)>(_a[1]))); break;
        case 3: showSaturatedPixels((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: selectButtonRelease(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
