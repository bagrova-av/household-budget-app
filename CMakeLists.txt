cmake_minimum_required(VERSION 3.16)

project(household-budget VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Charts PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Charts PrintSupport)

find_library(BCRYPT_LIBRARY bcrypt)

set(PROJECT_SOURCES
        main.cpp
        ui/mainwindow.cpp
        ui/mainwindow.h
        ui/mainwindow.ui
        app/authmanager.cpp
        app/authmanager.h
        app/databasemanager.cpp
        app/databasemanager.h
        app/configmanager.cpp
        app/configmanager.h
        models/articlemodel.cpp
        models/articlemodel.h
        models/balancemodel.cpp
        models/balancemodel.h
        models/operationmodel.cpp
        models/operationmodel.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(household-budget
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        app/databasemanager.h app/databasemanager.cpp
        app/authmanager.h app/authmanager.cpp

        models/operationmodel.h models/operationmodel.cpp
        models/balancemodel.h models/balancemodel.cpp
        models/articlemodel.h models/articlemodel.cpp
        app/configmanager.h app/configmanager.cpp
        ui/authorizationwindow.h ui/authorizationwindow.cpp ui/authorizationwindow.ui
        ui/operationdialog.h ui/operationdialog.cpp ui/operationdialog.ui
        app/analyticsmanager.h app/analyticsmanager.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET household-budget APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(household-budget SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(household-budget
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(household-budget PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(household-budget PRIVATE Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(household-budget PRIVATE Qt${QT_VERSION_MAJOR}::Charts)
target_link_libraries(household-budget PRIVATE Qt${QT_VERSION_MAJOR}::PrintSupport)
target_link_libraries(household-budget ${BCRYPT_LIBRARIES})


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.household-budget)
endif()
set_target_properties(household-budget PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS household-budget
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(household-budget)
endif()
