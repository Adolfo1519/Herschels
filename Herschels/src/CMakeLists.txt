INCLUDE_DIRECTORIES(
        .
        gui
        ${CMAKE_BINARY_DIR}/plugins/Herschels/src
        ${CMAKE_SOURCE_DIR}/plugins/Oculars/src
        ${CMAKE_SOURCE_DIR}/plugins/Oculars/src/gui
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/gui

        #/home/adolfo/Documents/stellarium-0.18.0/plugins/Oculars/src
        #/home/adolfo/Documents/stellarium-0.18.0/plugins/Oculars/src/gui
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
#        /home/adolfo/Documents/stellarium-0.18.0/src
#        /home/adolfo/Documents/stellarium-0.18.0/src/core
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/modules
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/planetsephems
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/external
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/qtcompress
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/glues_stel/source/
#        /home/adolfo/Documents/stellarium-0.18.0/src/core/external/glues_stel/source/libtess
#        /home/adolfo/Documents/stellarium-0.18.0/src/gui
#        /home/adolfo/Documents/stellarium-0.18.0/src/scripting
        ${INTL_INCLUDE_DIR}
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_BINARY_DIR}/src
        ${ZLIB_INCLUDE_DIR}
)

LINK_DIRECTORIES(/src)

SET(Herschels_SRCS
    # Source cde
    Herschels.hpp
    Herschels.cpp
    Sweep.hpp
    Sweep.cpp
    SweepTools.hpp
    SweepTools.cpp
    gui/HerschelsDialog.hpp
  	gui/HerschelsDialog.cpp
    gui/PropertyBasedTableModel_Point.hpp
        gui/PropertyBasedTableModel_Point.cpp
    gui/SweepEffectsDialog.hpp
        gui/SweepEffectsDialog.cpp
    gui/CalendarDialog.hpp
        gui/CalendarDialog.cpp
    gui/StopDialog.hpp
        gui/StopDialog.cpp
 )

SET(Herschels_UIS
    # Forms
    gui/herschelsDialog.ui
    gui/sweepEffectsDialog.ui
    gui/calendarDialog.ui
    gui/stopDialog.ui
)

SET(Herschels_RES_CXX
    # Resources
    ../resources/Herschels.qrc
)

SET(Herschels_MOC_HDRS
  gui/PropertyBasedTableModel_Point.hpp
  gui/HerschelsDialog.hpp
  gui/SweepEffectsDialog.hpp
  gui/CalendarDialog.hpp
  gui/StopDialog.hpp
  Herschels.hpp
  Sweep.hpp
  SweepTools.hpp
  )
find_package(Qt5 COMPONENTS Widgets REQUIRED)
QT5_WRAP_CPP(Herschels_MOC_SRCS ${Herschels_MOC_HDRS})

# Wrapping UI

QT5_WRAP_UI(Herschels_UIS_H ${Herschels_UIS})
QT5_ADD_RESOURCES(Herschels_RES_CXX ${Herschels_RES_CXX})

SET(extLinkerOption ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(Herschels MODULE ${Herschels_SRCS} ${Herschels_MOC_SRCS} ${Herschels_RES_CXX} ${Herschels_UIS_H})
 QT5_USE_MODULES(Herschels Core Gui)
 TARGET_LINK_LIBRARIES(Herschels ${StelMain} Qt5::Core Qt5::Widgets ${extLinkerOption})

 INSTALL(TARGETS Herschels DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)


############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(Herschels-static STATIC ${Herschels_SRCS} ${Herschels_MOC_SRCS} ${Herschels_RES_CXX} ${Herschels_UIS_H})
 SET_TARGET_PROPERTIES(Herschels-static PROPERTIES OUTPUT_NAME "Herschels")
 TARGET_LINK_LIBRARIES(Herschels-static Qt5::Core Qt5::Widgets ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(Herschels-static PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
 SET_TARGET_PROPERTIES(Herschels-static PROPERTIES FOLDER "plugins/Herschels")

 add_dependencies(AllStaticPlugins Herschels-static)
ENDIF(BUILD_STATIC_LIB)



# Library properties
#ADD_LIBRARY(PointTo-static MODULE ${PointTo_SRCS} ${PointTo_RES_SXX} ${PointTo_UIS_H})
#QT5_USE_MODULES(PointTo-static Core Gui)
#SET_TARGET_PROPERTIES(PointTo-static PROPERTIES OUTPUT_NAME "PointTo" AUTOMOC TRUE COMPILE_FLAGS "-#DQT_POINTTO")
#TARGET_LINK_LIBRARIES(PointTo-static Qt5::Widgets${extLinkerOption})
