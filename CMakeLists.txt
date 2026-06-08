cmake_minimum_required(VERSION 3.16)

project(HammerDown LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Multimedia
    Charts
    Sql
    PrintSupport
    Network
    OpenGLWidgets
    MultimediaWidgets
    SerialPort
    Concurrent
)
find_package(OpenGL REQUIRED)

# OpenCV is optional — only needed if NexusWidget AI features are enabled
find_package(OpenCV QUIET)
if(OpenCV_FOUND)
    message(STATUS "OpenCV found — AI modeling features enabled")
    add_definitions(-DWITH_OPENCV)
else()
    message(STATUS "OpenCV NOT found — AI modeling features disabled (build will still succeed)")
endif()

set(PROJECT_SOURCES
    main.cpp

    # Core window
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    loginwindow.cpp
    loginwindow.h
    homewindow.cpp
    homewindow.h

    # Communication & auth
    smtpsender.cpp
    smtpsender.h
    voicecommandengine.cpp
    voicecommandengine.h
    connection.h

    # UI Widgets
    chatbotdialog.cpp
    chatbotdialog.h
    loreguidewidget.cpp
    loreguidewidget.h
    buttonanimator.cpp
    buttonanimator.h
    qrcodegen.cpp
    qrcodegen.h
    weatherassistant.cpp
    weatherassistant.h
    imagedropzone.cpp
    imagedropzone.h
    modelingwidget.cpp
    modelingwidget.h
    welcomenotificationbar.cpp
    welcomenotificationbar.h
    creative_components.cpp
    creative_components.h

    # Nexus & Costs
    nexuswidget.h
    nexuswidget.cpp
    nexuswidget_inference.cpp
    nexuswidget_ui.cpp
    nexuswidget_maintenance.cpp
    costswidget.h
    costswidget.cpp
    costswidget_engine.cpp
    costswidget_tco.cpp
    costswidget_subtabs.cpp

    # Feature modules
    equipment.cpp
    equipment.h
    supplier.cpp
    supplier.h
    order.cpp
    order.h
    employee.cpp
    employee.h
    clientmanagement.cpp
    clientmanagement.h

    # UI forms
    login.ui
    home.ui
    order_management.ui
    client_management.ui
    employee_management.ui
    supplier_management.ui
    equipment_management.ui

    # Resources
    resources.qrc
    sounds.qrc
)

add_executable(HammerDown ${PROJECT_SOURCES})

target_link_libraries(HammerDown PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Charts
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Network
    Qt6::OpenGLWidgets
    Qt6::MultimediaWidgets
    Qt6::SerialPort
    Qt6::Concurrent
    OpenGL::GL
)

if(OpenCV_FOUND)
    target_include_directories(HammerDown PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(HammerDown PRIVATE ${OpenCV_LIBS})
endif()
