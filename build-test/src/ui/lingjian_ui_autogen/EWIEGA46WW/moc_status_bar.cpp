/****************************************************************************
** Meta object code from reading C++ file 'status_bar.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../src/ui/status_bar.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'status_bar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_StatusBar_t {
    uint offsetsAndSizes[22];
    char stringdata0[10];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[5];
    char stringdata5[23];
    char stringdata6[16];
    char stringdata7[18];
    char stringdata8[16];
    char stringdata9[12];
    char stringdata10[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_StatusBar_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_StatusBar_t qt_meta_stringdata_StatusBar = {
    {
        QT_MOC_LITERAL(0, 9),  // "StatusBar"
        QT_MOC_LITERAL(10, 16),  // "inputModeToggled"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 9),  // "InputMode"
        QT_MOC_LITERAL(38, 4),  // "mode"
        QT_MOC_LITERAL(43, 22),  // "punctuationModeToggled"
        QT_MOC_LITERAL(66, 15),  // "PunctuationMode"
        QT_MOC_LITERAL(82, 17),  // "voiceInputClicked"
        QT_MOC_LITERAL(100, 15),  // "keyboardClicked"
        QT_MOC_LITERAL(116, 11),  // "skinClicked"
        QT_MOC_LITERAL(128, 9)   // "aiClicked"
    },
    "StatusBar",
    "inputModeToggled",
    "",
    "InputMode",
    "mode",
    "punctuationModeToggled",
    "PunctuationMode",
    "voiceInputClicked",
    "keyboardClicked",
    "skinClicked",
    "aiClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_StatusBar[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,
       5,    1,   53,    2, 0x06,    3 /* Public */,
       7,    0,   56,    2, 0x06,    5 /* Public */,
       8,    0,   57,    2, 0x06,    6 /* Public */,
       9,    0,   58,    2, 0x06,    7 /* Public */,
      10,    0,   59,    2, 0x06,    8 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject StatusBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_StatusBar.offsetsAndSizes,
    qt_meta_data_StatusBar,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_StatusBar_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StatusBar, std::true_type>,
        // method 'inputModeToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<InputMode, std::false_type>,
        // method 'punctuationModeToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PunctuationMode, std::false_type>,
        // method 'voiceInputClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'keyboardClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'skinClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'aiClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void StatusBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StatusBar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->inputModeToggled((*reinterpret_cast< std::add_pointer_t<InputMode>>(_a[1]))); break;
        case 1: _t->punctuationModeToggled((*reinterpret_cast< std::add_pointer_t<PunctuationMode>>(_a[1]))); break;
        case 2: _t->voiceInputClicked(); break;
        case 3: _t->keyboardClicked(); break;
        case 4: _t->skinClicked(); break;
        case 5: _t->aiClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StatusBar::*)(InputMode );
            if (_t _q_method = &StatusBar::inputModeToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StatusBar::*)(PunctuationMode );
            if (_t _q_method = &StatusBar::punctuationModeToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StatusBar::*)();
            if (_t _q_method = &StatusBar::voiceInputClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StatusBar::*)();
            if (_t _q_method = &StatusBar::keyboardClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StatusBar::*)();
            if (_t _q_method = &StatusBar::skinClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (StatusBar::*)();
            if (_t _q_method = &StatusBar::aiClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *StatusBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StatusBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StatusBar.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int StatusBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void StatusBar::inputModeToggled(InputMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void StatusBar::punctuationModeToggled(PunctuationMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void StatusBar::voiceInputClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StatusBar::keyboardClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void StatusBar::skinClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void StatusBar::aiClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
