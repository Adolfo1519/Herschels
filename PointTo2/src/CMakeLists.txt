INCLUDE_DIRECTORIES(
        .
        gui
        ${CMAKE_BINARY_DIR}/plugins/PointTo2/src
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
)

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

LINK_DIRECTORIES(/src)

SET(PointTo2_SRCS
    # Source cde
    PointTo2.hpp
    PointTo2.cpp
    Sweep.hpp
    Sweep.cpp
    SweepTools.hpp
    SweepTools.cpp
    gui/PointTo2Dialog.hpp
  	gui/PointTo2Dialog.cpp
    gui/PropertyBasedTableModel_Point.hpp
        gui/PropertyBasedTableModel_Point.cpp
 )

SET(PointTo2_UIS
    # Forms
    gui/pointTo2Dialog.ui
)

SET(PointTo2_RES_CXX
    # Resources
    ../resources/PointTo2.qrc
)

SET(PointTo2_MOC_HDRS
  gui/PropertyBasedTableModel_Point.hpp
  gui/PointTo2Dialog.hpp
  PointTo2.hpp
  Sweep.hpp
  SweepTools.hpp
  )
find_package(Qt5 COMPONENTS Widgets REQUIRED)
QT5_WRAP_CPP(PointTo2_MOC_SRCS ${PointTo2_MOC_HDRS})

# Wrapping UI

QT5_WRAP_UI(PointTo2_UIS_H ${PointTo2_UIS})
QT5_ADD_RESOURCES(PointTo2_RES_CXX ${PointTo2_RES_CXX})

SET(extLinkerOption ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(PointTo2 MODULE ${PointTo2_SRCS} ${PointTo2_MOC_SRCS} ${PointTo2_RES_CXX} ${PointTo2_UIS_H})
 QT5_USE_MODULES(PointTo2 Core Gui)
 TARGET_LINK_LIBRARIES(PointTo2 ${StelMain} Qt5::Core Qt5::Widgets ${extLinkerOption})

 INSTALL(TARGETS PointTo2 DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)


############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(PointTo2-static STATIC ${PointTo2_SRCS} ${PointTo2_MOC_SRCS} ${PointTo2_RES_CXX} ${PointTo2_UIS_H})
 SET_TARGET_PROPERTIES(PointTo2-static PROPERTIES OUTPUT_NAME "PointTo2")
 TARGET_LINK_LIBRARIES(PointTo2-static Qt5::Core Qt5::Widgets ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(PointTo2-static PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
 add_dependencies(AllStaticPlugins PointTo2-static)
ENDIF(BUILD_STATIC_LIB)



# Library properties
#ADD_LIBRARY(PointTo-static MODULE ${PointTo_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
#QT5_USE_MODULES(PointTo-static Core Gui)
#SET_TARGET_PROPERTIES(PointTo-static PROPERTIES OUTPUT_NAME "PointTo" AUTOMOC TRUE COMPILE_FLAGS "-#DQT_POINTTO")
#TARGET_LINK_LIBRARIES(PointTo-static Qt5::Widgets${extLinkerOption})
