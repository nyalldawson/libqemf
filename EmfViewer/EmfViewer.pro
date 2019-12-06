# qmake project file for building the emfviewer tool

#include( ../config.pri )

TARGET         = EmfViewer
TEMPLATE       = app
CONFIG        += warn_on release thread

# uncomment the line bellow in order to allow static building
#CONFIG        += static_build
contains(CONFIG, StaticBuild){
	QTPLUGIN += qjpeg qgif qtiff qmng
	DEFINES += STATIC_BUILD
}

QT += svg

contains(QT_VERSION, ^5.*){
	QT += widgets printsupport
	macx: QMAKE_MAC_SDK = macosx10.12
}

# uncomment the line bellow in order to disable building as application bundle on Mac OS X
#macx: CONFIG -= app_bundle

MOC_DIR      = ../tmp
OBJECTS_DIR  = ../tmp
DESTDIR      = ./

###################### ICONS ################################################
INCLUDEPATH  += icons/
win32:RC_FILE = icons/emfviewer.rc
macx:RC_FILE   = icons/emfviewer.icns
RESOURCES     = icons/icons.qrc

INCLUDEPATH += ../src

HEADERS  = EmfViewer.h
HEADERS += ../src/Bitmap.h\
	../src/BitmapHeader.h\
	../src/EmfEnums.h\
	../src/EmfHeader.h\
	../src/EmfLogger.h\
	../src/EmfObjects.h\
	../src/EmfOutput.h\
	../src/EmfParser.h\
	../src/EmfRecords.h\
	../src/QEmfRenderer.h

SOURCES  = main.cpp
SOURCES += EmfViewer.cpp
SOURCES += ../src/Bitmap.cpp\
	../src/BitmapHeader.cpp\
	../src/EmfHeader.cpp\
	../src/EmfLogger.cpp\
	../src/EmfObjects.cpp\
	../src/EmfOutput.cpp\
	../src/EmfParser.cpp\
	../src/EmfRecords.cpp\
	../src/QEmfRenderer.cpp

win32: LIBS += -lgdi32
mac:   LIBS += -framework Cocoa
