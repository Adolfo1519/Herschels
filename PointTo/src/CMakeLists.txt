# Copyright (C) 2007 Fabien Chereau
# Copyright (C) 2015 Kirill Snezhko
#
# Based on the HelloStelModule

INCLUDE_DIRECTORIES(.)
#INCLUDE_DIRECTORIES(
#        ${CMAKE_BINARY_DIR}
#        ${CMAKE_SOURCE_DIR}/src
#        ${CMAKE_SOURCE_DIR}/src/gui
#        ${STELLARIUM_SOURCE_DIR}/src
#        ${STELLARIUM_SOURCE_DIR}/src/core
#        ${STELLARIUM_SOURCE_DIR}/src/core/modules
#        ${STELLARIUM_SOURCE_DIR}/src/core/planetsephems
#        ${STELLARIUM_SOURCE_DIR}/src/core/external
#        ${STELLARIUM_SOURCE_DIR}/src/core/external/qtcompress
#        ${STELLARIUM_SOURCE_DIR}/src/core/external/glues_stel/source/
#        ${STELLARIUM_SOURCE_DIR}/src/core/external/glues_stel/source/libtess
#        ${STELLARIUM_SOURCE_DIR}/src/gui
#        ${STELLARIUM_SOURCE_DIR}/src/scripting
#        ${INTL_INCLUDE_DIR}
#        ${CMAKE_BINARY_DIR}/src
#        ${ZLIB_INCLUDE_DIR}
#)
LINK_DIRECTORIES(${BUILD_DIR}/src)

SET(PointTo_SRCS
    # Source cde
    PointTo.hpp
    PointTo.cpp
    Sweep.hpp
    Sweep.cpp
    SweepTools.hpp
    SweepTools.cpp
    gui/PointToDialog.hpp
  	gui/PointToDialog.cpp
    gui/PropertyBasedTableModel.hpp
	gui/PropertyBasedTableModel.cpp
 )

SET(PointTo_UIS
    # Forms
    gui/pointToDialog.ui
)

SET(PointTo_RES_SXX
    # Resources
)

# Wrapping UI
find_package(Qt5 COMPONENTS Widgets REQUIRED)

QT5_WRAP_UI(PointTo_UIS_H ${PointTo_UIS})

# Library properties
ADD_LIBRARY(PointTo-static MODULE ${PointTo_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
QT5_USE_MODULES(PointTo-static Core Gui)
SET_TARGET_PROPERTIES(PointTo-static PROPERTIES OUTPUT_NAME "PointTo" AUTOMOC TRUE COMPILE_FLAGS "-DQT_POINTTO")
TARGET_LINK_LIBRARIES(PointTo-static Qt5::Widgets${extLinkerOption})
