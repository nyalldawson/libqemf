## add_subdirectory(demo)


########### library target ###############

set(EmfViewer_SRCS
    main.cpp
    EmfViewer.cpp

    ../src/EmfParser.cpp
    ../src/EmfLogger.cpp
    ../src/EmfOutput.cpp
    ../src/QEmfRenderer.cpp
    ../src/EmfHeader.cpp
    ../src/EmfRecords.cpp
    ../src/Bitmap.cpp
    ../src/BitmapHeader.cpp
    ../src/EmfObjects.cpp
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
FIND_PACKAGE(Qt5Svg REQUIRED)


include_directories(SYSTEM ${Qt5Core_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Gui_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Widgets_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Printer_INCLUDE_DIRS})
include_directories(SYSTEM ${Qt5Svg_INCLUDE_DIRS})
include_directories(../src)

set(CMAKE_AUTOMOC ON)

add_executable( emf_viewer ${EmfViewer_SRCS} )
target_link_libraries( emf_viewer ${Qt5Gui_LIBRARIES} ${Qt5PrintSupport_LIBRARIES} ${Qt5Svg_LIBRARIES} )

#set_target_properties(libemf PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} )


########### install files ###############

#install(TARGETS libemf  ${INSTALL_TARGETS_DEFAULT_ARGS})

#install(
#    FILES
#    EmfEnums.h
#    EmfRecords.h
#    EmfObjects.h
#    EmfHeader.h
#    Bitmap.h
#    EmfParser.h
#    EmfOutput.h
#    EmfOutputDebugStrategy.h
#    EmfOutputPainterStrategy.h
#    emf_export.h
#    DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel
#)
