cmake_minimum_required(VERSION 3.16)

project(SmartTimerApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Джерела
set(SOURCES
    main.cpp
    mainwindow.cpp
    TimerManager.cpp
    EditTimerDialog.cpp       # Редагування таймера
    AddTimerDialog.cpp        # Додавання нового таймера
)

# Хедери
set(HEADERS
    mainwindow.h
    TimerManager.h
    EditTimerDialog.h
    AddTimerDialog.h
)

# UI файли
set(UIS
    mainwindow.ui
)

qt6_add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UIS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
