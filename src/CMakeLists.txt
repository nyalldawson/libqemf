## add_subdirectory(demo)


########### library target ###############

set(libemf_LIB_SRCS 
    BitmapHeader.cpp
    EmfHeader.cpp
    EmfLogger.cpp
    EmfObjects.cpp
    EmfOutput.cpp
    EmfParser.cpp
    EmfRecords.cpp
    QEmfRenderer.cpp
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)

add_library(libemf SHARED ${libemf_LIB_SRCS})

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})


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
