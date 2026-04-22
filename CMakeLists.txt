cmake_minimum_required(VERSION 3.16)

project(HammerDown LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia Charts Sql PrintSupport Network OpenGLWidgets MultimediaWidgets SerialPort)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    loginwindow.cpp
    loginwindow.h
    homewindow.cpp
    homewindow.h
    smtpsender.cpp
    smtpsender.h
    voicecommandengine.cpp
    voicecommandengine.h
    chatbotdialog.cpp
    chatbotdialog.h
    weatherassistant.cpp
    weatherassistant.h
    imagedropzone.cpp
    imagedropzone.h
    qrcodegen.cpp
    qrcodegen.h
    buttonanimator.cpp
    buttonanimator.h
    modelingwidget.cpp
    modelingwidget.h
    nexuswidget.h
    nexuswidget.cpp
    nexuswidget_inference.cpp
    nexuswidget_ui.cpp
    nexuswidget_maintenance.cpp
    welcomenotificationbar.cpp
    welcomenotificationbar.h
    costswidget.h
    costswidget.cpp
    costswidget_engine.cpp
    costswidget_tco.cpp
    costswidget_subtabs.cpp
    login.ui
    home.ui
    order_management.ui
    client_management.ui
    employee_management.ui
    supplier_management.ui
    equipment_management.ui
    resources.qrc
    sounds.qrc
)

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(HammerDown ${PROJECT_SOURCES})

target_link_libraries(HammerDown PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::Charts Qt6::Sql Qt6::PrintSupport Qt6::Network Qt6::OpenGLWidgets Qt6::MultimediaWidgets Qt6::SerialPort OpenGL::GL ${OpenCV_LIBS})

# Add translations if they exist
# install(TARGETS HammerDown RUNTIME DESTINATION bin)
