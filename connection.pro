#-------------------------------------------------
#
# Project created for RealDeal Application
# Qt Project File (.pro)
#
#-------------------------------------------------

QT       += core gui widgets charts sql multimedia multimediawidgets printsupport network svg openglwidgets

win32: LIBS += -lopengl32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HammerDown
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    loginwindow.cpp \
    homewindow.cpp \
    chatbotdialog.cpp \
    loreguidewidget.cpp \
    buttonanimator.cpp \
    qrcodegen.cpp \
    weatherassistant.cpp \
    imagedropzone.cpp \
    modelingwidget.cpp \
    nexuswidget.cpp \
    nexuswidget_inference.cpp \
    nexuswidget_ui.cpp \
    nexuswidget_maintenance.cpp \
    welcomenotificationbar.cpp \
    costswidget.cpp \
    costswidget_engine.cpp \
    costswidget_tco.cpp \
    costswidget_subtabs.cpp

# Header files
HEADERS += \
    connection.h \
    mainwindow.h \
    loginwindow.h \
    homewindow.h \
    chatbotdialog.h \
    loreguidewidget.h \
    buttonanimator.h \
    qrcodegen.h \
    weatherassistant.h \
    imagedropzone.h \
    modelingwidget.h \
    nexuswidget.h \
    welcomenotificationbar.h \
    costswidget.h

# UI files
FORMS += \
    mainwindow.ui \
    login.ui \
    home.ui \
    order_management.ui \
    client_management.ui \
    employee_management.ui \
    supplier_management.ui \
    equipment_management.ui

# Resource files
RESOURCES += \
    resources.qrc \
    sounds.qrc

## Translation files removed to avoid .qm dependency

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
