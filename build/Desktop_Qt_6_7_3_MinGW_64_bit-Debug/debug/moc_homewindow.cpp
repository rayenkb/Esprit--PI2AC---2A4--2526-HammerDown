/****************************************************************************
** Meta object code from reading C++ file 'homewindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../homewindow.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'homewindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSHomeWindowENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSHomeWindowENDCLASS = QtMocHelpers::stringData(
    "HomeWindow",
    "employesClicked",
    "",
    "clientClicked",
    "orderClicked",
    "fournisseurClicked",
    "equipmentClicked",
    "languageChanged",
    "language",
    "volumeChanged",
    "volume",
    "disconnectClicked",
    "handleEmployes",
    "handleClient",
    "handleOrder",
    "handleFournisseur",
    "handleEquipment",
    "handleSettingsClicked",
    "handleDisconnect",
    "handleChatBot",
    "handleHelp"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSHomeWindowENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  116,    2, 0x06,    1 /* Public */,
       3,    0,  117,    2, 0x06,    2 /* Public */,
       4,    0,  118,    2, 0x06,    3 /* Public */,
       5,    0,  119,    2, 0x06,    4 /* Public */,
       6,    0,  120,    2, 0x06,    5 /* Public */,
       7,    1,  121,    2, 0x06,    6 /* Public */,
       9,    1,  124,    2, 0x06,    8 /* Public */,
      11,    0,  127,    2, 0x06,   10 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    0,  128,    2, 0x08,   11 /* Private */,
      13,    0,  129,    2, 0x08,   12 /* Private */,
      14,    0,  130,    2, 0x08,   13 /* Private */,
      15,    0,  131,    2, 0x08,   14 /* Private */,
      16,    0,  132,    2, 0x08,   15 /* Private */,
      17,    0,  133,    2, 0x08,   16 /* Private */,
      18,    0,  134,    2, 0x08,   17 /* Private */,
      19,    0,  135,    2, 0x08,   18 /* Private */,
      20,    0,  136,    2, 0x08,   19 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QReal,   10,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject HomeWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_CLASSHomeWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSHomeWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSHomeWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<HomeWindow, std::true_type>,
        // method 'employesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clientClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'orderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'fournisseurClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'equipmentClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'languageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'volumeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        // method 'disconnectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleEmployes'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleClient'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleOrder'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleFournisseur'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleEquipment'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleSettingsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleDisconnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleChatBot'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleHelp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void HomeWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HomeWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->employesClicked(); break;
        case 1: _t->clientClicked(); break;
        case 2: _t->orderClicked(); break;
        case 3: _t->fournisseurClicked(); break;
        case 4: _t->equipmentClicked(); break;
        case 5: _t->languageChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->volumeChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 7: _t->disconnectClicked(); break;
        case 8: _t->handleEmployes(); break;
        case 9: _t->handleClient(); break;
        case 10: _t->handleOrder(); break;
        case 11: _t->handleFournisseur(); break;
        case 12: _t->handleEquipment(); break;
        case 13: _t->handleSettingsClicked(); break;
        case 14: _t->handleDisconnect(); break;
        case 15: _t->handleChatBot(); break;
        case 16: _t->handleHelp(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::employesClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::clientClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::orderClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::fournisseurClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::equipmentClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)(const QString & );
            if (_t _q_method = &HomeWindow::languageChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)(qreal );
            if (_t _q_method = &HomeWindow::volumeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (HomeWindow::*)();
            if (_t _q_method = &HomeWindow::disconnectClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *HomeWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *HomeWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSHomeWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int HomeWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void HomeWindow::employesClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void HomeWindow::clientClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void HomeWindow::orderClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void HomeWindow::fournisseurClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void HomeWindow::equipmentClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void HomeWindow::languageChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void HomeWindow::volumeChanged(qreal _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void HomeWindow::disconnectClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
