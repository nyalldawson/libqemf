FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5PrintSupport REQUIRED)
FIND_PACKAGE(Qt5Svg REQUIRED)


########### library target ###############

SET(EMFVIEWER_SRCS
    main.cpp
    EmfViewer.cpp
)

INCLUDE_DIRECTORIES(SYSTEM
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${Qt5Widgets_INCLUDE_DIRS}
                    ${Qt5Printer_INCLUDE_DIRS}
                    ${Qt5Svg_INCLUDE_DIRS}
                    )
INCLUDE_DIRECTORIES(
                    ${CMAKE_SOURCE_DIR}/src
                    )

SET(CMAKE_AUTOMOC ON)

ADD_EXECUTABLE( emf_viewer ${EMFVIEWER_SRCS} )
TARGET_LINK_LIBRARIES( emf_viewer
  libemf
  ${Qt5Gui_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
)

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
