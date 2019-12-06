############################################################
# sources

SET(LIBEMF_SRCS
    Bitmap.cpp
    BitmapHeader.cpp
    EmfHeader.cpp
    EmfLogger.cpp
    EmfObjects.cpp
    EmfOutput.cpp
    EmfParser.cpp
    EmfRecords.cpp
    QEmfRenderer.cpp
)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)

INCLUDE_DIRECTORIES(SYSTEM
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS})

#############################################################
# libemf library
ADD_LIBRARY(libemf SHARED ${LIBEMF_SRCS})

TARGET_LINK_LIBRARIES(libemf
  ${Qt5Core_LIBRARIES}
  ${Qt5Gui_LIBRARIES}
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
