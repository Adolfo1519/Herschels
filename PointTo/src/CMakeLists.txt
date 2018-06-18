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
    ../resources/PointTo.qrc
)

SET(PointTo_MOC_HDRS
  gui/PropertyBasedTableModel.hpp
  gui/PointToDialog.hpp
  PointTo.hpp
  Sweep.hpp
  SweepTools.hpp
  )

# Wrapping UI
find_package(Qt5 COMPONENTS Widgets REQUIRED)

QT5_WRAP_UI(PointTo_UIS_H ${PointTo_UIS})
QT5_ADD_RESOURCES(PointTo_RES_SXX ${PointTo_RES})
QT5_WRAP_CPP(PointTo_MOC_SRCS ${PointTo_MOC_HDRS})

SET(extLinkerOption ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(PointTo MODULE ${PointTo_SRCS} ${PointTo_MOC_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
 QT5_USE_MODULES(PointTo Core Gui)
 TARGET_LINK_LIBRARIES(PointTo ${StelMain} Qt5::Widgets ${extLinkerOption})

 INSTALL(TARGETS PointTo DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)


############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(PointTo-static STATIC ${PointTo_SRCS} ${PointTo_MOC_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
 QT5_USE_MODULES(PointTo-static Core Gui)
 SET_TARGET_PROPERTIES(PointTo-static PROPERTIES OUTPUT_NAME "PointTo")
 TARGET_LINK_LIBRARIES(PointTo-static ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(PointTo-static PROPERTIES COMPILE_FLAGS "-fPIC -DQT_STATICPLUGIN")
ENDIF(BUILD_STATIC_LIB)



# Library properties
#ADD_LIBRARY(PointTo-static MODULE ${PointTo_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
#QT5_USE_MODULES(PointTo-static Core Gui)
#SET_TARGET_PROPERTIES(PointTo-static PROPERTIES OUTPUT_NAME "PointTo" AUTOMOC TRUE COMPILE_FLAGS "-#DQT_POINTTO")
#TARGET_LINK_LIBRARIES(PointTo-static Qt5::Widgets${extLinkerOption})
