cmake_minimum_required(VERSION 3.5)
project(panda-files)

set(TARGET_NAME panda-files)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(QT Core Gui Widgets DBus Svg X11Extras LinguistTools)
find_package(Qt5 REQUIRED ${QT})
find_package(PkgConfig REQUIRED)
find_package(GLIB REQUIRED COMPONENTS gio gio-unix gobject gthread)
find_package(MenuCache REQUIRED)
find_package(Exif REQUIRED)
find_package(XCB REQUIRED)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)

add_subdirectory(src)
