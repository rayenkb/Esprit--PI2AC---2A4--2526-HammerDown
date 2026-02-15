cmake_minimum_required(VERSION 3.16)

project(RealDeal LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    loginwindow.cpp
    loginwindow.h
    homewindow.cpp
    homewindow.h
    login.ui
    home.ui
    order_management.ui
    client_management.ui
    employee_management.ui
    supplier_management.ui
    equipment_management.ui
    resources.qrc
)

add_executable(RealDeal ${PROJECT_SOURCES})

target_link_libraries(RealDeal PRIVATE Qt6::Widgets Qt6::Sql Qt6::Charts)
