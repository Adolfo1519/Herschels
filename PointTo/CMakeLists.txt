
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
#set_property(TARGET tgt PROPERTY CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)
PROJECT(PointTo)

SET(DYNAMICPLUGINTEMPLATE_VERSION "0.1.0")

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Widgets)

SET(STELLARIUM_SOURCE_DIR $ENV{STELROOT})

INCLUDE_DIRECTORIES(
	${CMAKE_BINARY_DIR}
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/src/gui
        /home/adolfo/Documents/stellarium-0.18.0/src
        /home/adolfo/Documents/stellarium-0.18.0/src/core
        /home/adolfo/Documents/stellarium-0.18.0/src/core/modules
        /home/adolfo/Documents/stellarium-0.18.0/src/core/planetsephems
        /home/adolfo/Documents/stellarium-0.18.0/src/core/external
        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/qtcompress
        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/glues_stel/source/
        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/glues_stel/source/libtess
        /home/adolfo/Documents/stellarium-0.18.0/src/gui
        /home/adolfo/Documents/stellarium-0.18.0/src/scripting
	${INTL_INCLUDE_DIR}
	${CMAKE_BINARY_DIR}/src
	${CMAKE_BINARY_DIR}/src	
	${ZLIB_INCLUDE_DIR}
)

ADD_SUBDIRECTORY( src )

SET(QT_LINK_PARAMETERS Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::Widgets)

# Building 
SET(CMAKE_INSTALL_PREFIX $ENV{HOME}/.stellarium)

# TODO: Remove this hardcoded path
install(FILES builds/unix/src/libPointTo.so DESTINATION modules/PointTo/)
