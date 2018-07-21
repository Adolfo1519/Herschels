
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
#set_property(TARGET tgt PROPERTY CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)
#PROJECT(Herschels)

SET(HERSCHELS_VERSION "0.1.0")
#SET(PACKAGE Herschels)
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets)

SET(BUILD_STATIC_LIB 1 CACHE BOOL "Build the static version of the library.")
SET(BUILD_DYNAMIC_LIB 0 CACHE BOOL "Build the dynamic (plugin) version of the library.")

SET(STELLARIUM_SOURCE_DIR $ENV{STELROOT})
ADD_SUBDIRECTORY( src )

SET(QT_LINK_PARAMETERS Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)

# Building 
SET(CMAKE_INSTALL_PREFIX $ENV{HOME}/Development/stellarium)

# TODO: Remove this hardcoded path
#install(FILES builds/macosx/src/libPointTo.so DESTINATION modules/Herschels/)
install(FILES DESTINATION "modules/Herschels")
